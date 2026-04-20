#include <stdint.h>
#include "tm4c123gh6pm.h"
//####### FreeRTOS includes #######
#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//#include "semphr.h"
//####### project includes #######
#include "LCD_frt.h"


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




void dummy_Task(void *pvParameters){
    char clear_msg = CLEAR_LCD;
    char c = 'a';
    while(1){
        xQueueSend(lcd_queue, &clear_msg, 1000);
        lcd_queueString("CARL TEST: ");
        xQueueSend(lcd_queue, &c, 1000);
        c++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    lcd_queue = xQueueCreate(QUEUE_LEN,sizeof(char));

    xTaskCreate(dummy_Task, "Dummy Task", USERTASK_STACK_SIZE, NULL, PRIO_MID, NULL );
    xTaskCreate(lcd_Task, "LCD Task", USERTASK_STACK_SIZE, NULL, PRIO_LOW, NULL );

    vTaskStartScheduler();

	return 0;
}
