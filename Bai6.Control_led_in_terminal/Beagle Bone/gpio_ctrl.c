#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/err.h>

/*-----------------------------------HW CODE----------------------------------------------------*/
//HW access

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

//GPIO Function Prototype
static unsigned int __iomem *gpio_addr;
static unsigned int Select_GPIO_port = GPIO1_ADDR_BASE;

static void gpio_mode(int gpio, int mode);
static void gpio_write(int gpio, int level);
static int  ioremap_for_gpio(void);
static void iounmap_for_gpio(void);

static void GPIO_Set_Mode(unsigned int *gpio_base, unsigned int pin, bool mode);
static void GPIO_Set_High(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Set_Low(unsigned int *gpio_base, unsigned int pin);
static void GPIO_Toggle_Pin(unsigned int *gpio_base, unsigned int pin);
static int  ioremap_for_gpio(void);
static void  iounmap_for_gpio(void);
/*----------------------------------------END HW CODE------------------------------------------*/
#define  DEVICE_NAME "gpio_ctrl"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "gpio_ctrl_class"        ///< The device class -- this is a character device driver
#define  DEV_NUMBER_NAME  "gpio_number"        ///< The device number  name
#define  FIRST_MINOR  0
#define  DEVICE_COUNT  1

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Tran Van Sinh");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux program that ctrl gpio by writing");  ///< The description -- see modinfo
MODULE_VERSION("0.1"); 

//Device driver structure
struct _gpio_drv {
	dev_t dev_num; //device number gom 2 cap so major va minor
	struct class *dev_class;
	struct device *dev;
  	struct cdev *gpio_cdev;
}gpio_drv;

//Variable for entry point function
int open_num = 0;
static char message[256] = {0};           ///< Memory for the string that is passed from userspace

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);



