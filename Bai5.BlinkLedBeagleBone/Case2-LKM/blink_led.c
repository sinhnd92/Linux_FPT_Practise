#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/delay.h>

#define GPIO0_ADDR_BASE		0x44E07000
#define GPIO1_ADDR_BASE		0x4804C000
#define GPIO2_ADDR_BASE		0x481AC000
#define GPIO3_ADDR_BASE		0x481AE000
#define GPIO_MASK			0xFFFFFFFF

#define OE_ADDR 			0x134
#define GPIO_DATAOUT 		0x13C
#define GPIO_DATAIN 		0x138

#define GPIO_SIZE			0x198

#define OUTPUT				0
#define INPUT 				1
#define HIGH				1
#define LOW					0

#define GPIO_SET_BIT(x)		(1 << x)
#define GPIO_CLEAR_BIT(x)	~(1 << x)

#define GPIO_PIN			17

unsigned int __iomem *gpio_addr;

static void GPIO_Set_Mode(unsigned int *gpio_base, unsigned int pin, bool mode)
{
	if (mode == INPUT)
	{
	 	gpio_base[OE_ADDR/4] |= GPIO_SET_BIT(pin);
	}else{
		gpio_base[OE_ADDR/4] &= GPIO_CLEAR_BIT(pin);
	}
}

static void GPIO_Set_High(unsigned int *gpio_base, unsigned int pin)
{
	gpio_base[GPIO_DATAOUT/4] |= GPIO_SET_BIT(pin);
}


static void GPIO_Set_Low(unsigned int *gpio_base, unsigned int pin)
{
	gpio_base[GPIO_DATAOUT/4] &= GPIO_CLEAR_BIT(pin);
}

static void GPIO_Toggle_Pin(unsigned int *gpio_base, unsigned int pin)
{
	gpio_base[GPIO_DATAOUT/4] ^= GPIO_SET_BIT(pin);
}

static int __init blink_led_init(void)
{
  	int i;
  	printk("Start led blink program\n");
	/*Remap the virtual memory*/
	gpio_addr = ioremap(GPIO1_ADDR_BASE, GPIO_SIZE);
	if(gpio_addr == NULL) {
		printk("Mapping gpio_addr fail\n");
	}
    /*Set up the GPIO1 as external pin*/
	GPIO_Set_Mode(gpio_addr, GPIO_PIN, OUTPUT);
  	GPIO_Set_High(gpio_addr, GPIO_PIN);
	for(i=0;i<20;i++)
	{
	 	GPIO_Toggle_Pin(gpio_addr, GPIO_PIN);
	 	msleep(1000);
	}
 return 0;
}

static void __exit blink_led_exit(void)
{
  printk("Exit led blink program\n");
	GPIO_Set_Low(gpio_addr, GPIO_PIN);
	iounmap(gpio_addr);
}

module_init(blink_led_init);
module_exit(blink_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tran Van Sinh");
MODULE_DESCRIPTION("A simple Linux GPIO Blink Led LKM for the BBB");
MODULE_VERSION("0.1");