#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define GPIO_CONFIG 11
#define GPIO_DECONFIG 10
#define GPIO_CONTROL 12
#define GPIO_ON_DELAY 13

#define GPIO_IN 0
#define GPIO_OUT 1

// #define PIN 17

struct gpio_ssjhs
{
    int pin;
    long value;
    // long s;
    // long ms;
    // long us;
};

struct gpio_ssjhs gpio;
static int fd;

void led_init(int pin)
{
    gpio.pin = pin;
    gpio.value = 0;

    system("sudo insmod /home/pi/app/kernel/gpio_control.ko");
    system("sudo chown pi:pi /dev/led_control_demo");
    fd = open("/dev/led_control_demo", O_RDWR);
    if (fd < 0)
    {
        perror("Open file failed!!!\r\n");
        exit(-5);
    }

    int ret = ioctl(fd, GPIO_CONFIG, gpio.pin);
    if (ret)
    {
        perror("ioctl write");
        exit(-5);
    }
}

void led_flash(int udelay)
{
    gpio.value = udelay;
    ioctl(fd, GPIO_ON_DELAY, &gpio);
}

void led_release()
{
    ioctl(fd, GPIO_DECONFIG, &gpio);
    close(fd);
    system("sudo rmmod gpio_control");
}

int main2(int argc, char **argv)
{
    long interval = atoi(argv[1]);
    led_init(17);
    led_flash(interval);
    led_release();
    return 0;
}
