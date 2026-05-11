#include "buttons.h"

extern QueueHandle_t green_led_queue;

void buttons_init(void){
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
    GPIO_PORTF_CR_R = 0x1F;           // allow changes

    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; //initialize GPIOF
    uint32_t dummy = SYSCTL_RCGC2_R;
    GPIO_PORTF_DIR_R &= ~(BUTTONS_MASK); // set proper PORTF bits to input
    GPIO_PORTF_DEN_R |= BUTTONS_MASK; // set digital function
    GPIO_PORTF_PUR_R |= BUTTONS_MASK; // enable pull-up resistors
}

void SW_1_Task(void *pvParamerters){
    uint8_t state = STATE_RELEASED;
    while(1){
        switch (state)
        {
            case STATE_RELEASED: //if we are released, we look for the button being pressed
                if (!(GPIO_PORTF_DATA_R & SW_1_PIN)){
                    state = STATE_PRESSED;
                    xQueueOverwrite(SW_1_queue, &state);

                }
                break;
            case STATE_PRESSED: //if we are pressed, we look for the button being released
                if (GPIO_PORTF_DATA_R & SW_1_PIN){
                    state = STATE_RELEASED;
                    xQueueOverwrite(SW_1_queue, &state);
                }
        }
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)); //debouncing delay
    }        
}

void SW_2_Task(void *pvParamerters){
    uint8_t state;
    while(1){
        switch (state)
        {
            case STATE_RELEASED:
                if (!(GPIO_PORTF_DATA_R & SW_2_PIN)){
                    state = STATE_PRESSED;
                    xQueueSend(SW_2_queue, &state, 1000);
                }
                break;

            case STATE_PRESSED:
                if (GPIO_PORTF_DATA_R & SW_2_PIN){
                    state = STATE_RELEASED;
                    xQueueSend(SW_2_queue, &state, 1000);
                }
        }
        
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)); //debouncing delay
    }
}
