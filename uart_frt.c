
#include "uart_frt.h"
#include "Led.h"
#include "coffee_control.h"
#include <string.h>


/*****************************   Variables   *******************************/
static char rx_buffer[RX_BUFFER_LEN];
uint8_t rx_tail = 0;

/*****************************   Functions   *******************************/


void uart0_fifos_enable()
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Enable the tx and rx fifos
******************************************************************************/
{
  UART0_LCRH_R  |= 0x00000010;
}

void uart0_fifos_disable()
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : disable the tx and rx fifos
******************************************************************************/
{
  UART0_LCRH_R  &= 0xFFFFFFEF;
}



bool_t uart0_tx_rdy()
{
  return ( UART0_FR_R & UART_FR_TXFE ) ? TRUE : FALSE ;
}


void uart0_queueString(char* string){
    while(*string){
        char c = *string;
        xQueueSend(uart_tx_queue, &c, 1000);
        string++;
    }
}

void uart0_init( uint32_t baud_rate, NOF_DATABITS_t databits, NOF_STOPBITS_t stopbits, PARITY_t parity )
{

  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;	// Enable clock for Port A
  uint32_t dummy = SYSCTL_RCGC2_R;
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0;	// Enable clock for UART 0
  dummy = SYSCTL_RCGC1_R;

  GPIO_PORTA_AFSEL_R |= (UART_TX_PIN | UART_RX_PIN);// set pins to alternative function (uart0)
  GPIO_PORTA_DIR_R   |= UART_TX_PIN;                // set uart tx pin to output
  GPIO_PORTA_DIR_R   &= ~(UART_RX_PIN);             // set uart rx pin to input
  GPIO_PORTA_DEN_R   |= (UART_TX_PIN | UART_RX_PIN);// enable digital operation of uart pins

  //calculate and configure baud rate
  uint32_t BRD = 64000000 / baud_rate; // X-sys*64/(16*baudrate) = 16M*4/baudrate
  UART0_IBRD_R = BRD / 64;
  UART0_FBRD_R = BRD & 0x0000003F;

  UART0_LCRH_R  = databits;
  UART0_LCRH_R += stopbits;
  UART0_LCRH_R += parity;

  uart0_fifos_disable();

  //RX interrupt setup

  UART0_ICR_R |= UART_IM_RXIM; //Clear interrupt
  UART0_IM_R  |= UART_IM_RXIM; //Allow uart RX interrupt to be sent to controller

  NVIC_EN0_R |=  UART_NVIC_INT;        //enable uart interrupt in vector table
  NVIC_PRI1_R &= ~(UART_INT_PRIO_MASK);// clear current interrupt priority
  NVIC_PRI1_R |= (UART_INT_PRIO << 13);

  UART0_CTL_R  |= (UART_CTL_UARTEN | UART_CTL_TXE );  // Enable UART
}

void uart_tx_Task(void *pvParameters){
    //uart0_init(19200, DBITS_8, SBIT_1, NO_PARITY);

    char msg; //character to put incoming data in

    while(1){
        if(xQueueReceive(uart_tx_queue, &msg, portMAX_DELAY) == pdPASS){
            while(!uart0_tx_rdy());
            UART0_DR_R = msg; // write character to queue
        }
    }
}

uint8_t parse_price(char* key){
    uint16_t price = 0;
    while(*key >= '0' && *key <= '9'){
        price *= 10;
        price += (*key - '0');
        key++;
    }
    if(price>255){
        uart0_queueString("TOO GREEDY\n");
        return 1;
    }
    return (uint8_t)price;
}

void uart_rx_Task(void *pvParameters){

    char cmd[RX_BUFFER_LEN];
    uint8_t msg_len;
    uint8_t led_cmd;
    while(1){
        if(xSemaphoreTake(uart_rx_sem, portMAX_DELAY) == pdPASS){
            UART0_IM_R &= ~(UART_IM_RXIM); //Disable uart RX interrupt, while copying the msg, to avoid overwriting it
            msg_len = rx_tail;
            int i = 0;

            while(rx_buffer[i]!='\0'){
                cmd[i] = rx_buffer[i];
                i++;
            }
            cmd[i] = '\0';
            rx_tail = 0;
            UART0_IM_R |= UART_IM_RXIM; //Re-enable uart RX interrupt

            if(strncmp(cmd,"SET_PRICE ",10)==0){
                char* key = &cmd[10];
                switch(*key){
                case 'E':
                    key += 2;
                    set_coffee_price(ESPRESSO, parse_price(key));
                    break;
                case 'L':
                    key += 2;
                    set_coffee_price(LATTE, parse_price(key));
                    break;
                case 'F':
                    key += 2;
                    set_coffee_price(FILTER, parse_price(key));
                    break;
                default:
                    uart0_queueString("INVALID PRICE CMD");
                }
            } else{
                uart0_queueString("INVALID CMD");
            }

            /*
            //temp fjolle kommandoer
            if(strncmp(cmd,"GIV MIG GULD",msg_len) == 0){
                uart0_queueString("Carl er sej\n");
            }
            if(strncmp(cmd,"SHOW ME MONEY",msg_len) == 0){
                led_cmd = 1;
                xQueueSend(green_led_queue, &led_cmd, 1000);;
            }
            if(strncmp(cmd,"MONEY AWAY",msg_len) == 0){
                led_cmd = 0;
                xQueueSend(green_led_queue, &led_cmd, 1000);;
            }
            */
        }
    }
}


void UART0_int_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    UART0_ICR_R |= UART_IM_RXIM; //Clear interrupt

    if(UART0_FR_R & UART_FR_RXFF){
      rx_buffer[rx_tail] = UART0_DR_R;
      if(rx_buffer[rx_tail]=='\n') {
        rx_buffer[rx_tail] = '\0';
        xSemaphoreGiveFromISR(uart_rx_sem, &xHigherPriorityTaskWoken);
      }
      rx_tail++;

      if(rx_tail == RX_BUFFER_LEN){
          rx_tail--;
          rx_buffer[rx_tail] = '\0';
          xSemaphoreGiveFromISR(uart_rx_sem, &xHigherPriorityTaskWoken);
      }

    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
















