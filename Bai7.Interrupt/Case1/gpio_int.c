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

/*-----------------------------------HW CODE----------------------------------------------------*/
//HW access
//In this LKM, due to using one button, I just use one interrupt line 0 (AM335x has 2 interrupt lines for each GPIO module)

#define GPIO0_ADDR_BASE   0x44E07000
#define GPIO1_ADDR_BASE   0x4804C000
#define GPIO2_ADDR_BASE   0x481AC000
#define GPIO3_ADDR_BASE   0x481AE000
#define GPIO_SIZE         0x1FFF
// CM_PER (Clock Module Peripheral Registers
#define CM_PER_ADDR_BASE 0x44e00000
#define CM_PER_SIZE       0x400
#define CM_PER_GPIO3_CLKCTRL 0xB4
#define CLK_EN      0x02

#define GPIO_MASK     0xFFFFFFFF

#define OE_ADDR       0x134
#define GPIO_DATAOUT    0x13C
#define GPIO_DATAIN     0x138
#define GPIO_IRQSTATUS_SET_0    0x34
#define GPIO_IRQSTATUS_SET_1    0x38
#define GPIO_FALLING_DETECT     0x14C
#define GPIO_RISING_DETECT     0x148
#define GPIO_IRQ_STATUS_0     0x2C
#define GPIO_IRQ_STATUS_1     0x30
#define GPIO_DEBOUNCE_EN     0x150
#define GPIO_DEBOUNCE_TIME     0x154
#define GPIO_CTRL         0x130

#define GPIO_SEL   115



#define OUTPUT        0
#define INPUT         1
#define HIGH        1
#define LOW         0
#define RISING        1
#define FALLING         0

#define GPIO_SET_BIT(x)   (1 << x)
#define GPIO_CLEAR_BIT(x) ~(1 << x)

#define GPIO_PIN      17  //GPIO1_17
#define BUTTON_PIN      19  //GPIO3_19
#define DEBOUNCE_TIME_MS   100

//GPIO Function Prototype
static unsigned int __iomem *gpio_led;
static unsigned int __iomem *gpio_btn;
static unsigned int __iomem *cm_per;

static unsigned int Select_GPIO_port = GPIO1_ADDR_BASE;
static unsigned int Select_button_port = GPIO3_ADDR_BASE;
static unsigned int Select_clk_en_cm_per = CM_PER_GPIO3_CLKCTRL;
static unsigned int irq_number;

