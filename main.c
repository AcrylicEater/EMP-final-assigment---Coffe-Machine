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


//####### global defines
#define PRIO_INCONSEQUENTIAL 0
#define PRIO_LOW             1
#define PRIO_MID             2
#define PRIO_VERYMID         3
#define PRIO_HIGH            4
#define PRIO_CRITCAL         5

#define USERTASK_STACK_SIZE configMINIMAL_STACK_SIZE

//####### global handlers #######
extern QueueHandle_t lcd_queue;
extern QueueHandle_t keypad_queue;

extern SemaphoreHandle_t keypad_sem;




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


int main(void)
{
    lcd_queue = xQueueCreate(QUEUE_LEN,sizeof(char));
    keypad_queue = xQueueCreate(QUEUE_LEN,sizeof(char));

    keypad_sem = xSemaphoreCreateBinary();


    xTaskCreate(dummy_Task, "Dummy Task", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );
    xTaskCreate(lcd_Task, "LCD Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL );

    xTaskCreate(keypad_task, "Keypad Task", USERTASK_STACK_SIZE, NULL, PRIO_HIGH, NULL );
    xTaskCreate(dummy_Task2, "Dummy Task 2", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );


    vTaskStartScheduler();

	return 0;
}
