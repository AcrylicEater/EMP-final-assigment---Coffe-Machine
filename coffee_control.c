#include "coffee_control.h"
#include "LCD_frt.h"
#include "keypad_frt.h"
#include "uart_frt.h"
#include "buttons.h"
#include "led.h"
#include "encoder_frt.h"
#include "CoffeeUtils.h"

const uint8_t led_on = ON_STATE;
const uint8_t led_off = OFF_STATE;

const char *options[] = {
    "1:ESPRESSO    kr",
    "2:LATTE       kr",
    "3:FILTER   kr/cl"};


void select_product(MACHINE_STATES_t *state_p, COFFEE_t *selected_product_p)
{
  char key;
  int current_option = 0;

  lcd_queueStringClear("SELECT PRODUCT:");

  xSemaphoreTake(keypad_sem,0); //if has pressed a key before the task was resumed, and old wakeup might wait
  vTaskResume(keypad_task);     //resume the keypad task

  while (*state_p == SEL_PRODUCT)
  {
    lcd_queuePos(LCD_LINE2); // go to start of second line
    lcd_queueString(options[current_option]); //write the name of the current option

    //write the price of current option. Special consideration for filter, as it's price unit is different
    write_price(get_coffee_price(current_option), (current_option == FILTER) ? PRICE_INDEX_F : PRICE_INDEX);

    // wait for key or one second timeout
    if (xQueueReceive(keypad_queue, &key, pdMS_TO_TICKS(PROD_DISP_SPEED)) == pdPASS)
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
      current_option = (current_option + 1) % NOF_PRODUCTS; // cycle through options, loop back at 3
    }
  }
}

