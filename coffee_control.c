#include "coffee_control.h"
#include "LCD_frt.h"
#include "keypad_frt.h"

uint32_t get_runtime()
{
  return xTaskGetTickCount() / configTICK_RATE_HZ;
}

timestamp_t get_timestamp()
{
  uint32_t all_seconds = get_runtime(); // get total amount of run time seconds since

  if (all_seconds > SEC_IN_DAY)
  {                            // we only need the time of day, if we run for more than one day
    all_seconds -= SEC_IN_DAY; // remove the extra day
  }

  timestamp_t return_val;
  return_val.sec = all_seconds % TIME_BASE;
  return_val.min = (all_seconds / TIME_BASE) % TIME_BASE;
  return_val.hr = all_seconds / (TIME_BASE * TIME_BASE);

  return return_val;
}


const char *options[] = {
    "1: ESPRESSO   kr",
    "2: LATTE      kr",
    "3: FILTER  kr/cl"
};

const char *payments[] = {
    "1: CARD",
    "2: CASH"
};

const uint8_t prices[] = {15, 27, 3};

void write_price(uint8_t price, char start_index){
    char ch;
    uint16_t div = 1;
    while(price / div > 0){
      xQueueSend(lcd_queue, &start_index, 1000);
      ch = (((price / div) % 10) + '0');
      xQueueSend(lcd_queue, &ch, 1000);
      div *= 10;
      start_index -= 1;
    }
}


void select_product(MACHINE_STATES_t* state_p, COFFEE_t* selected_product_p){
    char key;
    char msg = CLEAR_LCD;
    xQueueSend(lcd_queue, &msg, 1000);
    lcd_queueString("SELECT PRODUCT:");
    msg = 16; //command to go to start of second line
    int current_option = 0;

    while(*state_p==SEL_PRODUCT){
        xQueueSend(lcd_queue, &msg, 1000); // go to start of second line
        lcd_queueString(options[current_option]);
        write_price(prices[current_option], (current_option == 2) ? 26 : 29);

    // wait for key or one second timeout
    if (xQueueReceive(keypad_queue, &key, pdMS_TO_TICKS(1000)) == pdPASS)
    {
      switch (key)
      {
      case '1':
        *selected_product_p = ESPRESSO;
        *state_p = SEL_PAYMENT;
        break;
      case '2':
        *selected_product_p = LATTE;
        *state_p = SEL_PAYMENT;
        break;
      case '3':
        *selected_product_p = FILTER;
        *state_p = SEL_PAYMENT;
        break;
      default:
        break;
      }
    }
    else
    {
      current_option = (current_option + 1) % 3;
    }
  }
}

void select_payment(MACHINE_STATES_t *state_p)
{
  char key;
  char msg = CLEAR_LCD;
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString(payments[0]);
  msg = 16; // command to go to start of second line
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString(payments[1]);

  while (*state_p == SEL_PAYMENT)
  {
    // check key every second
    if (xQueueReceive(keypad_queue, &key, pdMS_TO_TICKS(1000)) == pdPASS)
    {
      switch (key)
      {
      case '1':
        *state_p = ENTER_CARD;
        break;
      case '2':
        *state_p = ENTER_CASH;
        break;
      default:
        break;
      }
    }
  }
}

void userflow_Task(void *pvParameters)
{
  MACHINE_STATES_t state = SEL_PRODUCT;
  COFFEE_t selected_product;


  while (1)
  {
    switch (state)
    {
    case SEL_PRODUCT:
      select_product(&state, &selected_product);
      break;
    case SEL_PAYMENT:
      select_payment(&state);
      break;
    }
  }
}
