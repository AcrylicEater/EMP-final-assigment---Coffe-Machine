#include "coffee_control.h"
#include "LCD_frt.h"
#include "keypad_frt.h"
#include "uart_frt.h"
#include "buttons.h"
#include "led.h"
#include "encoder_frt.h"

uint32_t get_runtime()
{
  return xTaskGetTickCount() / configTICK_RATE_HZ;
}

timestamp_t get_timestamp()
{
  uint32_t all_seconds = get_runtime(); // get total amount of run time seconds since

  // Shorten to entire days
  all_seconds %= SEC_IN_DAY;

  timestamp_t return_val;
  return_val.sec = all_seconds % TIME_BASE;
  return_val.min = (all_seconds / TIME_BASE) % TIME_BASE;
  return_val.hr = all_seconds / (TIME_BASE * TIME_BASE);

  return return_val;
}

const char *options[] = {
    "1:ESPRESSO    kr",
    "2:LATTE       kr",
    "3:FILTER   kr/cl"};

uint8_t prices[] = {15, 27, 3};

uint8_t get_coffee_price(COFFEE_t product)
{
  xSemaphoreTake(price_wr_mutex, portMAX_DELAY); // take the mutex, to ensure other tasks dont write to it
  uint8_t price = prices[product];
  xSemaphoreGive(price_wr_mutex);
  return price;
}

void set_coffee_price(COFFEE_t product, uint8_t price)
{
  xSemaphoreTake(price_wr_mutex, portMAX_DELAY); // take the mutex, to ensure other tasks dont write to it
  prices[product] = price;
  xSemaphoreGive(price_wr_mutex);
}

void write_price(uint8_t price, char start_index)
{
  char ch;
  uint16_t div = 1;
  while (price / div > 0)
  {
    xQueueSend(lcd_queue, &start_index, 1000);
    ch = (((price / div) % 10) + '0');
    xQueueSend(lcd_queue, &ch, 1000);
    div *= 10;
    start_index -= 1;
  }
}

void write_num(uint16_t num, QueueHandle_t *queue)
{
  char digit_buf[17]; // buffer for digits, index will start at 1, which is why it is 17
  int index = 0;
  // parse the number to chars, this will be in reverse, so the largest digit is last
  do
  {
    digit_buf[index++] = '0' + (num % 10); // extract the lowest digit
    num /= 10;                             // remove the lowest digit

  } while (num > 0);
  // write the number on the LCD
  while (index--)
  {
    xQueueSend(*queue, &digit_buf[index], 1000);
  }
}