static void GPIO_CLK_Enable(unsigned int gpio_select_res);
static void GPIO_Enable(unsigned int *gpio_base);
static void GPIO_Disable(unsigned int *gpio_base);
static void GPIO_Set_Mode(unsigned int *gpio_base, unsigned int pin, bool mode);
static void GPIO_Set_High(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Set_Low(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Toggle_Pin(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Setting_Debounce_Enable(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Setting_Debounce_Time(unsigned int *gpio_base, unsigned int time_in_ms);
static void GPIO_Enable_Interrupt(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Set_Interrupt_Detect_mode(unsigned int *gpio_base, unsigned int pin, bool mode);
static void GPIO_Clear_Interrupt_Flag(unsigned int *gpio_base, unsigned int pin);
static irqreturn_t external_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);



//The LKM initialization function
static int __init led_init(void)
{
    int ret;
    printk(KERN_INFO "GPIO: Start driver\n");
    /*Remap GPIO Registers*/
    gpio_led = ioremap(Select_GPIO_port, GPIO_SIZE);
    if(gpio_led == NULL) {
            printk(KERN_INFO "GPIO: Mapping GPIO_CTRL fail\n");
    }

    gpio_btn = ioremap(Select_button_port, GPIO_SIZE);
    if(gpio_btn == NULL) {
        printk(KERN_INFO "GPIO: Mapping GPIO for button fail\n");
        iounmap(gpio_led);
        return -1;
    }
    cm_per = ioremap(CM_PER_ADDR_BASE, CM_PER_SIZE);
    if(gpio_btn == NULL) {
        printk(KERN_INFO "GPIO: Mapping the clock enable for GPIO register fail\n");
        iounmap(gpio_led);
        iounmap(gpio_btn);
        return -1;
    } 
    /*Enable clock*/
    GPIO_CLK_Enable(Select_clk_en_cm_per);
    /*Disable module first*/
    // GPIO_Disable(gpio_led);
    // GPIO_Disable(gpio_btn);
    /*Set up GPIO3_19 as INPUT pin*/
    GPIO_Set_Mode(gpio_btn, BUTTON_PIN, INPUT);
    /*Set up GPIO_20 as OUTPUT pin*/
    GPIO_Set_Mode(gpio_led, GPIO_PIN, OUTPUT);
    /*Set the debounce time for interrupt pin*/
    GPIO_Setting_Debounce_Time(gpio_btn, DEBOUNCE_TIME_MS);
    /*Debouce feature enable*/
    GPIO_Setting_Debounce_Enable(gpio_btn, BUTTON_PIN);
    /*Setting interrupt detect mode*/
    GPIO_Set_Interrupt_Detect_mode(gpio_btn, BUTTON_PIN, RISING);
    /*Init interrupt*/
    irq_number = gpio_to_irq(GPIO_SEL);
    // irq_number = GPIO3_INTLINE_0;
    ret = request_irq(irq_number, (irq_handler_t)external_interrupt_handler, 0, "GPIO handler", NULL);
    if(ret){
        printk(KERN_INFO "GPIO: Init IRQ handler success\n"); 
        printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irq_number);
    }
    else
    {
        free_irq(irq_number, (void *)external_interrupt_handler);
        iounmap(gpio_led);
        iounmap(gpio_btn); 
        return -1;       
    }
    /*Enable interrupt line*/
    // GPIO_Enable_Interrupt(gpio_btn, BUTTON_PIN);
    /*Enable module*/
    // GPIO_Enable(gpio_led);
    // GPIO_Enable(gpio_btn); 
    GPIO_Set_High(gpio_led, GPIO_PIN);
    return 0;
}

//The LKM cleanup function
static void __exit led_exit(void)
{
    printk(KERN_INFO "GPIO: Exit driver\n");
    GPIO_Set_Low(gpio_led, GPIO_PIN);
    free_irq(irq_number, (void *)external_interrupt_handler);
    iounmap(gpio_led);
    iounmap(gpio_btn);
    iounmap(cm_per);
};

//IRQ handler for pin
static irqreturn_t external_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    GPIO_Clear_Interrupt_Flag(gpio_btn, BUTTON_PIN);
    printk(KERN_INFO "GPIO: Run interrupt handler\n");
    GPIO_Toggle_Pin(gpio_led, GPIO_PIN);
    return IRQ_HANDLED;
}

//Enable clock for GPIO
static void GPIO_CLK_Enable(unsigned int gpio_select_res)
{
    cm_per[gpio_select_res/4] &= (GPIO_CLEAR_BIT(0) & GPIO_CLEAR_BIT(1));
    cm_per[gpio_select_res/4] |= CLK_EN;
}

//Enable GPIO module
static void GPIO_Enable(unsigned int *gpio_base)
{
    gpio_base[GPIO_CTRL/4] &= GPIO_CLEAR_BIT(0);
}

//Disable GPIO module
static void GPIO_Disable(unsigned int *gpio_base)
{
    gpio_base[GPIO_CTRL/4] |= GPIO_SET_BIT(0);
}

//Set mode input/output for GPIO
static void GPIO_Set_Mode(unsigned int *gpio_base, unsigned int pin, bool mode)
{
    if (mode == INPUT)
    {
      gpio_base[OE_ADDR/4] |= GPIO_SET_BIT(pin);
    }else{
      gpio_base[OE_ADDR/4] &= GPIO_CLEAR_BIT(pin);
    }
}
//Set GPIO high level
static void GPIO_Set_High(unsigned int *gpio_base, unsigned int pin)
{
    gpio_base[GPIO_DATAOUT/4] |= GPIO_SET_BIT(pin);
}

//Set GPIO low level
static void GPIO_Set_Low(unsigned int *gpio_base, unsigned int pin)
{
    gpio_base[GPIO_DATAOUT/4] &= GPIO_CLEAR_BIT(pin);
}

//Set GPIO toggle
static void GPIO_Toggle_Pin(unsigned int *gpio_base, unsigned int pin)
{
    gpio_base[GPIO_DATAOUT/4] ^= GPIO_SET_BIT(pin);
}

//Enable the debouce feature that help decreasing noise
static void GPIO_Setting_Debounce_Enable(unsigned int *gpio_base, unsigned int pin)
{
    gpio_base[GPIO_DEBOUNCE_EN/4]  |= GPIO_SET_BIT(pin);
}

//Setting the debouce time that help decreasing noise
static void GPIO_Setting_Debounce_Time(unsigned int *gpio_base, unsigned int time_in_ms)
{
    //Calculate Input Debouncing Value in 31 microsecond steps. Debouncing Value = (DEBOUNCETIME + 1) * 31 microseconds
    int debouce_value = time_in_ms/31 +1;
    gpio_base[GPIO_DEBOUNCE_TIME/4]  &= ~GPIO_MASK;
    gpio_base[GPIO_DEBOUNCE_TIME/4]  |= debouce_value;
}

//Setting the interupt in the coresponding pin
static void GPIO_Enable_Interrupt(unsigned int *gpio_base, unsigned int pin)
{
    gpio_base[GPIO_IRQSTATUS_SET_0/4]  |= GPIO_SET_BIT(pin);
    // gpio_base[GPIO_IRQSTATUS_SET_1/4]  |= GPIO_SET_BIT(pin);
}

//Setting the detect mode (falling/rising or both) for the event
static void GPIO_Set_Interrupt_Detect_mode(unsigned int *gpio_base, unsigned int pin, bool mode)
{
    if(mode == RISING)
    {
      gpio_base[GPIO_RISING_DETECT/4] |= GPIO_SET_BIT(pin);
    }else
    {
      gpio_base[GPIO_FALLING_DETECT/4] |= GPIO_SET_BIT(pin);
    }
}
//Clear the interrupt flag in the coresponding gpio pin
static void GPIO_Clear_Interrupt_Flag(unsigned int *gpio_base, unsigned int pin)
{
    gpio_base[GPIO_IRQ_STATUS_0/4] |= GPIO_SET_BIT(pin);
    // gpio_base[GPIO_IRQ_STATUS_1/4] |= GPIO_SET_BIT(pin);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);

