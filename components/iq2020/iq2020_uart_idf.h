#pragma once

#include "esphome.h"

class IQ2020UartIdf : public esphome::Component {
 public:
  // Constructor with optional pin and baud overrides
  IQ2020UartIdf(gpio_num_t tx = GPIO_NUM_26,
                gpio_num_t rx = GPIO_NUM_32,
                gpio_num_t de = GPIO_NUM_21,
                int baud = 38400)
      : tx_(tx), rx_(rx), de_(de), baud_(baud) {}

  // Called once at boot
  void setup() override;

  // Optional: show config in logs
  void dump_config() override;

  // Send raw bytes to IQ2020 controller
  size_t write_bytes(const uint8_t *data, size_t len, TickType_t tx_wait_ticks = pdMS_TO_TICKS(100));

  // Read bytes from IQ2020 controller
  int read_bytes(uint8_t *buf, size_t max_len, TickType_t timeout_ticks);

 private:
  gpio_num_t tx_;
  gpio_num_t rx_;
  gpio_num_t de_;
  int baud_;
};
