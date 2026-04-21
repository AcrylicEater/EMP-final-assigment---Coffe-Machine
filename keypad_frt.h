/***************** Header *********************/
/**********************************************
* Univeristy of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME: keypad.h
* PROJECT: EMP
* DESCRIPTION: driver for EMP board keypad
* Change log:
**********************************************
* Date of Change
* 16/03/2026: MODULE CREATED
**********************************************


/***************** Include files **************/
#ifndef KEYPAD_FRT_H
#define KEYPAD_FRT_H
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/queue.h"
#include "../frt10/inc/semphr.h"
/***************** Defines ********************/
#define KEYPAD_X_MASK 0b00011100
#define KEYPAD_Y_MASK 0b00001111



#define STATE_RELEASED 0
#define STATE_PRESSED 1 

QueueHandle_t       keypad_queue;
SemaphoreHandle_t   keypad_sem;



/***************** Functions ******************/
void keypad_init(void);
/**********************************************
* Input: none
* Output: none
* Function: setup TIVA hardware for keypad
***********************************************/

void keypad_task(void *pvParameters);
/**********************************************
* Input: none
* Output: may write to queue
* Function: run task for keypad
***********************************************/

void GPIOE_int_Handler(void);

/***************** End of module **************/
#endif
