#ifndef ENCODER_FRT_H
#define ENCODER_FRT_H
/***************** Header *********************/
/**********************************************
* Univeristy of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME: encoder_frt.h
* PROJECT: EMP
* DESCRIPTION: FreeRTOS driver for encoder on emp board
* Change log:
**********************************************
* Date of Change
* 13/04/2026: MODULE CREATED
**********************************************


/***************** Include files **************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/semphr.h"
#include "../frt10/inc/queue.h"

/***************** Defines ********************/
#define DIGI_A_PIN  0b00100000
#define DIGI_B_PIN  0b01000000


#define GPIOA_INTERRUPT    0b00000001
#define GPIOA_INTPRIO_MASK 0b11100000

#define GPIOA_INTPRIO 5 //priority of interrupt

QueueHandle_t encoder_queue; //Muligvis midlertidig
SemaphoreHandle_t encoder_sem;


/***************** Functions ******************/

void enc_init(void);
/**********************************************
* Input: none
* Output: none
* Function: setup TIVA hardware for encoder
***********************************************/

void encoder_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: pulses on queue
* Function: parses pulses and direction of pulse of encoder
***********************************************/


void GPIOA_int_handler(void);
/**********************************************
* Input: none
* Output: none
* Function: activate encoder direction parsing task
***********************************************/

#endif
