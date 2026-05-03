
#include "uart_frt.h"




/*****************************   Variables   *******************************/
bool_t string_rx_flag = FALSE;
//static char rx_buffer[RX_BUFFER_SIZE];
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


bool_t uart0_rx_rdy()
{
  return( UART0_FR_R & UART_FR_RXFF );
}

uint8_t uart0_getc()
{
  return ( UART0_DR_R );
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

/*
extern BOOLEAN uart0_update(void)
/*****************************************************************************
*   Function : See module specification (.h-file).
****************************************************************************
{
  if(uart0_rx_rdy() && rx_tail<RX_BUFFER_SIZE){
    rx_buffer[rx_tail]=uart0_getc();
    if(rx_buffer[rx_tail]=='\n') {string_rx_flag=TRUE;}
    rx_tail++;
  }
  return string_rx_flag;
}
*/

/*
char* readString(void)
/*****************************************************************************
*   Function : See module specification (.h-file).
****************************************************************************
{
  rx_tail = 0;
  string_rx_flag = FALSE;
  return &rx_buffer[0];
}
*/














