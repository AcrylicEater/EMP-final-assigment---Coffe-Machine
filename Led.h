#ifndef LED_H
#define LED_H
/***************** Header *********************/
/**********************************************
* Univeristy of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME: led.h
* PROJECT: EMP
* DESCRIPTION: LED driver for emp board
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
#include "../frt10/inc/semphr.h"
#include "../frt10/inc/queue.h"

/***************** Defines ********************/
#define GREEN_LED_PIN   0b00100000
#define YELLOW_LED_PIN  0b01000000
#define RED_LED_PIN     0b10000000
#define LED_MASK        0b00001110


/***************** Functions ******************/

void Led_init(void);
/**********************************************
* Input: none
* Output: none
* Function: setup TIVA hardware for LED
***********************************************/

void Green_LED_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: none
* Function: activate green LED task
***********************************************/


void Yellow_LED_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: none
* Function: activate yellow LED task
***********************************************/

void Red_LED_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: none
* Function: activate red LED task
***********************************************/

#endif
