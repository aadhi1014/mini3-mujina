#ifndef PIN_CTRL_H
#define PIN_CTRL_H

#include <stdint.h>

void gpio_init(uint8_t cnt);

/*
 * ASIC chain RST / config-mode gate -- GPIO31, Bank A, output,
 * ACTIVE-HIGH (held LOW during normal operation, chips running; HIGH =
 * held in reset / config mode).
 *
 * Confirmed via live RE of Mini3's stock RTOS firmware (physical RAM dump,
 * bigcore_full.bin, analyzed in Ghidra): the stock gpio_init() hardcodes
 * pin 31 as the only GPIO it ever configures as output, and a
 * gpio_set_uart(pin, level) helper in the same code region writes a level
 * then reads it back to verify, logging "the output pin is not low"/"not
 * high" on mismatch -- matching this exact assert/deassert-with-verify
 * shape. GPIO31's Linux device-tree node is status="disabled", confirming
 * it's reserved for the RT-Smart big core, not shared with Linux.
 *
 * Unlike Nano3s -- which has a separate 4-relay control board (RO0-RO3,
 * GPIO2-5) used as a config-mode gate distinct from its own GPIO31 RST --
 * Mini3 has no relay board. Its stock firmware uses this single pin for
 * both roles, so ENTER_CONFIG_MODE/ENTER_WORK_MODE below collapse onto it
 * too, rather than indexing a relay array that doesn't exist on this
 * board.
 */
#define ASIC_RST_PIN 31

int gpio_asic_rst_init(void);
void gpio_asic_rst_assert(void);   /* HIGH -- hold chips in reset / config mode */
void gpio_asic_rst_deassert(void); /* LOW -- release, chips run / work mode */
int gpio_read_pin(uint16_t pin);   /* -1 on failure, else 0/1 */

/*
 * Mini3 has no relay-driven config-mode gate separate from ASIC_RST_PIN --
 * channel/board index "i" is accepted (main.c's call sites pass the UART
 * channel, unchanged from Nano3s) but unused, since there's only ever one
 * physical pin to drive on this board.
 */
#define ENTER_CONFIG_MODE(i) do { (void)(i); gpio_asic_rst_assert(); } while (0)
#define ENTER_WORK_MODE(i)   do { (void)(i); gpio_asic_rst_deassert(); } while (0)

/*
 * power_en (GPIO29) and the chain's power-on-reset pulse (GPIO34) are
 * both handled by mm_miner on the Linux side -- confirmed via mm_miner
 * disassembly (see MINI3_GPIO_MAP.md's mcu_start() section), not by this
 * file. This mirrors Nano3s's own power_en-on-Linux-side convention.
 */

#endif
