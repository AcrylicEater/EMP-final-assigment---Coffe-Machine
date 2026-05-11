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


#define PROD_DISP_SPEED 2500
#define PRICE_INDEX     28
#define PRICE_INDEX_F   25

#define GRIND_INTERVAL 7500 / 16
#define BREW_INTERVAL 14000 / 16
#define FROTH_INTERVAL 6200 / 16

#define FILTER_FREQ 10
#define START_SPEED 0.6 / FILTER_FREQ
#define LATER_SPEED 14.5 / FILTER_FREQ
#define SPEED_CHANGE 3*START_SPEED*FILTER_FREQ 


typedef enum {
    SEL_PRODUCT,
    SEL_PAYMENT,
    ENTER_CASH,
    ENTER_CARD,
    RETURN_CHANGE,
    WAIT_CUP,
    BREW_ESPRESSO,
    FROTH_MILK,
    BREW_FILTER,
    REMOVE_CUP,
    CUP_ABORTED,
    FINISH_PROD
} MACHINE_STATES_t;

typedef enum {
    ESPRESSO = 0,
    LATTE    = 1,
    FILTER   = 2
} COFFEE_t;



/***************** Functions ******************/

void userflow_Task(void *pvParameters);
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : formats the systick count
******************************************************************************/

#endif
