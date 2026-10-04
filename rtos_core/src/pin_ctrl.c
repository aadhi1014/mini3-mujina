/*
 * GPIO control for Mini3's single ASIC chain-control line (reset /
 * config-mode gate), via the RT-Smart /dev/gpio character device.
 *
 * Ported from Nano3s's pin_ctrl.c. Nano3s has a 4-relay control board
 * (RO0-RO3, GPIO2-5) used as a config-mode gate separate from its own
 * GPIO31 RST; Mini3 has no relay board -- RE of Mini3's stock firmware
 * found exactly one GPIO (31) covering this whole role (see pin_ctrl.h),
 * so the relay_pins[] array and gpio_set_value() indirection are dropped
 * and ENTER_CONFIG_MODE/ENTER_WORK_MODE now call gpio_asic_rst_assert()/
 * deassert() directly.
 */

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "pin_ctrl.h"

/* ioctl command numbers: 'G'-magic ioctls on /dev/gpio, each taking a
 * {pin, value} pair. */
#define KD_IOCTL_SET_OUTPUT  _IOW('G', 0, int)
#define KD_IOCTL_SET_INPUT   _IOW('G', 1, int)
#define KD_IOCTL_WRITE_LOW   _IOW('G', 4, int)
#define KD_IOCTL_WRITE_HIGH  _IOW('G', 5, int)
#define KD_IOCTL_READ_VALUE  _IOW('G', 12, int)

typedef struct {
        unsigned short pin;
        unsigned short value;
} gpio_ioctl_arg_t;

static int gpio_dev_fd = -1;

void gpio_init(uint8_t cnt)
{
        (void)cnt; /* Mini3 has no relay board -- nothing to size this by */

        gpio_dev_fd = open("/dev/gpio", O_RDWR);
}

int gpio_asic_rst_init(void)
{
        gpio_ioctl_arg_t arg;

        if (gpio_dev_fd < 0)
                return -1;

        arg.pin = ASIC_RST_PIN;
        return ioctl(gpio_dev_fd, KD_IOCTL_SET_OUTPUT, &arg);
}

void gpio_asic_rst_assert(void)
{
        gpio_ioctl_arg_t arg;

        arg.pin = ASIC_RST_PIN;
        ioctl(gpio_dev_fd, KD_IOCTL_WRITE_HIGH, &arg);
}

void gpio_asic_rst_deassert(void)
{
        gpio_ioctl_arg_t arg;

        arg.pin = ASIC_RST_PIN;
        ioctl(gpio_dev_fd, KD_IOCTL_WRITE_LOW, &arg);
}

int gpio_read_pin(uint16_t pin)
{
        gpio_ioctl_arg_t arg;
        int rc;

        if (gpio_dev_fd < 0)
                return -1;

        arg.pin = pin;
        arg.value = 0xffff; /* sentinel to detect if the driver leaves it untouched */
        rc = ioctl(gpio_dev_fd, KD_IOCTL_READ_VALUE, &arg);
        if (rc < 0)
                return -1;
        return arg.value;
}
