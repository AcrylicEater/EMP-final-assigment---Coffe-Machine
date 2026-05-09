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
#define GRIND_INTERVAL 7500 / 16
#define BREW_INTERVAL 14000 / 16

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
} MACHINE_STATES_t;

typedef enum {
    ESPRESSO = 0,
    LATTE    = 1,
    FILTER   = 2
} COFFEE_t;

SemaphoreHandle_t price_wr_mutex;


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

void set_coffee_price(COFFEE_t product, uint8_t price);
/*****************************************************************************
*   Input    : new price to set and the product target
*   Output   : -
*   Function : freertos shared memory safe function to update coffee_price
******************************************************************************/

uint8_t get_coffee_price(COFFEE_t product);
/*****************************************************************************
*   Input    : product target
*   Output   : price of product
*   Function : freertos shared memory safe function to get coffee_price
******************************************************************************/


void userflow_Task(void *pvParameters);
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : formats the systick count
******************************************************************************/

#endif
