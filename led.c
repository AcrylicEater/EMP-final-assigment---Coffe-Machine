#include "Led.h"

void Led_init(void){
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; //initialize GPIOF
    uint32_t dummy = SYSCTL_RCGC2_R;
    GPIO_PORTF_DIR_R |= LED_MASK; //output
    GPIO_PORTF_DEN_R |= LED_MASK; //digital function
    GPIO_PORTF_DATA_R &= ~LED_MASK; //Set all LEDs off
}