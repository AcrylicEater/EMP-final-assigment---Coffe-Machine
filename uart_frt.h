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
#include "../frt10/inc/semphr.h"
/*****************************    Defines    *******************************/
#define UART_TX_PIN 0b00000010
#define UART_RX_PIN 0b00000001

#define TX_QUEUE_LEN       32
#define RX_BUFFER_LEN      32

#define UART_NVIC_INT      0x20
#define UART_INT_PRIO_MASK 0xE000
#define UART_INT_PRIO      4

QueueHandle_t   uart_tx_queue;
SemaphoreHandle_t uart_rx_sem;

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


bool_t uart0_tx_rdy();
/*****************************************************************************
*   Input    : -
*   Output   : True or false, depending on ready
*   Function : uart0 TX buffer ready
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

void uart_rx_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: none
* Function: handles commands recieved from uart
***********************************************/

void UART0_int_handler(void);
/**********************************************
* Input: none
* Output: none
* Function: recieves single characters from uart RX and activates task on completion of message
***********************************************/







/****************************** End Of Module *******************************/
#endif

