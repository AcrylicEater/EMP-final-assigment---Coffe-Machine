/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: uart_frt.h
*
* PROJECT....: EMP
*
* DESCRIPTION: Test.
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260503  MoH   Module derievd from uart0.
*
*****************************************************************************/
#ifndef _UART_H
  #define _UART_H


/***************************** Include files *******************************/
#include <tm4c123gh6pm.h>
#include <stdint.h>
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/queue.h"
/*****************************    Defines    *******************************/
#define UART_TX_PIN 0b00000010
#define UART_RX_PIN 0b00000001

#define TX_QUEUE_LEN     32

QueueHandle_t uart_tx_queue;

typedef enum { FALSE, TRUE } bool_t;

typedef enum {
    DBITS_5 = 0x00,
    DBITS_6 = 0x20,
    DBITS_7 = 0x40,
    DBITS_8 = 0x60
} NOF_DATABITS_t;

typedef enum  {
    SBIT_1 = 0x00,
    SBIT_2 = 0x08
}NOF_STOPBITS_t;

typedef enum{
    NO_PARITY    = 0x00,
    EVEN_PARITY  = 0x06,
    ODD_PARITY   = 0x02,
    MARK_PARITY  = 0x82,
    SPACE_PARITY = 0x86
} PARITY_t;




/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/

bool_t uart0_rx_rdy();
/*****************************************************************************
*   Input    : -
*   Output   : True or false, depending on ready
*   Function : Character ready at uart0 RX
******************************************************************************/

uint8_t uart0_getc();
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Get character from uart0 RX
******************************************************************************/

bool_t uart0_tx_rdy();
/*****************************************************************************
*   Input    : -
*   Output   : True or false, depending on ready
*   Function : uart0 TX buffer ready
******************************************************************************/

void uart0_putc(char ch);
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : Put character to uart0 TX
******************************************************************************/

void uart0_init( uint32_t baud_rate, NOF_DATABITS_t databits, NOF_STOPBITS_t stopbits, PARITY_t parity );
/*****************************************************************************
*   Input    : baudrate for communications, number of databits and stop bits aswell as parity options
*   Output   : -
*   Function : Initialize uart 0
******************************************************************************/

void uart0_queueString(char* string);
/**********************************************
* Input: String to send to queue
* Output: none
* Function: writes to characters to uart queue
***********************************************/

void uart_tx_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: none
* Function: writes to uart from queue
***********************************************/

//bool_t uart0_update(void);

char* readString(void);

//void writeString(char*);

/****************************** End Of Module *******************************/
#endif