// Assign these handles into the file operation
static struct file_operations fops =
{
  .owner = THIS_MODULE,
  .open = dev_open,
  .release = dev_release,
  .write = dev_write,
  .read = dev_read,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init vchar_drv_gpio_ctrl_init(void)
{
  	printk(KERN_INFO "Init device driver for gpio ctrl\n");
  	int ret = -1;

  	//Allocate device number
  	ret = alloc_chrdev_region(&gpio_drv.dev_num, FIRST_MINOR, DEVICE_COUNT, DEV_NUMBER_NAME);
  	if(ret < 0){
	    printk(KERN_INFO "GPIO: fail to allocate dev num\n");
	    return ret;
  	}
  	printk(KERN_INFO "GPIO: Allocate device number successfully at major: %d\n", MAJOR(gpio_drv.dev_num));
  	//Register class device
	gpio_drv.dev_class = class_create(THIS_MODULE, CLASS_NAME);
	if(gpio_drv.dev_class == NULL){
		printk(KERN_INFO "GPIO: Fail to create new class\n");
		//Unregister device number
		unregister_chrdev_region(gpio_drv.dev_num, DEVICE_COUNT);
		return -1;
	}
	printk(KERN_INFO "GPIO: Register device calss successfully\n");  
	//Register device driver
	gpio_drv.dev = device_create(gpio_drv.dev_class, NULL, gpio_drv.dev_num, NULL, DEVICE_NAME);
	if(gpio_drv.dev == NULL){
		printk(KERN_INFO "GPIO: Fail to create new device\n");
		//Unregister class
		class_destroy(gpio_drv.dev_class);
		//Unregister device number
		unregister_chrdev_region(gpio_drv.dev_num, DEVICE_COUNT);
		return -1;
	}
	printk(KERN_INFO "GPIO: Register device %s successfully\n", DEVICE_NAME);
	//Register entry points in kernel
	gpio_drv.gpio_cdev = cdev_alloc();
	if(gpio_drv.gpio_cdev ==NULL){
		printk(KERN_INFO "GPIO: Fail to allocate cdev structure\n");
		//unregister device
		device_destroy(gpio_drv.dev_class, gpio_drv.dev_num);
		//Unregister class
		class_destroy(gpio_drv.dev_class);
		//Unregister device number
		unregister_chrdev_region(gpio_drv.dev_num, DEVICE_COUNT);
		return -1;
	}else{
		printk(KERN_INFO "GPIO: Alloc cdev structure successfully\n");
	}
	//Init cdev structure
	cdev_init(gpio_drv.gpio_cdev, &fops);
	//Register cdev in kernel
	ret = cdev_add(gpio_drv.gpio_cdev, gpio_drv.dev_num, 1);
	if(ret<0){
		printk(KERN_INFO "GPIO: Fail to register char device to system\n");
		//unregister device
		device_destroy(gpio_drv.dev_class, gpio_drv.dev_num);
		//Unregister class
		class_destroy(gpio_drv.dev_class);
		//Unregister device number
		unregister_chrdev_region(gpio_drv.dev_num, DEVICE_COUNT);
		return -1;
	}else{
		printk(KERN_INFO "GPIO: Add cdev structure to the system successfully\n");
	}
	//Mapping virtual memory
	ret = ioremap_for_gpio();
	if(ret!=0){
		 printk(KERN_INFO "GPIO: Fail to init GPIO");
		 return -1;
	}
	printk(KERN_INFO "GPIO: Initialize vchar driver successfully\n");

	return 0;				
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit vchar_drv_gpio_ctrl_exit(void)
{
    iounmap_for_gpio();
    cdev_del(gpio_drv.gpio_cdev);
    device_destroy(gpio_drv.dev_class, gpio_drv.dev_num);
    class_destroy(gpio_drv.dev_class);
    unregister_chrdev_region(gpio_drv.dev_num, DEVICE_COUNT);
    printk(KERN_INFO "GPIO: Exit vchar driver\n");
}
	
/** @brief The device open function that is calgpio each time the device is opened
 *  This will only increment the open_num counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   	open_num++;
   	printk(KERN_INFO "GPIO: Device has been opened %d time(s)\n", open_num);
   	return 0;
}

/** @brief The device release function that is calgpio whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "GPIO: Device successfully closed\n");
   return 0;
}

/** @brief This function is calgpio whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, len);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "GPIO: Sent %d characters to the user\n", len);
      return 0;
   }
   else {
      printk(KERN_INFO "GPIO: Fail to send %d characters to the user\n", error_count);
      return -EFAULT;              // Faigpio -- return a bad address message (i.e. -14)
   }
}

/** @brief This function is calgpio whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   copy_from_user(message, buffer, len);
   printk(KERN_INFO "GPIO: Received %zu characters from the user\n", len);
   printk(KERN_INFO "GPIO: message[0] = %c\n", message[0]);

   if(message[0] == '1'){
		printk(KERN_INFO "GPIO: Turn on gpio\n");
		GPIO_Set_Mode(gpio_addr, GPIO_PIN, OUTPUT);
		GPIO_Set_High(gpio_addr, GPIO_PIN);
   }else if (message[0] == '0'){
     	printk(KERN_INFO "GPIO: Turn off gpio\n");
		GPIO_Set_Mode(gpio_addr, GPIO_PIN, OUTPUT);
		GPIO_Set_Low(gpio_addr, GPIO_PIN);
   }else{
     	printk(KERN_INFO "GPIO: The writen argument is invalid\n");
   }
   return len;
}

/*--------------------------------Functin for GPIO------------------------------------------*/
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

//remap register
static int  ioremap_for_gpio(void)
{
    gpio_addr = ioremap(Select_GPIO_port, GPIO_SIZE);
  if(gpio_addr == NULL) {
    printk(KERN_INFO "GPIO: Mapping gpio_addr fail\n");
      return -ENOMEM;
  }
  return 0;
}
//unmap register
static void  iounmap_for_gpio(void)
{
    iounmap(gpio_addr);
}

module_init(vchar_drv_gpio_ctrl_init);
module_exit(vchar_drv_gpio_ctrl_exit);
