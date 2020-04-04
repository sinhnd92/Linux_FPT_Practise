#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/err.h>

#define AUTHOR "TRAN VAN SINH"

#define	IOMUX_BASE_ADDR	0x20E0000
#define	IOMUX_DT06_OFFSET	0x64
#define	IOMUX_DT07_OFFSET	0x68
#define IOMUX_21_OFFSET 	0x68
#define IOMUX_20_OFFSET 	0x64

#define GPIO_ADDR_BASE			0x209C000
#define GPIO_DR_OFFSET			0x00U
#define GPIO_GDIR_OFFSET		0x04
#define GPIO_PSR_OFFSET			0x08
#define GPIO_ICR1_OFFSET		0x0C
#define GPIO_ICR2_OFFSET		0x10
#define GPIO_IMR_OFFSET			0x14
#define GPIO_ISR_OFFSET			0x18
#define GPIO_EDGE_SEL_OFFSET		0x1C

#define GPIO_SIZE			0x1801C
#define IOMUX_SIZE		0x87C

#define	HIGH					1
#define	LOW					  0
#define	OUTPUT				1
#define	INPUT					0
#define	MASK					1
#define	UNMASK				0

#define	LOW_LEVEL		  0b00
#define	HIGH_LEVEL		0b01
#define	RISING_EDGE		0b10
#define	FALLING_EDGE	0b11


#define	SET_BIT(x)		(1<<x)
#define	CLEAR_BIT(x)	(~(1<<x))

#define ATL1					1
#define ATL2					2
#define ATL3					3
#define ATL4					4
#define ATL5					5
#define ATL6					6

#define	GPIO_21					21
#define	GPIO_20					20

unsigned int __iomem *GPIO_CTRL;
unsigned int __iomem *IOMUX_CTRL;
unsigned char irq_number;

irqreturn_t external_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
static void Set_IOMUX_Mode_GPIO(int iomux_offset);
static void Set_GPIO_Mode(int gpio, int mode);
static void Set_GPIO_level(int gpio, int level);
static int Get_GPIO_level(int gpio);
static void GPIO_Toggle(int gpio);
static void Set_Interrupt_GPIO(int gpio);
static void Set_Interrupt_Detect_mode(int gpio, int mode);


static int __init led_init(void)
{
    int ret;
    printk("Start driver\n");
    /*Remap GPIO Registers*/
    GPIO_CTRL = ioremap(GPIO_ADDR_BASE, GPIO_SIZE);
    if(GPIO_CTRL == NULL) {
            printk("Mapping GPIO_CTRL fail\n");
    }
 	  /*Remap IOMUX Register*/
    IOMUX_CTRL = ioremap(IOMUX_BASE_ADDR, IOMUX_SIZE);
    if(IOMUX_CTRL == NULL) {
            printk("Mapping IOMUX_CTRL fail!\n");
            goto IOMUX_CTRL_fail;
    }
    /*Set up GPIO_20 pin and GPIO_21 pin as GPIO in IOMUX Register*/
    Set_IOMUX_Mode_GPIO(IOMUX_DT07_OFFSET);
		Set_IOMUX_Mode_GPIO(IOMUX_DT06_OFFSET);
    /*Set up GPIO_21 as INPUT pin*/
    Set_GPIO_Mode(GPIO_21, INPUT);
    /*Set up GPIO_20 as OUTPUT pin*/
		Set_GPIO_Mode(GPIO_20, OUTPUT);
    /*Set up GPIO_20 as interrupt falling edge detection*/
    Set_Interrupt_Detect_mode(GPIO_21, FALLING_EDGE);
    /*Enable interrupt line in GPIO_21*/
    Set_Interrupt_GPIO(GPIO_21);
    /*Init interrupt*/
    irq_number = gpio_to_irq(GPIO_21);
	  ret = request_irq(irq_number, (irq_handler_t) external_interrupt_handler, IRQF_SHARED, "INT_GPIO21", (void *)external_interrupt_handler);
	  if(ret)
		  printk("Init IRQ handler success\n");	
    return 0;

IOMUX_CTRL_fail:
    iounmap(GPIO_CTRL);
    return -ENOMEM;
}

static void __exit led_exit(void)
{
  printk("Exit driver\n");
  free_irq(irq_number, (void *)external_interrupt_handler);
  iounmap(GPIO_CTRL);
  iounmap(IOMUX_CTRL);
};
irqreturn_t external_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
  printk("Run interrupt handler\n");
  GPIO_Toggle(GPIO_20);
	return IRQ_HANDLED;
}

static void Set_IOMUX_Mode_GPIO(int iomux_offset)
{
  iomux_offset /= 4;
  IOMUX_CTRL[iomux_offset] |= ATL5;
}

static void Set_GPIO_Mode(int gpio, int mode)
{
  if(mode == INPUT) {
          printk("gpio_%d INPUT\n", gpio);
          GPIO_CTRL[GPIO_GDIR_OFFSET / 4] &= CLEAR_BIT(gpio);
  } else if(mode == OUTPUT) {
          printk("gpio_%d OUTPUT\n", gpio);
          GPIO_CTRL[GPIO_GDIR_OFFSET / 4] |= SET_BIT(gpio);
  }
}

static void Set_GPIO_level(int gpio, int level)
{
  if(level == HIGH) {
          printk("gpio_%d HIGH\n", gpio);
          GPIO_CTRL[GPIO_DR_OFFSET / 4] |= SET_BIT(gpio);
  } else if(level == LOW) {
          printk("gpio_%d LOW\n", gpio);
          GPIO_CTRL[GPIO_DR_OFFSET / 4] &= CLEAR_BIT(gpio);
  }
}
static int Get_GPIO_level(int gpio)
{
  return (GPIO_CTRL[GPIO_DR_OFFSET/4] & SET_BIT(gpio));
}

static void GPIO_Toggle(int gpio)
{
  if (Get_GPIO_level(gpio))
	{
      		Set_GPIO_level(gpio, LOW);

	}else{
      		Set_GPIO_level(gpio, HIGH);

	}
}

static void Set_Interrupt_GPIO(int gpio)
{
  GPIO_CTRL[GPIO_IMR_OFFSET/4]  |= SET_BIT(gpio);
}

static void Set_Interrupt_Detect_mode(int gpio, int mode)
{
  if(gpio<=15)
    GPIO_CTRL[GPIO_ICR1_OFFSET/4] |= mode<<(gpio*2);
  else if((gpio>15)&&(gpio<=31))
    GPIO_CTRL[GPIO_ICR2_OFFSET/4] |= mode<<((gpio-16)*2);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);

