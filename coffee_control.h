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
#define SECOND          1000 

#define NOF_PRODUCTS    3
#define PROD_DISP_SPEED 2500
#define PRICE_INDEX     28
#define PRICE_INDEX_F   25

#define CW_CASH         20
#define CCW_CASH        5

#define CHANGE_PERIOD  250

#define LOADING_CHAR  0xFF

#define GRIND_INTERVAL 7500 / 16
#define BREW_INTERVAL 14000 / 16
#define FROTH_INTERVAL 6200 / 16

#define FILTER_TIMEOUT 5000

#define OP_FREQ     10
#define START_SPEED 0.6 / OP_FREQ
#define LATER_SPEED 14.5 / OP_FREQ
#define SPEED_CHANGE 3*START_SPEED*OP_FREQ

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
*   Function : Main control task for the userflow
******************************************************************************/

void select_product(MACHINE_STATES_t *state_p, COFFEE_t *selected_product_p);
/*****************************************************************************
*   Input    : pointer to state and to selected product
*   Output   : writes to lcd and selects product
*   Function : state function for selecting product
******************************************************************************/

void select_payment(MACHINE_STATES_t *state_p);
/*****************************************************************************
*   Input    : pointer to state
*   Output   : writes to lcd
*   Function : state function for selecting payment type
******************************************************************************/

void enter_card(MACHINE_STATES_t *state_p, char *card_number);
/*****************************************************************************
*   Input    : pointer to state and the card number
*   Output   : writes to lcd and card number
*   Function : state function for entering card details
******************************************************************************/

void enter_cash(MACHINE_STATES_t *state_p, COFFEE_t *select_product, uint16_t *paid_cash);
/*****************************************************************************
*   Input    : pointer to state, selected product and the paid amount of cash
*   Output   : writes to lcd and saves the amount of paid cash
*   Function : state function for inputting cash
******************************************************************************/

void return_change(MACHINE_STATES_t *state_p, uint16_t change);
/*****************************************************************************
*   Input    : pointer to state, and the amount of change to be returned
*   Output   : writes to lcd and blinks green LED
*   Function : state function for returning change
******************************************************************************/

void wait_for_cup(MACHINE_STATES_t *state_p, COFFEE_t *selected_product);
/*****************************************************************************
*   Input    : pointer to state and selected product
*   Output   : writes to lcd
*   Function : state function for prompting the user to place cup
******************************************************************************/

void brew_espresso(MACHINE_STATES_t *state_p, COFFEE_t *selected_product);
/*****************************************************************************
*   Input    : pointer to state and selected product
*   Output   : writes to lcd and controls red and yellow LED
*   Function : state function for brewing espresso
******************************************************************************/

void froth_milk(MACHINE_STATES_t *state_p);
/*****************************************************************************
*   Input    : pointer to state
*   Output   : writes to lcd and controls green LED
*   Function : state function for frothing milk for latte
******************************************************************************/


void brew_filter(MACHINE_STATES_t *state_p, uint16_t paid_cash, uint16_t *amount);
/*****************************************************************************
*   Input    : pointer to state, the amount of paid cash and a pointer to the amount of product created
*   Output   : writes to lcd and controls red LED, aswell as saving amount produced
*   Function : state function for brewing filter coffee
******************************************************************************/

void remove_cup(MACHINE_STATES_t* state_p);
/*****************************************************************************
*   Input    : pointer to state
*   Output   : writes to lcd
*   Function : state function for prompting the user to remove their cup
******************************************************************************/

void finish_prod(MACHINE_STATES_t* state_p, COFFEE_t* selected_product, char *card_number, uint16_t *paid_cash, uint16_t *amount);
/*****************************************************************************
*   Input    : pointer to state, selected product, card number and amount of paidcash+product produced
*   Output   : writes a transaction report to UART
*   Function : state function for ending production by sending a report to uart
******************************************************************************/

#endif
