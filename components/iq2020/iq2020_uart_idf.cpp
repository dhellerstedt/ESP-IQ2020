#include "iq2020_uart_idf.h"

extern "C" {
  #include "driver/uart.h"
  #include "driver/gpio.h"
  #include "esp_log.h"
  #include "esp_timer.h"
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
}

static const char *TAG_IQ_UART = "iq2020_uart_idf";
static const uart_port_t IQ_UART_NUM = UART_NUM_1;

void IQ2020UartIdf::setup() {
  uart_config_t cfg = {
    .baud_rate = baud_,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  #if ESP_IDF_VERSION_MAJOR >= 5
    .source_clk = UART_SCLK_DEFAULT
  #endif
  };

  ESP_ERROR_CHECK(uart_param_config(IQ_UART_NUM, &cfg));
  ESP_ERROR_CHECK(uart_set_pin(IQ_UART_NUM, tx_, rx_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(IQ_UART_NUM, 2048, 0, 0, nullptr, 0));

  ESP_ERROR_CHECK(gpio_set_direction(de_, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(de_, 0));

  ESP_LOGI(TAG_IQ_UART, "UART initialized: TX=%d RX=%d DE=%d @ %d baud", tx_, rx_, de_, baud_);
}

void IQ2020UartIdf::dump_config() {
  ESP_LOGI(TAG_IQ_UART, "IQ2020 UART (ESP-IDF): UART%d, TX=%d RX=%d DE=%d, %d baud",
           (int)IQ_UART_NUM, (int)tx_, (int)rx_, (int)de_, baud_);
}

size_t IQ2020UartIdf::write_bytes(const uint8_t *data, size_t len, TickType_t tx_wait_ticks) {
  gpio_set_level(de_, 1);  // Enable transmit
  int written = uart_write_bytes(IQ_UART_NUM, reinterpret_cast<const char *>(data), len);
  uart_wait_tx_done(IQ_UART_NUM, tx_wait_ticks);
  gpio_set_level(de_, 0);  // Back to receive
  return written > 0 ? (size_t)written : 0;
}

int IQ2020UartIdf::read_bytes(uint8_t *buf, size_t max_len, TickType_t timeout_ticks) {
  return uart_read_bytes(IQ_UART_NUM, buf, max_len, timeout_ticks);
}
