#include "coffee_control.h"
#include "LCD_frt.h"
#include "keypad_frt.h"
#include "uart_frt.h"

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
    "1:ESPRESSO    kr",
    "2:LATTE       kr",
    "3:FILTER   kr/cl"
};


uint8_t prices[] = {15, 27, 3};

uint8_t get_coffee_price(COFFEE_t product){
    xSemaphoreTake(price_wr_mutex, portMAX_DELAY); //take the mutex, to ensure other tasks dont write to it
    uint8_t price = prices[product];
    xSemaphoreGive(price_wr_mutex);
    return price;
}

void set_coffee_price(COFFEE_t product, uint8_t price){
    xSemaphoreTake(price_wr_mutex, portMAX_DELAY); //take the mutex, to ensure other tasks dont write to it
    prices[product] = price;
    xSemaphoreGive(price_wr_mutex);
}

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

        write_price(get_coffee_price(current_option), (current_option == 2) ? 25 : 28);

    // wait for key or one second timeout
    if (xQueueReceive(keypad_queue, &key, pdMS_TO_TICKS(2500)) == pdPASS)
    {
      switch (key)
      {
      case '1':
        *selected_product_p = ESPRESSO;
        *state_p = SEL_PAYMENT;
        uart0_queueString("ESPRESSO\n");
        break;
      case '2':
        *selected_product_p = LATTE;
        *state_p = SEL_PAYMENT;
        uart0_queueString("LATTE\n");
        break;
      case '3':
        *selected_product_p = FILTER;
        *state_p = SEL_PAYMENT;
        uart0_queueString("FILTER\n");
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
  lcd_queueString("1: CARD");
  msg = 16; // command to go to start of second line
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString("2: CASH");

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

void enter_card(MACHINE_STATES_t *state_p)
{
  // Clear display
  char msg = CLEAR_LCD;
  char key;
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString("ENTER NUM & PIN");
  vTaskDelay(pdMS_TO_TICKS(1500));
  xQueueSend(lcd_queue, &msg, 1000);
  msg = 16;
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString("PIN: ");
  msg = 0;
  xQueueSend(lcd_queue, &msg, 1000);

  char card_number[16];
  char pin_code[4];
  int input_index = 0;

  uint8_t offset = 0;

    // Show card numbers as they are typed
    // check key
    while(*state_p == ENTER_CARD){
        if (xQueueReceive(keypad_queue, &key, portMAX_DELAY) == pdPASS)
        {
            if((key == '*') || (key == '#')){ // delete character
              if(input_index){
                input_index--;

                if(input_index > 15){ //if we are entering pin code, we must account for offset from the PIN: string
                    offset = 5;
                } else{
                    offset = 0;
                }

                msg = input_index + offset;
                xQueueSend(lcd_queue, &msg, 1000); //update cursor to 1 before current position
                msg = ' ';
                xQueueSend(lcd_queue, &msg, 1000); //clear current character
                msg = input_index + offset;
                xQueueSend(lcd_queue, &msg, 1000); //update cursor to 1 before current position
              }
             } else { //normal character

              if(input_index < 16){
                card_number[input_index] = key;
              } else{
                  pin_code[input_index - 16] = key;
              }

              xQueueSend(lcd_queue, &key, 1000);
              input_index++;
              if(input_index == 16){
                  msg = 21;
                  xQueueSend(lcd_queue, &msg, 1000);
              } else if(input_index == 20){
                  if((card_number[15] % 2) == (pin_code[3] % 2)){
                      *state_p = WAIT_CUP;
                  } else{
                      msg = CLEAR_LCD;
                      xQueueSend(lcd_queue, &msg, 1000);
                      lcd_queueString("INVALID CARD");
                  }
              }

             }

        }
    }
}

void enter_cash(MACHINE_STATES_t *state_p)
{
  // Resume encoder thingy

  // Clear display
  char msg = CLEAR_LCD;
  char key;
  xQueueSend(lcd_queue, &msg, 1000);

  // Show cash as the encoder is turned
  while (*state_p == ENTER_CARD)
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
/*
void wait_for_cup(MACHINE_STATES_t* state_p, COFFEE_t* selected_product){
    char msg = CLEAR_LCD;
    uint8_t SW_1_state;
    xQueueSend(lcd_queue, &msg, 1000);
    lcd_queueString("PLACE CUP");

    while(*state_p == WAIT_CUP){
        if(xQueueReceive(SW_1_queue, &SW_1_state, portMAX_DELAY) == pdPASS){
            if(SW_1_state == STATE_PRESSED){
                switch(*selected_product){
                    case ESPRESSO:
                        *state_p = BREW_ESPRESSO;
                        break;
                    case LATTE:
                        *state_p = BREW_LATTE;
                        break;
                    case FILTER:
                        *state_p = BREW_FILTER;
                        break;
                }
            }
        }
    }
}
*/
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
    case ENTER_CARD:
      enter_card(&state);
      break;
    case ENTER_CASH:
      enter_cash(&state);
      break;
    case WAIT_CUP:
      //wait_for_cup(&state, &selected_produdct);
      break;
    }
  }
}