void select_payment(MACHINE_STATES_t *state_p)
{
  char key;
  lcd_queueStringClear("1: CARD");
  lcd_queuePos(LCD_LINE2);
  lcd_queueString("2: CASH");

  while (*state_p == SEL_PAYMENT)
  {
    // check key every second
    if (xQueueReceive(keypad_queue, &key, pdMS_TO_TICKS(SECOND)) == pdPASS)
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

void enter_card(MACHINE_STATES_t *state_p, char *card_number)
{
  char key;
  uint8_t sw2_msg;

  lcd_queueStringClear("ENTER NUM & PIN");
  vTaskDelay(pdMS_TO_TICKS(SECOND));
  lcd_queueClear();
  lcd_queuePos(LCD_LINE2);
  lcd_queueString("PIN: ");
  lcd_queuePos(0);
  int i;
  for (i = 0; i < 16; i++) card_number[i] = '\0';
  char pin_code[4];
  int input_index = 0;

  uint8_t offset = 0;

  // Show card numbers as they are typed
  // check key
  while (*state_p == ENTER_CARD)
  {
    if (xQueueReceive(keypad_queue, &key, SECOND / OP_FREQ) == pdPASS) // check for a key press
    {
      if ((key == '*' || key == '#') && input_index) // delete character, make sure we are not at the first character
      { 
          input_index--;

          // if we are entering pin code, we must account for offset from the PIN: string
          offset = (input_index > 15) ? 5 : 0;

          lcd_queuePos(input_index + offset); // update cursor to 1 before current position
          lcd_queueString(" ");               // delete character
          lcd_queuePos(input_index + offset); // update cursor to 1 before current position
      }
      else if (input_index < 20)
      { // normal character, and we are not full
        if (input_index < 16)
          card_number[input_index] = key;
        else
          pin_code[input_index - 16] = key;

        xQueueSend(lcd_queue, &key, portMAX_DELAY);
        if (input_index++ == 15)
        {
          lcd_queuePos(21); // account for the PIN: string
        }
      }
    }

    if (xQueueReceive(SW_2_queue, &sw2_msg, 0) == pdPASS && sw2_msg == STATE_PRESSED)
    { // check for start button press
      if (input_index != 20) //not enough informaton has been put in
      {
        lcd_queueStringClear("NOT ENOUGH INFO");
        vTaskDelay(pdMS_TO_TICKS(SECOND));
        *state_p = SEL_PAYMENT;
      }
      else if ((card_number[15] % 2) == (pin_code[3] % 2)){ // correct card format
        *state_p = WAIT_CUP;
         vTaskSuspend(keypad_task); //suspend the keypad task before leaving this state
      }
      else // invalid card format
      {
        lcd_queueStringClear("INVALID CARD");
        vTaskDelay(pdMS_TO_TICKS(SECOND));
        *state_p = SEL_PAYMENT;
      }
    }
  }
}

void enter_cash(MACHINE_STATES_t *state_p, COFFEE_t *select_product, uint16_t *paid_cash)
{
  uint16_t cash = 0;
  int8_t enc_dir;
  int8_t sw2_msg;

  uint16_t price = get_coffee_price(*select_product);

  // Resume encoder thingy and suspend the keypad
  vTaskSuspend(keypad_task);
  xSemaphoreTake(encoder_sem, 0); // if has turned the encoder before the task was resumed, and old wakeup might wait
  vTaskResume(encoder_task);  // resume the encoder task

  // initial display messages
  lcd_queueStringClear("DEPOSIT CASH");
  lcd_queuePos(LCD_LINE2);
  write_num(cash,&lcd_queue);
  lcd_queueString(" kr");

  // Show cash as the encoder is turned
  while (*state_p == ENTER_CASH)
  {
    if (xQueueReceive(encoder_queue, &enc_dir, SECOND / OP_FREQ) == pdPASS)
    {
      cash += (enc_dir > 0) ? CW_CASH : CCW_CASH;
      lcd_queuePos(LCD_LINE2);
      write_num(cash,&lcd_queue);
      lcd_queueString(" kr");
    }

    if (xQueueReceive(SW_2_queue, &sw2_msg, 0) == pdPASS && sw2_msg == STATE_PRESSED)
    {
        if (cash < price)
        {
          lcd_queueStringClear("NOT ENOUGH CASH");
          vTaskDelay(pdMS_TO_TICKS(SECOND));
          lcd_queueStringClear("DEPOSIT CASH");
          lcd_queuePos(LCD_LINE2);
          write_num(cash,&lcd_queue);
          lcd_queueString(" kr");
        }
        else
        {
          *state_p = ((cash > price) && *select_product != FILTER) ? RETURN_CHANGE : WAIT_CUP;
          *paid_cash = cash;
        }
    }
  }
  vTaskSuspend(encoder_task); //stop the encoder task again before we leave this state
}

void return_change(MACHINE_STATES_t *state_p, uint16_t change)
{
  lcd_queueClear();
  lcd_queueString("RETURNING CHANGE");

  while (*state_p == RETURN_CHANGE)
  {
    if (!change--)
      return (void)(*state_p = WAIT_CUP);
    xQueueOverwrite(green_led_queue, &led_on);
    vTaskDelay(pdMS_TO_TICKS(CHANGE_PERIOD));
    xQueueOverwrite(green_led_queue, &led_off);
    vTaskDelay(pdMS_TO_TICKS(CHANGE_PERIOD));
  }
}

void wait_for_cup(MACHINE_STATES_t *state_p, COFFEE_t *selected_product)
{
  uint8_t sw1_msg;
  lcd_queueStringClear("PLACE CUP");

  while (*state_p == WAIT_CUP)
  {
    if (xQueueReceive(SW_1_queue, &sw1_msg, portMAX_DELAY) == pdPASS && sw1_msg == STATE_PRESSED)
    {
      switch (*selected_product)
      {
      case ESPRESSO:
        *state_p = BREW_ESPRESSO;
        break;
      case LATTE:
        *state_p = BREW_ESPRESSO;
        break;
      case FILTER:
        *state_p = BREW_FILTER;
        break;
      }
    }
  }
}

void brew_espresso(MACHINE_STATES_t *state_p, COFFEE_t *selected_product)
{
  uint8_t sw1_msg;
  const char loading_char = LOADING_CHAR;

  lcd_queueStringClear("GRINDING BEANS");
  lcd_queuePos(LCD_LINE2);
  xQueueOverwrite(yellow_led_queue, &led_on);

  uint8_t work_count = 0;

  TickType_t lastWakeTime = xTaskGetTickCount();
  while (*state_p == BREW_ESPRESSO)
  {
    uint16_t time_offset = (work_count < LCD_LINE2) ? GRIND_INTERVAL : BREW_INTERVAL;
    if (xQueueReceive(SW_1_queue, &sw1_msg, 0) == pdPASS && sw1_msg == STATE_RELEASED)
    {
        lcd_queueStringClear("BREWING STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        xQueueOverwrite(red_led_queue, &led_off);
        vTaskDelay(pdMS_TO_TICKS(SECOND));
        *state_p = FINISH_PROD;
    }

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(time_offset));
    xQueueSend(lcd_queue, &loading_char, portMAX_DELAY);
    work_count++;

    if (work_count == LCD_LINE2)
    {
      lcd_queueStringClear("BREWING COFFEE");
      lcd_queuePos(LCD_LINE2);
      xQueueOverwrite(red_led_queue, &led_on);
      xQueueOverwrite(yellow_led_queue, &led_off);
    }
    else if (work_count == 2*LCD_LINE2)
    {
      xQueueOverwrite(red_led_queue, &led_off);
      *state_p = (*selected_product == LATTE) ? FROTH_MILK : REMOVE_CUP;
    }
  }
}

void froth_milk(MACHINE_STATES_t *state_p)
{
  uint8_t sw1_msg;
  const char loading_char = LOADING_CHAR;

  lcd_queueStringClear("STEAMING MILK");
  lcd_queuePos(LCD_LINE2);
  xQueueOverwrite(green_led_queue, &led_on);

  uint8_t work_count = 0;

  TickType_t lastWakeTime = xTaskGetTickCount();

  while (*state_p == FROTH_MILK)
  {
    if (xQueueReceive(SW_1_queue, &sw1_msg, 0) == pdPASS && sw1_msg == STATE_RELEASED)
    {
        lcd_queueStringClear("BREWING STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        xQueueOverwrite(red_led_queue, &led_off);
        vTaskDelay(pdMS_TO_TICKS(SECOND));
        *state_p = FINISH_PROD;
    }

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(FROTH_INTERVAL));
    xQueueSend(lcd_queue, &loading_char, portMAX_DELAY);
    work_count++;

    if (work_count == LCD_LINE2)
    {
      xQueueOverwrite(green_led_queue, &led_off);
      *state_p = REMOVE_CUP;
    }
  }
}


void brew_filter(MACHINE_STATES_t *state_p, uint16_t paid_cash, uint16_t *amount)
{
  uint8_t sw1_msg;
  uint8_t sw2_msg;
  TickType_t timeout_stamp = 0;

  uint8_t producing = TRUE;

  //if we pay with card, paid cash is 0 and there is no max prepaid amount
  uint16_t max_amount = (!paid_cash) ? 0xFFFF : ( paid_cash / get_coffee_price(FILTER));
  double amount_precise = 0.0f;

  lcd_queueStringClear("DISPENSE COFFEE");
  lcd_queuePos(LCD_LINE2);
  write_num(0, &lcd_queue);
  lcd_queueString(" ml");
  xQueueOverwrite(yellow_led_queue, &led_on);

  TickType_t worktime_stamp = xTaskGetTickCount();
  while (*state_p == BREW_FILTER)
  {
    //check if cup was removed
    if (xQueueReceive(SW_1_queue, &sw1_msg, 0) == pdPASS && sw1_msg == STATE_RELEASED)
    {
        lcd_queueStringClear("BREWING STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        xQueueOverwrite(red_led_queue, &led_off);
        vTaskDelay(pdMS_TO_TICKS(SECOND));
        *state_p = (!producing) ? FINISH_PROD : FINISH_PROD;
    }

    vTaskDelayUntil(&worktime_stamp, pdMS_TO_TICKS(SECOND / OP_FREQ)); //wait for a filter cycle
    if(producing) //here we will produce coffee until sw2 is released or max amount is reached
    { 
      amount_precise += (amount_precise < SPEED_CHANGE) ? START_SPEED : LATER_SPEED; //calculate the amount dispensed, take account for the speed up
      if(amount_precise > max_amount){ //check if we have reached the max amount
        amount_precise = max_amount;
        lcd_queueStringClear("CASH EXHAUSTED");
        *state_p = REMOVE_CUP; //if so, stop dispensing
      } else{
        lcd_queuePos(LCD_LINE2);
        write_num((uint16_t)(amount_precise*10.0f), &lcd_queue);
        lcd_queueString(" ml");
      }

      //if sw2 is released, pause dispensing
      if (xQueueReceive(SW_2_queue, &sw2_msg, 0) == pdPASS && sw2_msg == STATE_RELEASED)
      {
        lcd_queuePos(LCD_LINE1); 
        lcd_queueString("DISPENSE STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        timeout_stamp = xTaskGetTickCount();
        producing = FALSE; 
      }
    }
    else{ //Here we will wait until SW2 is pressed or 5 seconds have passed
      if (xTaskGetTickCount() - timeout_stamp >= pdMS_TO_TICKS(FILTER_TIMEOUT)){
          lcd_queueStringClear("PRODUCT FINISHED");
          *state_p = REMOVE_CUP;
      } 
      else if (xQueueReceive(SW_2_queue, &sw2_msg, SECOND / OP_FREQ) == pdPASS && sw2_msg == STATE_PRESSED)
      {
        lcd_queuePos(LCD_LINE1);
        lcd_queueString("DISPENSE COFFEE ");
        xQueueOverwrite(yellow_led_queue, &led_on);
        producing = TRUE;
      }
    }
  }

  //before we leave this state
  *amount = (uint16_t)amount_precise;

  lcd_queuePos(LCD_LINE2);
  write_num(*amount,&lcd_queue);
  lcd_queueString(" cl / ");
  write_num( *amount * get_coffee_price(FILTER), &lcd_queue );
  lcd_queueString(" kr");
  vTaskDelay(pdMS_TO_TICKS(2*SECOND));
}


void remove_cup(MACHINE_STATES_t* state_p)
{
  uint8_t sw1_msg;

  lcd_queueStringClear("REMOVE THE CUP");
  while (*state_p == REMOVE_CUP)
    if (xQueueReceive(SW_1_queue, &sw1_msg, SECOND / OP_FREQ) == pdPASS && sw1_msg == STATE_RELEASED) *state_p = FINISH_PROD;
 }


void finish_prod(MACHINE_STATES_t* state_p, COFFEE_t* selected_product, char *card_number, uint16_t *paid_cash, uint16_t *amount){
  timestamp_t time = get_timestamp();

  uart0_queueString("PRODUCT: ");
  switch(*selected_product){
    case ESPRESSO:
      increment_operating_data(NOF_ESPRESSO, 1);
      uart0_queueString("ESPRESSO\n");
      break;
    case LATTE:
      increment_operating_data(NOF_LATTE, 1);
      uart0_queueString("LATTE\n");
      break;
    case FILTER:
      increment_operating_data(NOF_FILTER, 1);
      uart0_queueString("FILTER\n");
      break;
  }

  uart0_queueString("TIME: ");
  write_num(time.hr,&uart_tx_queue);
  uart0_queueString(":");
  write_num(time.min,&uart_tx_queue);
  uart0_queueString(":");
  write_num(time.sec,&uart_tx_queue);
  uart0_queueString("\n");

  uart0_queueString("AMOUNT: ");
  write_num(*amount,&uart_tx_queue);
  uart0_queueString("\n");

  uint16_t coffee_price = *amount * get_coffee_price(*selected_product);
  uart0_queueString("PAYMENT: ");
  if(*paid_cash){
    increment_operating_data(CASH_AMOUNT, coffee_price);
    uart0_queueString("CASH\n");
  }else{
    increment_operating_data(CARD_AMOUNT, coffee_price);
    uart0_queueString(card_number);
  }
  uart0_queueString("PRICE: ");
  write_num(coffee_price, &uart_tx_queue);
  uart0_queueString("\n");

  *state_p = SEL_PRODUCT;
}

void userflow_Task(void *pvParameters)
{
  MACHINE_STATES_t state = SEL_PRODUCT;
  COFFEE_t selected_product;
  uint16_t paid_cash;
  char card_number[16];
  uint16_t amount;
  
  while (1)
  {
    switch (state)
    {
    case SEL_PRODUCT:
      paid_cash = 0;
      select_product(&state, &selected_product);
      break;
    case SEL_PAYMENT:
      select_payment(&state);
      break;
    case ENTER_CARD:
      enter_card(&state, card_number);
      break;
    case ENTER_CASH:
      enter_cash(&state, &selected_product, &paid_cash);
      break;
    case RETURN_CHANGE:
      return_change(&state, paid_cash - get_coffee_price(selected_product));
      break;
    case WAIT_CUP:
      wait_for_cup(&state, &selected_product);
      break;
    case BREW_ESPRESSO:
      amount = 1;
      brew_espresso(&state, &selected_product);
      break;
    case FROTH_MILK:
      froth_milk(&state);
      break;
    case BREW_FILTER:
      brew_filter(&state, paid_cash, &amount);
      break;
    case REMOVE_CUP:
      remove_cup(&state);
      break;
    case FINISH_PROD:
      finish_prod(&state, &selected_product, card_number, &paid_cash, &amount);
      break;
    }
  }
}
