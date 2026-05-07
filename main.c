#include <stdint.h>
#include "tm4c123gh6pm.h"

//####### FreeRTOS includes #######
#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//####### project includes #######
#include "LCD_frt.h"
#include "keypad_frt.h"
#include "encoder_frt.h"
#include "uart_frt.h"
#include "coffee_control.h"
#include "Led.h"
#include "buttons.h"


//####### global defines
#define PRIO_INCONSEQUENTIAL 0
#define PRIO_LOW             1
#define PRIO_MID             2
#define PRIO_VERYMID         3
#define PRIO_HIGH            4
#define PRIO_CRITCAL         5

#define USERTASK_STACK_SIZE configMINIMAL_STACK_SIZE

//####### global handlers #######
extern QueueHandle_t     lcd_queue;
extern QueueHandle_t  keypad_queue;
extern QueueHandle_t encoder_queue;
extern QueueHandle_t uart_tx_queue;
extern QueueHandle_t green_led_queue;
extern QueueHandle_t yellow_led_queue;
extern QueueHandle_t red_led_queue;
extern QueueHandle_t SW_1_queue;
extern QueueHandle_t SW_2_queue;

extern SemaphoreHandle_t     keypad_sem;
extern SemaphoreHandle_t    encoder_sem;
extern SemaphoreHandle_t    uart_rx_sem;
extern SemaphoreHandle_t price_wr_mutex;



//###### TEMPORARY FOR DEBUG #####
#define LED_MASK  0b00001110
enum led_Color {
    BLACK = 0,
    RED = 0b00000010,
    BLUE = 0b00000100,
    PINK = 0b00000110,
    GREEN = 0b00001000,
    YELLOW = 0b00001010,
    CYAN = 0b00001100,
    WHITE = 0b00001110,
};

void init_RGB(void){
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; //initialize GPIOF
    uint32_t dummy = SYSCTL_RCGC2_R;
    GPIO_PORTF_DIR_R |= LED_MASK; // set proper PORTF bits to output
    GPIO_PORTF_DEN_R |= LED_MASK; // set digital function
}

void set_Color(led_Color){
    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~(LED_MASK)) | led_Color; //set corresponding RGB bits on/off
}

//###### END TEMPORARY #####


void dummy_Task(void *pvParameters){
    char clear_msg = CLEAR_LCD;
    char c = 'a';
    while(1){
        xQueueSend(lcd_queue, &clear_msg, 1000);
        lcd_queueString("CARL TEST: ");
        xQueueSend(lcd_queue, &c, 1000);
        c++;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void dummy_Task2(void *pvParameters){
    char msg;
    char pos = 16;
    while(1){
        if(xQueueReceive(keypad_queue, &msg, portMAX_DELAY) == pdPASS){
            xQueueSend(lcd_queue, &pos, 1000);
            xQueueSend(lcd_queue, &msg, 1000);
            pos++;
        }
    }
}

void dummy_Task3(void *pvParameters){
    init_RGB();
    set_Color(BLACK);
    int8_t dir;
    while(1){
        if(xQueueReceive(encoder_queue, &dir, portMAX_DELAY) == pdPASS){
            set_Color( (dir == -1) ? CYAN : PINK );
            vTaskDelay(pdMS_TO_TICKS(250));
            set_Color(BLACK);
        }
    }
}

void dummy_Task4(void *pvParameters){
    while(1){
        uart0_queueString("Carl test\n");
        vTaskDelay(pdMS_TO_TICKS(1750));
    }
}


int main(void)
{
    lcd_queue = xQueueCreate(QUEUE_LEN,sizeof(char));
    keypad_queue = xQueueCreate(QUEUE_LEN,sizeof(char));
    encoder_queue = xQueueCreate(QUEUE_LEN,sizeof(int8_t));
    uart_tx_queue = xQueueCreate(TX_QUEUE_LEN,sizeof(char));
    green_led_queue = xQueueCreate(QUEUE_LEN,sizeof(uint8_t));
    yellow_led_queue = xQueueCreate(QUEUE_LEN,sizeof(uint8_t));
    red_led_queue = xQueueCreate(QUEUE_LEN,sizeof(uint8_t));
    SW_1_queue = xQueueCreate(QUEUE_LEN,sizeof(uint8_t));
    SW_2_queue = xQueueCreate(QUEUE_LEN,sizeof(uint8_t));

    keypad_sem = xSemaphoreCreateBinary();
    encoder_sem = xSemaphoreCreateBinary();
    uart_rx_sem = xSemaphoreCreateBinary();
    price_wr_mutex = xSemaphoreCreateMutex();

    uart0_init(19200, DBITS_8, SBIT_1, NO_PARITY); //must be here for some reason or the mcu crashes
    Led_init();
    buttons_init();

    //xTaskCreate(dummy_Task, "Dummy Task", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );
    xTaskCreate(lcd_Task, "LCD Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL );

    xTaskCreate(keypad_task, "Keypad Task", USERTASK_STACK_SIZE, NULL, PRIO_HIGH, NULL );

    xTaskCreate(userflow_Task, "Userflow Task", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL);
    //xTaskCreate(dummy_Task2, "Dummy Task 2", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );

    //xTaskCreate(encoder_Task, "Encoder Task", USERTASK_STACK_SIZE, NULL, PRIO_VERYMID, NULL);
    //xTaskCreate(dummy_Task3, "Dummy Task 3", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );

    xTaskCreate(uart_tx_Task, "Uart TX Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    //xTaskCreate(dummy_Task4, "Dummy Task 4", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );

    xTaskCreate(uart_rx_Task, "Uart RX Task", USERTASK_STACK_SIZE, NULL, PRIO_INCONSEQUENTIAL, NULL);

    xTaskCreate(Green_LED_Task, "Green LED Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    xTaskCreate(Yellow_LED_Task, "Yellow LED Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    xTaskCreate(Red_LED_Task, "Red LED Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);

    xTaskCreate(SW_1_Task, "SW 1 Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    xTaskCreate(SW_2_Task, "SW 2 Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);

    vTaskStartScheduler();

	return 0;
}
