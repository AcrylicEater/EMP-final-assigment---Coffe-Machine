#ifndef COFFEE_CONTROL_H
#define COFFEE_CONTROL_H
/***************** Header *********************/
/**********************************************
* Univeristy of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME: coffee_control.h
* PROJECT: EMP
* DESCRIPTION: main control and functions for the coffee machine
* Change log:
**********************************************
* Date of Change
* 06/05/2026: MODULE CREATED
**********************************************/

/***************** Include files **************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/semphr.h"
#include "../frt10/inc/queue.h"

/***************** Defines ********************/

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t  hr;
} timestamp_t;

#define TIME_BASE  60
#define SEC_IN_DAY TIME_BASE*TIME_BASE*24


/***************** Functions ******************/

uint32_t get_runtime();
/*****************************************************************************
*   Input    : -
*   Output   : runtime since FreeRTOS scheduler was started
*   Function : calculates the amount of seconds the machine has been on
******************************************************************************/

timestamp_t get_timestamp();
/*****************************************************************************
*   Input    : -
*   Output   : struct containing current second, minute and hour
*   Function : formats the systick count
******************************************************************************/


#endif
