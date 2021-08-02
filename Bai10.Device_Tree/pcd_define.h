#ifndef PCD_DEFINE_H
#define PCD_DEFINE_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_device.h>

#define MAX_DEVICES 4

/*Permission codes */

#define RDWR 0x11
#define RDONLY 0x01
#define WRONLY 0x10

/* List devices */
enum device_list
{
	DEVICE0,
	DEVICE1,
	DEVICE2,
	DEVICE3
};

struct device_config 
{
	int config_item1;
	int config_item2;
};

/*platform data of the pcdev */
struct device_platform_data
{
	int size;
	int permission;
	const char *serial_number;

};

/*Device private data structure */
struct device_private_data
{
	struct device_platform_data data;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

/*Driver private data structure */
struct driver_private_data
{
	int total_devices;
	dev_t device_num_base;
	struct class *class_dev;
	struct device *device_dev;
};

#endif