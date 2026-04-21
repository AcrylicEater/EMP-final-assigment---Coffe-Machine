#include "keypad_frt.h"


 const char keypad_map[4][3] = {
   {'#','0','*'},
   {'9','8','7'},
   {'6','5','4'},
   {'3','2','1'},
};


void keypad_init(void){
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; //initialize GPIOA
    uint32_t dummy = SYSCTL_RCGC2_R;
    GPIO_PORTA_DIR_R |= KEYPAD_X_MASK; //output
    GPIO_PORTA_DEN_R |= KEYPAD_X_MASK; //digital function

    GPIO_PORTA_DATA_R |= KEYPAD_X_MASK; //Set all keypad inputs high

    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; //initialize GPIOE
    dummy = SYSCTL_RCGC2_R;
    GPIO_PORTE_DIR_R &= ~(KEYPAD_Y_MASK); //input
    GPIO_PORTE_DEN_R |= KEYPAD_Y_MASK;

    //setup interrupt for keypad task
    GPIO_PORTE_IS_R &= ~(KEYPAD_Y_MASK); //interrupt is edge sensitive
    GPIO_PORTE_IBE_R |= KEYPAD_Y_MASK; //interrupt on both edges

    GPIO_PORTE_ICR_R |= KEYPAD_Y_MASK; //clear interrupt
    GPIO_PORTE_IM_R |= KEYPAD_Y_MASK; //unmask interrupt

    NVIC_EN0_R |= (1<<4);              // enable GPIOE interrupt
    NVIC_PRI1_R = (NVIC_PRI1_R & ~(0x7 << 5)) | (5 << 5);
}

uint8_t port_to_index(uint8_t p_data){
    uint8_t index = 0;
    while(p_data >>=1){index++;}
    return index;
}

//The state and the delay is used for debouncing
void keypad_task(void *pvParameters){
    static uint8_t state = STATE_RELEASED;
    keypad_init();

    while(1){
        if(xSemaphoreTake(keypad_sem,portMAX_DELAY)){
            GPIO_PORTE_IM_R &= ~(KEYPAD_Y_MASK); //disable interrupt to avoid disruption
            uint8_t row = GPIO_PORTE_DATA_R & KEYPAD_Y_MASK;
            if(row && (state == STATE_RELEASED)){
                state = STATE_PRESSED;
                uint8_t col;
                for(col = 0b00010000; col>0b00000010; col >>=1){ //start deactivating each column
                    GPIO_PORTA_DATA_R -= col; //deactivate column
                    if( !(GPIO_PORTE_DATA_R & row) ){break;} //if the current row is now low, we have found the key
                }
                GPIO_PORTA_DATA_R |= KEYPAD_X_MASK; //Set all keypad inputs high
                row = port_to_index(row);
                col = port_to_index(col>>2);
                char key = keypad_map[row][col];
                xQueueSend(keypad_queue, &key, portMAX_DELAY);

            } else if(!row && (state == STATE_PRESSED)){
                state = STATE_RELEASED;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            GPIO_PORTE_IM_R |= KEYPAD_Y_MASK; //re-enable interrupt
        }
    }
}

void GPIOE_int_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIO_PORTE_ICR_R = 0x0F;   // clear interrupt

    xSemaphoreGiveFromISR(keypad_sem, &xHigherPriorityTaskWoken);


    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
