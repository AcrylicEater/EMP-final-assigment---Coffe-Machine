/***************** Header *********************/
/**********************************************
* Univeristy of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME:  LCD_frt.h
* PROJECT: EMP
* DESCRIPTION: control emp board lcd in a freertos framework
* Change log:
**********************************************
* Date of Change
* 11/05/2026 creation of coffe utils
**********************************************/
/***************** Include files **************/
#ifndef COFFEE_UTILS_H
#define COFFEE_UTILS_H
#include <tm4c123gh6pm.h>
#include <stdint.h>
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/queue.h"
#include "../frt10/inc/semphr.h"
#include "coffee_control.h"

/***************** Defines ********************/
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t  hr;
} timestamp_t;

#define TIME_BASE  60
#define SEC_IN_DAY TIME_BASE*TIME_BASE*24

SemaphoreHandle_t price_wr_mutex;


/***************** functions ********************/
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

void write_price(uint8_t price, char start_index);
/*****************************************************************************
*   Input    : price and the the end of where to write it
*   Output   : writes to lcd_queue
*   Function : writes the price in corret format
******************************************************************************/

void write_num(uint16_t num, QueueHandle_t *queue);
/*****************************************************************************
*   Input    : number and the queue to write it to
*   Output   : writes to queue
*   Function : write a number to the queue as characters
******************************************************************************/

#endif