void select_product(MACHINE_STATES_t *state_p, COFFEE_t *selected_product_p)
{
  char key;
  char msg = 16; // command to go to start of second line

  lcd_queueStringClear("SELECT PRODUCT:");

  int current_option = 0;

  while (*state_p == SEL_PRODUCT)
  {
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
  lcd_queueStringClear("1: CARD");
  char msg = 16; // command to go to start of second line
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

void enter_card(MACHINE_STATES_t *state_p, char *card_number)
{
  // Clear display
  char msg = CLEAR_LCD;
  char key;
  uint8_t sw2_msg;

  lcd_queueStringClear("ENTER NUM & PIN");
  vTaskDelay(pdMS_TO_TICKS(1000));
  xQueueSend(lcd_queue, &msg, 1000);
  msg = 16;
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString("PIN: ");
  msg = 0;
  xQueueSend(lcd_queue, &msg, 1000);
  int i;
  for (i = 0; i < 16; i++) card_number[i] = '\0';
  char pin_code[4];
  int input_index = 0;

  uint8_t offset = 0;

  // Show card numbers as they are typed
  // check key
  while (*state_p == ENTER_CARD)
  {
    if (xQueueReceive(keypad_queue, &key, 50) == pdPASS) // check for a key press
    {
      if ((key == '*' || key == '#') && input_index)
      { // delete character
          input_index--;

          // if we are entering pin code, we must account for offset from the PIN: string
          offset = (input_index > 15) ? 5 : 0;

          msg = input_index + offset;
          xQueueSend(lcd_queue, &msg, 1000); // update cursor to 1 before current position
          msg = ' ';
          xQueueSend(lcd_queue, &msg, 1000); // clear current character
          msg = input_index + offset;
          xQueueSend(lcd_queue, &msg, 1000); // update cursor to 1 before current position
      }
      else if (input_index < 20)
      { // normal character, and we are not full

        if (input_index < 16)
          card_number[input_index] = key;
        else
          pin_code[input_index - 16] = key;

        xQueueSend(lcd_queue, &key, 1000);
        if (input_index++ == 15)
        {
          msg = 21;
          xQueueSend(lcd_queue, &msg, 1000);
        }
      }
    }

    if (xQueueReceive(SW_2_queue, &sw2_msg, 0) == pdPASS && sw2_msg == STATE_PRESSED)
    { // check for start button press
      if (input_index != 20)
      {
        lcd_queueStringClear("NOT ENOUGH INFO");
        vTaskDelay(pdMS_TO_TICKS(1000));
        *state_p = SEL_PAYMENT;
      }
      else if ((card_number[15] % 2) == (pin_code[3] % 2))
        *state_p = WAIT_CUP;
      else
      {
        lcd_queueStringClear("INVALID CARD");
        vTaskDelay(pdMS_TO_TICKS(1000));
        *state_p = SEL_PAYMENT;
      }
    }
  }
}

void enter_cash(MACHINE_STATES_t *state_p, COFFEE_t *select_product, uint16_t *paid_cash)
{
  const char lcd_line2 = 16;
  uint16_t cash = 0;
  int8_t enc_dir;
  int8_t sw2_msg;

  uint16_t price = get_coffee_price(*select_product);
  // Resume encoder thingy
  xQueueReset(encoder_queue); // if has turned the encoder before the task was resumed, old messages might lie in wait
  vTaskResume(encoder_task);  // resume the encoder task

  // initial display messages
  lcd_queueStringClear("DEPOSIT CASH");
  xQueueSend(lcd_queue, &lcd_line2, 1000);
  write_num(cash,&lcd_queue);
  lcd_queueString(" kr");

  // Show cash as the encoder is turned
  while (*state_p == ENTER_CASH)
  {
    if (xQueueReceive(encoder_queue, &enc_dir, 100) == pdPASS)
    {
      cash += (enc_dir > 0) ? 20 : 5;
      xQueueSend(lcd_queue, &lcd_line2, 1000);
      write_num(cash,&lcd_queue);
      lcd_queueString(" kr");
    }

    if (xQueueReceive(SW_2_queue, &sw2_msg, 0) == pdPASS)
    {
      if (sw2_msg == STATE_PRESSED)
      {
        if (cash < price)
        {
          lcd_queueStringClear("NOT ENOUGH CASH");
          vTaskDelay(pdMS_TO_TICKS(1000));
          lcd_queueStringClear("DEPOSIT CASH");
          xQueueSend(lcd_queue, &lcd_line2, 1000);
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
  }
}

void return_change(MACHINE_STATES_t *state_p, uint16_t change)
{
  char msg = CLEAR_LCD;
  const uint8_t led_on = ON_STATE;
  const uint8_t led_off = OFF_STATE;

  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString("RETURNING CHANGE");

  while (*state_p == RETURN_CHANGE)
  {
    if (!change--)
      return (void)(*state_p = WAIT_CUP);
    xQueueOverwrite(green_led_queue, &led_on);
    vTaskDelay(pdMS_TO_TICKS(250));
    xQueueOverwrite(green_led_queue, &led_off);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

void wait_for_cup(MACHINE_STATES_t *state_p, COFFEE_t *selected_product)
{
  char msg = CLEAR_LCD;
  uint8_t sw1_msg;
  xQueueSend(lcd_queue, &msg, 1000);
  lcd_queueString("PLACE CUP");

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
  const char lcd_line2 = 16;
  const char loading_char = 0xFF;
  const uint8_t led_on = ON_STATE;
  const uint8_t led_off = OFF_STATE;

  lcd_queueStringClear("GRINDING BEANS");
  xQueueSend(lcd_queue, &lcd_line2, 1000);
  xQueueOverwrite(yellow_led_queue, &led_on);

  uint8_t work_count = 0;

  TickType_t lastWakeTime = xTaskGetTickCount();
  while (*state_p == BREW_ESPRESSO)
  {
    uint16_t time_offset = (work_count < 16) ? GRIND_INTERVAL : BREW_INTERVAL;
    if (xQueueReceive(SW_1_queue, &sw1_msg, 0) == pdPASS && sw1_msg == STATE_RELEASED)
    {
        lcd_queueStringClear("BREWING STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        xQueueOverwrite(red_led_queue, &led_off);
        vTaskDelay(pdMS_TO_TICKS(1000));
        *state_p = CUP_ABORTED;
    }

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(time_offset));

    xQueueSend(lcd_queue, &loading_char, 1000);
    work_count++;

    if (work_count == 16)
    {
      lcd_queueStringClear("BREWING COFFEE");
      xQueueSend(lcd_queue, &lcd_line2, 1000);
      xQueueOverwrite(red_led_queue, &led_on);
      xQueueOverwrite(yellow_led_queue, &led_off);
    }
    else if (work_count == 32)
    {
      xQueueOverwrite(red_led_queue, &led_off);
      *state_p = (*selected_product == LATTE) ? FROTH_MILK : REMOVE_CUP;
    }
  }
}

void froth_milk(MACHINE_STATES_t *state_p)
{
  uint8_t sw1_msg;
  const char lcd_line2 = 16;
  const char loading_char = 0xFF;
  const uint8_t led_on = ON_STATE;
  const uint8_t led_off = OFF_STATE;

  lcd_queueStringClear("STEAMING MILK");
  xQueueSend(lcd_queue, &lcd_line2, 1000);
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
        vTaskDelay(pdMS_TO_TICKS(1000));
        *state_p = CUP_ABORTED;
    }

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(FROTH_INTERVAL));

    xQueueSend(lcd_queue, &loading_char, 1000);
    work_count++;

    if (work_count == 16)
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
  uint8_t state = WAIT_FOR_RELEASE;
  const char lcd_line2 = 16;
  const char lcd_line1 = 0;
  TickType_t timeout_stamp = 0;
  TickType_t worktime_stamp = xTaskGetTickCount();
  const uint8_t led_on = ON_STATE;
  const uint8_t led_off = OFF_STATE;

  uint16_t max_amount = 0xFFFF;
  if(paid_cash)
    max_amount = 10*( paid_cash / get_coffee_price(FILTER) );

  *amount = 0;

  lcd_queueStringClear("DISPENSE COFFEE");
  xQueueSend(lcd_queue, &lcd_line2, 1000);
  write_num(*amount,&lcd_queue);
  lcd_queueString(" ml");
  xQueueOverwrite(yellow_led_queue, &led_on);

  while (*state_p == BREW_FILTER)
  {
    if (xQueueReceive(SW_1_queue, &sw1_msg, 0) == pdPASS && sw1_msg == STATE_RELEASED)
    {
        lcd_queueStringClear("BREWING STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        xQueueOverwrite(red_led_queue, &led_off);
        vTaskDelay(pdMS_TO_TICKS(1000));
        *state_p = (state == WAIT_FOR_PRESS) ? SEL_PRODUCT : CUP_ABORTED; 
        return;
    }

    switch (state)
    {
    case WAIT_FOR_RELEASE:
      if (xQueueReceive(SW_2_queue, &sw2_msg, 100) == pdPASS && sw2_msg == STATE_RELEASED)
      {
        xQueueSend(lcd_queue, &lcd_line1, 1000); 
        lcd_queueString("DISPENSE STOPPED");
        xQueueOverwrite(yellow_led_queue, &led_off);
        timeout_stamp = xTaskGetTickCount();
        state = 1; 
      }
      if(xTaskGetTickCount() - worktime_stamp >= pdMS_TO_TICKS(1000)){
        worktime_stamp = xTaskGetTickCount();
        *amount += (*amount < 3*START_SPEED) ? START_SPEED : LATER_SPEED;
        if(*amount > max_amount){
          *amount = max_amount;
          xQueueOverwrite(yellow_led_queue, &led_off);
          lcd_queueStringClear("CASH EXHAUSTED");
          xQueueSend(lcd_queue, &lcd_line2, 1000);
          write_num(*amount,&lcd_queue);
          lcd_queueString(" ml / ");
          write_num( (*amount / 10) * get_coffee_price(FILTER), &lcd_queue );
          lcd_queueString(" kr");
          vTaskDelay(pdMS_TO_TICKS(2000));
          *state_p = REMOVE_CUP;
          return;
        }else{
          xQueueSend(lcd_queue, &lcd_line2, 1000);
          write_num(*amount, &lcd_queue);
          lcd_queueString(" ml");
        }

      }

      break;
    case WAIT_FOR_PRESS:
      if (xTaskGetTickCount() - timeout_stamp >= pdMS_TO_TICKS(5000)){
          lcd_queueStringClear("PRODUCT FINISHED");
          xQueueOverwrite(yellow_led_queue, &led_off);
          xQueueSend(lcd_queue, &lcd_line2, 1000);
          write_num(*amount,&lcd_queue);
          lcd_queueString(" ml / ");
          write_num( (*amount / 10) * get_coffee_price(FILTER),&lcd_queue );
          lcd_queueString(" kr");
          vTaskDelay(pdMS_TO_TICKS(2000));
          *state_p = REMOVE_CUP;
          return;
      } 
      if (xQueueReceive(SW_2_queue, &sw2_msg, 100) == pdPASS && sw2_msg == STATE_PRESSED)
      {
        xQueueSend(lcd_queue, &lcd_line1, 1000);
        lcd_queueString("DISPENSE COFFEE ");
        xQueueOverwrite(yellow_led_queue, &led_on);
        worktime_stamp = xTaskGetTickCount();
        state = 0;
      }
      
      break;
    }
  }
}

void remove_cup(MACHINE_STATES_t* state_p, COFFEE_t* selected_product, char *card_number, uint16_t *paid_cash, uint16_t *amount)
{
  uint8_t sw1_msg;

  lcd_queueStringClear("REMOVE THE DAMN CUP MOTHERFUCKER");
  timestamp_t time = get_timestamp();

  uart0_queueString("PRODUCT: ");
  switch(*selected_product){
    case ESPRESSO:
      uart0_queueString("ESPRESSO\n");
      break;
    case LATTE:
      uart0_queueString("LATTE\n");
      break;
    case FILTER:
      uart0_queueString("FILTER\n");
      *amount /= 10;
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

  uart0_queueString("PAYMENT TYPE: ");
  if(*paid_cash)
    uart0_queueString("CASH");
  else
    uart0_queueString(card_number);
  uart0_queueString("\n");

  uart0_queueString("PAYED AMOUNT: ");
  uint16_t coffee_price = amount * get_coffee_price(*selected_product);
  write_num(coffee_price, &uart_tx_queue);
  uart0_queueString("\n");


  while (*state_p == REMOVE_CUP)
    if (xQueueReceive(SW_1_queue, &sw1_msg, 100) == pdPASS && sw1_msg == STATE_RELEASED) *state_p = SEL_PRODUCT;
 }

 void cup_abort(MACHINE_STATES_t* state_p)
 {
    lcd_queueStringClear("FUCK YOU BITCH  IMA KEEP YO CASH");
    while (*state_p == CUP_ABORTED)
    {
      vTaskDelay(pdMS_TO_TICKS(2500));
      *state_p = SEL_PRODUCT; 
    }
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
      remove_cup(&state, &selected_product, card_number, &paid_cash, &amount);
      break;
    case CUP_ABORTED:
      cup_abort(&state);
      break;
    }
  }
}
