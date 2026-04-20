#include "LCD_frt.h"

static uint8_t cursor_pos = 0;


void timer0_init(void)
{
    SYSCTL_RCGCTIMER_R |= 0x01;     // enable clock to Timer0
    while((SYSCTL_PRTIMER_R & 0x01) == 0); // wait until ready

    TIMER0_CTL_R = 0;               // disable timer during setup
    TIMER0_CFG_R = 0x04;            // 16-bit timer configuration
    TIMER0_TAMR_R = 0x02;           // periodic mode

    TIMER0_TAILR_R = 0xFFFF;        // max reload
    TIMER0_TAPR_R = 15;             // prescaler -> 1 tick = 1 µs (16MHz /16)

    TIMER0_CTL_R |= 0x01;           // enable timer
}

void delay_us(uint16_t us)
{
    uint16_t start = TIMER0_TAV_R;

    while((uint16_t)(start - TIMER0_TAV_R) < us);
}

void lcd_command(uint8_t cmd){
    GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~(LCD_DATA)) | (cmd & LCD_DATA); //Put the four highest bits of CMD on portC
    GPIO_PORTD_DATA_R &= ~(LCD_RS); //Select command register
    GPIO_PORTD_DATA_R |= LCD_EN; //Pull enable pin high to latch data
    delay_us(TIMEOUT);
    GPIO_PORTD_DATA_R &= ~(LCD_EN); //Pull enable pin low
    delay_us(TIMEOUT); // give it time to write to the register

    GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~(LCD_DATA)) | (cmd<<4); //write the lowest four bits of CMD on portD
    GPIO_PORTD_DATA_R |= LCD_EN; //Pull enable pin high to latch data
    delay_us(TIMEOUT);
    GPIO_PORTD_DATA_R &= ~(LCD_EN); //Pull enable pin low
    delay_us(TIMEOUT); // give it time to write to the register
}



void lcd_init(){
    timer0_init();

    SYSCTL_RCGC2_R |= (SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD); //initialize GPIOC & GPIOD
    uint32_t dummy = SYSCTL_RCGC2_R;

    delay_us(15000);
    //Set data direction
    GPIO_PORTC_DIR_R |= LCD_DATA; //PC7-PC4 as output
    GPIO_PORTD_DIR_R |= (LCD_RS | LCD_EN); //PD2 and PD3 as output

    //Set digital function
    GPIO_PORTC_DEN_R |= LCD_DATA;
    GPIO_PORTD_DEN_R |= (LCD_RS | LCD_EN);

    //LCD SETUP
    GPIO_PORTD_DATA_R &= ~(LCD_RS | LCD_EN); //pull control lines low for minimization of funny business

    lcd_command(0x02); //return to start of dram
    delay_us(TIMEOUT); // wait for execution time of
    lcd_command(0x28); // 4-bit mode, 2 lines, 8 dot per symbol
    delay_us(500);
    lcd_command(0x01); // clear display
    lcd_command(0x0C); //Turn display on, turn off cursor
    delay_us(500);
    //lcd_command(0x06);
}

void lcd_setcursor(uint8_t pos){
    if(pos>=32){return;}
    uint8_t shift_cmd;
    int8_t dir;
    if(pos>cursor_pos){
        dir = 1;
        shift_cmd = 0b00010100;
    } else if (pos<cursor_pos){
        dir = -1;
        shift_cmd = 0b00010000;
    } else{
        return;
    }
    while(cursor_pos!=pos){
        if(cursor_pos==16){
            uint8_t i;
            for(i=0; i<24; i++ ){
                lcd_command(shift_cmd);
            }
        }
        lcd_command(shift_cmd);
        cursor_pos += dir;
    }
    return;
}

void lcd_char(uint8_t character){
    switch(cursor_pos){
        case 16:
        {
            uint8_t i;
            for(i=0; i<24; i++ ){
                lcd_command(0b00010100);
            }
        }
            break;
        case 32:
        {
            lcd_setcursor(0);
        }
            break;
    }
    GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~(LCD_DATA)) | (character & LCD_DATA); //Put the four highest bits of CMD on portC
    GPIO_PORTD_DATA_R |= LCD_RS; //Select data register
    GPIO_PORTD_DATA_R |= LCD_EN; //Pull enable pin high to latch data
    delay_us(TIMEOUT);
    GPIO_PORTD_DATA_R &= ~(LCD_EN); //Pull enable pin low

    GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~(LCD_DATA)) | (character<<4); //write the lowest four bits of CMD on portD
    GPIO_PORTD_DATA_R |= LCD_EN; //Pull enable pin high to latch data
    delay_us(TIMEOUT);
    GPIO_PORTD_DATA_R &= ~(LCD_EN); //Pull enable pin low
    delay_us(TIMEOUT); // give it time to write to the register
    cursor_pos++;
}

void lcd_string(uint8_t *str){
    while(*str){
        lcd_char(*str);
        str++;
    }
}

void lcd_clear(){
    lcd_command(0x01);
    cursor_pos = 0;
}


uint8_t lcd_getcursor(){
    return cursor_pos;
}

void lcd_queueString(char* string){
    while(*string){
        char c = *string;
        xQueueSend(lcd_queue, &c, 1000);
        string++;
    }



}

void lcd_Task(void *pvParameters){
    lcd_init();

    //lcd_queue = xQueueCreate(QUEUE_LEN,sizeof(char));

    char msg; //Variable to put queue contents


    while(1){
        if(xQueueReceive(lcd_queue, &msg, portMAX_DELAY) == pdPASS){ //if there is stuff in queue, put in msg
            if(msg == CLEAR_LCD ){ //if we pass the CLEAR_LCD character (0x7F or DEL)
                lcd_clear();
            }else if (msg<32){     // 32 positions on screen, so numbers 0 to 31 are reserved for setting cursor
               lcd_setcursor(msg);
            }else{
                lcd_char(msg);
            }

        }
    }
}

