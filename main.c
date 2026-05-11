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

extern TaskHandle_t encoder_task;



int main(void)
{
    lcd_queue = xQueueCreate(QUEUE_LEN,sizeof(char));
    keypad_queue = xQueueCreate(QUEUE_LEN,sizeof(char));
    encoder_queue = xQueueCreate(QUEUE_LEN,sizeof(int8_t));

    uart_tx_queue = xQueueCreate(TX_QUEUE_LEN,sizeof(char));

    green_led_queue = xQueueCreate(1,sizeof(uint8_t));
    yellow_led_queue = xQueueCreate(1,sizeof(uint8_t));
    red_led_queue = xQueueCreate(1,sizeof(uint8_t));

    SW_1_queue = xQueueCreate(1,sizeof(uint8_t));
    SW_2_queue = xQueueCreate(1,sizeof(uint8_t));

    keypad_sem = xSemaphoreCreateBinary();
    encoder_sem = xSemaphoreCreateBinary();
    uart_rx_sem = xSemaphoreCreateBinary();
    price_wr_mutex = xSemaphoreCreateMutex();

    uart0_init(19200, DBITS_8, SBIT_1, NO_PARITY); //must be here for some reason or the mcu crashes
    Led_init();
    buttons_init();

    xTaskCreate(lcd_Task, "LCD Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL );
    xTaskCreate(keypad_task, "Keypad Task", USERTASK_STACK_SIZE, NULL, PRIO_HIGH, NULL );
    xTaskCreate(encoder_Task, "Encoder Task", USERTASK_STACK_SIZE, NULL, PRIO_VERYMID, &encoder_task);

    xTaskCreate(uart_tx_Task, "Uart TX Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    xTaskCreate(uart_rx_Task, "Uart RX Task", USERTASK_STACK_SIZE, NULL, PRIO_INCONSEQUENTIAL, NULL);

    xTaskCreate(Green_LED_Task, "Green LED Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    xTaskCreate(Yellow_LED_Task, "Yellow LED Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);
    xTaskCreate(Red_LED_Task, "Red LED Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);

    xTaskCreate(SW_1_Task, "SW 1 Task", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL);
    xTaskCreate(SW_2_Task, "SW 2 Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL);

    xTaskCreate(userflow_Task, "Userflow Task", USERTASK_STACK_SIZE, NULL, PRIO_VERYMID, NULL);
    vTaskStartScheduler();

	return 0;
}



