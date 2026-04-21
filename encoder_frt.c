#include "encoder_frt.h"

//SemaphoreHandle_t encEdgeSem;
//SemaphoreHandle_t encPushSem;




void enc_init(){
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; //initialize GPIOA clock
    uint32_t dummy = SYSCTL_RCGC2_R;      //dummy read for reasons

    //Pin initialization
    GPIO_PORTA_DIR_R &= ~(DIGI_A_PIN | DIGI_B_PIN); //set pins as input
    GPIO_PORTA_DEN_R |= (DIGI_A_PIN | DIGI_B_PIN);  //digital function

    //Interrupt setup
    GPIO_PORTA_IS_R  &= ~(DIGI_A_PIN);  //Interrupt fires on edges
    GPIO_PORTA_IBE_R |= DIGI_A_PIN;     //Detect both edges of A pin



    GPIO_PORTA_ICR_R |= (DIGI_A_PIN);    //Clear interrupt
    GPIO_PORTA_IM_R |= (DIGI_A_PIN);     //Allow the interrupt to be sent to the interrupt controller

    NVIC_EN0_R |= GPIOA_INTERRUPT;        //Enable the GPIOA interrupt in the vector table
    NVIC_PRI0_R &= ~(GPIOA_INTPRIO_MASK); //Clear the current GPIOA interrupt priority
    NVIC_PRI0_R |= (GPIOA_INTPRIO << 5);  //Set the GPIOA interrupt Priority
}

void encoder_Task(void *pvParameters){
    enc_init();

    while(1){
        if(xSemaphoreTake(encoder_sem,portMAX_DELAY)) {       //wait for encoder interrupt

            uint8_t pinA = GPIO_PORTA_DATA_R & DIGI_A_PIN; //get the state of the A pin of the encoder
            uint8_t pinB = GPIO_PORTA_DATA_R & DIGI_B_PIN; //get the state of the B pin

            int8_t dir = (pinA == (pinB>>1)) ? -1 : 1 ;    //Determine the direction by comparing state of A and B pin
            xQueueSend(encoder_queue, &dir, 1000);

        }
    }
}


void GPIOA_int_handler(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    GPIO_PORTA_ICR_R |= DIGI_A_PIN;    // clear interrupt

    xSemaphoreGiveFromISR(encoder_sem, &xHigherPriorityTaskWoken); //wakeup encoder task
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


