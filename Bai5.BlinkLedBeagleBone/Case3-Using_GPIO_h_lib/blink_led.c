#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // Required for the GPIO functions
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/kthread.h>    // Using kthreads for the flashing functionality
#include <linux/delay.h>      // Using this header for the msleep() function


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tran Van Sinh");
MODULE_DESCRIPTION("A simple Linux GPIO Blink Led LKM for the BBB");
MODULE_VERSION("0.1");

static unsigned int gpioLED = 49;       ///< hard coding the LED gpio for this example to P9_23 (GPIO49), corresponding with GPIO1_17
module_param(gpioLED, uint, S_IRUGO);       ///< Param desc. S_IRUGO can be read/not changed
MODULE_PARM_DESC(gpioLED, " GPIO LED number (default=40)");         ///< parameter description

static bool     ledOn = false;          ///< Is the LED on or off? Used to invert its state (off by default)

static int __init led_blink_init(void)
{
  int i;
    printk(KERN_INFO "GPIO_TEST: Initializing the GPIO_TEST LKM\n");
    /*Is the GPIO a valid GPIO number (e.g., the BBB has 4x32 but not all available)*/
    if (!gpio_is_valid(gpioLED)){
        printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
        return -ENODEV;
    }
    /*Going to set up the LED. It is a GPIO in output mode and will be on by default*/
    ledOn = true;
    gpio_request(gpioLED, "sysfs");          // gpioLED is hardcoded to 48, request it
    gpio_direction_output(gpioLED, ledOn);   // Set the gpio to be in output mode and on
    gpio_export(gpioLED, false);             // Causes selected gpio to appear in /sys/class/gpio

    for(i=0; i<20; i++)
    {
        if(ledOn == true)
        {
            ledOn =false;
            gpio_set_value(gpioLED, ledOn);
            msleep(1000);
        }
        else
        {
            ledOn =true;
            gpio_set_value(gpioLED, ledOn);
            msleep(1000);           
        }
    }
    return 0;
}

static void __exit led_blink_exit(void){
   gpio_set_value(gpioLED, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioLED);                  // Unexport the LED GPIO
   gpio_free(gpioLED);                      // Free the LED GPIO
   printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(led_blink_init);
module_exit(led_blink_exit);

