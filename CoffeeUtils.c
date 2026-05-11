#include "CoffeeUtils.h"
#include "LCD_frt.h"

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

struct operating_data {
    uint16_t nof_espresso;
    uint16_t nof_latte;
    uint16_t nof_filter;
    uint16_t cash_amount;
    uint16_t card_amount
};

struct operating_data op_data = {0,0,0,0,0};

void increment_operating_data(OPDATA_t target_data, uint16_t amount)
{
  xSemaphoreTake(data_wr_mutex, portMAX_DELAY); // take the mutex, to ensure other tasks dont write to it
  switch (target_data)
  {
  case NOF_ESPRESSO:
    op_data.nof_espresso += amount;
    break;
  case NOF_LATTE:
    op_data.nof_latte += amount;
    break;
  case NOF_FILTER:
    op_data.nof_filter += amount;
    break;
  case CASH_AMOUNT:
    op_data.cash_amount += amount;
    break;
  case CARD_AMOUNT:
    op_data.card_amount += amount;
    break;
  }
  xSemaphoreGive(data_wr_mutex); // release the mutex
}

uint16_t get_operating_data(OPDATA_t target_data)
{
  xSemaphoreTake(data_wr_mutex, portMAX_DELAY); // take the mutex, to ensure other tasks dont write to it
  uint16_t data;
  switch(target_data){
    case NOF_ESPRESSO:
      data = op_data.nof_espresso;
      break;
    case NOF_LATTE:
      data = op_data.nof_latte;
      break;
    case NOF_FILTER:
      data = op_data.nof_filter;
      break;
    case CASH_AMOUNT:
      data = op_data.cash_amount;
      break;
    case CARD_AMOUNT:
      data = op_data.card_amount;
      break;
  }
  xSemaphoreGive(data_wr_mutex); // release the mutex
  return data;
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
