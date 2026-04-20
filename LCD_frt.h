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
* 25/02/2026 creation of base lcd library
* xx/03/2026 modification to FreeRTOS
* 13/04/2026 creation of lcd task
**********************************************/
/***************** Include files **************/
#ifndef LCD_FRT_H
#define LCD_FRT_H
#include <tm4c123gh6pm.h>
#include <stdint.h>
#include "../frt10/inc/FreeRTOS.h"
#include "../frt10/inc/task.h"
#include "../frt10/inc/queue.h"

/***************** Defines ********************/
#define LCD_DATA 0b11110000
#define LCD_RS   0b00000100
#define LCD_EN   0b00001000

#define TIMEOUT       1000
#define QUEUE_LEN     32
#define CLEAR_LCD     0x7F

QueueHandle_t lcd_queue;

/***************** Functions ******************/


void lcd_command(uint8_t cmd);
/**********************************************
* Input: 8bit command to send to lcd
* Output: none
* Function: sends command to lcd
***********************************************/
void lcd_init();
/**********************************************
* Input: none
* Output: none
* Function: initializes LCD functionality
***********************************************/
void lcd_char(uint8_t character);
/**********************************************
* Input: character to write
* Output: none
* Function: writes a single character at lcd cursor position
***********************************************/
void lcd_string(uint8_t *str);
/**********************************************
* Input: string to write
* Output: none
* Function: writes a string at cursor position
***********************************************/
void lcd_clear();
/**********************************************
* Input: none
* Output: none
* Function: clears display and resets cursor position
***********************************************/
void lcd_setcursor(uint8_t pos);
/**********************************************
* Input: cursor position
* Output: none
* Function: sets cursor position
***********************************************/
uint8_t lcd_getcursor();
/**********************************************
* Input: none
* Output: current cursor position
* Function: returns cursor position
***********************************************/

void lcd_queueString(char* string);
/**********************************************
* Input: String to send to queue
* Output: none
* Function: writes to characters to lcd queue
***********************************************/

void lcd_Task(void *pvParameters);
/**********************************************
* Input: none
* Output: none
* Function: writes to the lcd from queue
***********************************************/




#endif /* LCD_H_ */
