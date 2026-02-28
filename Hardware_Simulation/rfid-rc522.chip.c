#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
  uart_dev_t uart;
  timer_t timer;
  uint32_t selectedCardAttr;

  uint32_t lastSelected;
  bool uartBusy;
} chip_state_t;

// 10 UID mẫu (bạn đổi theo UID bạn muốn)
static const char *UIDS[10] = {
  "12 34 56 78",
  "50 9D 39 23",
  "77 18 40 05",
  "9F D6 B1 BD",
  "6D 7C 91 BC"
};

static void on_uart_write_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  chip->uartBusy = false;
}

static void tick(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;

  uint32_t v = attr_read(chip->selectedCardAttr);

  // Chỉ gửi khi giá trị slider thay đổi
  if (v == chip->lastSelected) return;
  chip->lastSelected = v;

  if (chip->uartBusy) return;

  // 0 = không có thẻ
  if (v == 0) {
    // có thể không gửi gì, hoặc gửi "UID:NONE\n"
    char msg[] = "UID:NONE\n";
    chip->uartBusy = uart_write(chip->uart, (uint8_t*)msg, (uint32_t)strlen(msg));
    return;
  }

  // 1..10
  if (v >= 1 && v <= 10) {
    char line[64];
    snprintf(line, sizeof(line), "UID:%s\n", UIDS[v - 1]);
    chip->uartBusy = uart_write(chip->uart, (uint8_t*)line, (uint32_t)strlen(line));
  }
}

void chip_init(void) {
  chip_state_t *chip = (chip_state_t*)calloc(1, sizeof(chip_state_t));

  // attributes (slider)
  chip->selectedCardAttr = attr_init("selectedCard", 0);
  chip->lastSelected = 0;
  chip->uartBusy = false;

  // UART pins
  const uart_config_t uart_cfg = {
    .tx = pin_init("TX", INPUT_PULLUP),
    .rx = pin_init("RX", INPUT),
    .baud_rate = 115200,
    .rx_data = NULL,
    .write_done = on_uart_write_done,
    .user_data = chip,
  };
  chip->uart = uart_init(&uart_cfg);

  // timer tick (poll slider)
  const timer_config_t tcfg = {
    .user_data = chip,
    .callback = tick,
  };
  chip->timer = timer_init(&tcfg);
  timer_start(chip->timer, 50 * 1000, true); // mỗi 50ms

  printf("RFID UART Emu chip initialized.\n");
}