#ifndef BUTTONS_H
#define BUTTONS_H
/***************** Header *********************/
/**********************************************
* Univeristy of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME: buttons.h
* PROJECT: EMP
* DESCRIPTION: FreeRTOS driver for buttons on emp board
* Change log:
**********************************************
* Date of Change
* 13/04/2026: MODULE CREATED
**********************************************/


/***************** Include files **************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/queue.h"

/***************** Defines ********************/
#define SW_1_PIN            0b00010000
#define SW_2_PIN            0b00000001
#define BUTTONS_MASK        0b00010001
#define STATE_PRESSED       1
#define STATE_RELEASED      0
#define DEBOUNCE_DELAY_MS   50


QueueHandle_t SW_1_queue; 
QueueHandle_t SW_2_queue; 

/***************** Functions ******************/

void buttons_init(void);
/**********************************************
* Input: none
* Output: none
* Function: setup TIVA hardware for buttons
***********************************************/

void SW_1_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: press on queue
* Function: parsed key release and press for SW1
***********************************************/

void SW_2_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: press on queue
* Function: parsed key release and press for SW1
***********************************************/
#endif
