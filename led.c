#include "Led.h"

void Led_init(void){
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; //initialize GPIOF
    uint32_t dummy = SYSCTL_RCGC2_R;
    GPIO_PORTF_DIR_R |= LED_MASK; //output
    GPIO_PORTF_DEN_R |= LED_MASK; //digital function
    GPIO_PORTF_DATA_R &= ~LED_MASK; //Set all LEDs off
}

void Green_LED_Task(void *pvParameters){
    uint8_t state;
    while(1){
        if(xQueueReceive(green_led_queue, &state, portMAX_DELAY) == pdPASS){
            if(state == ON_STATE){
                GPIO_PORTF_DATA_R |= GREEN_LED_PIN; //Turn on green LED
            } else {
                GPIO_PORTF_DATA_R &= ~GREEN_LED_PIN; //Turn off green LED
            }
        }
    }
}

void Yellow_LED_Task(void *pvParameters){
    uint8_t state;
    while(1){
        if(xQueueReceive(yellow_led_queue, &state, portMAX_DELAY) == pdPASS){
            if(state == ON_STATE){
                GPIO_PORTF_DATA_R |= YELLOW_LED_PIN; //Turn on yellow LED
            } else {
                GPIO_PORTF_DATA_R &= ~YELLOW_LED_PIN; //Turn off yellow LED
            }
        }
    }
}

void Red_LED_Task(void *pvParameters){
    uint8_t state;
    while(1){
        if(xQueueReceive(red_led_queue, &state, portMAX_DELAY) == pdPASS){
            if(state == ON_STATE){
                GPIO_PORTF_DATA_R |= RED_LED_PIN; //Turn on red LED
            } else {
                GPIO_PORTF_DATA_R &= ~RED_LED_PIN; //Turn off red LED
            }
        }
    }
}