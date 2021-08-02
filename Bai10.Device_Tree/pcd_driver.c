
#include "pcd_functions.h"


#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__
/*
=======================================================================================
								LOCAL FUNCTION
=======================================================================================
*/
/* Driver entry functions */
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *flip);

/* Driver functions when matching and unplug device*/
/* Called when matched platform device is found */
int pcd_platform_driver_probe(struct platform_device *pdev);
/* Called when the device is removed from the system */
int pcd_platform_driver_remove(struct platform_device *pdev);
/*
=======================================================================================
								LOCAL VARIABLE
=======================================================================================
*/
/* Config for device */
struct device_config pcdev_config[] = 
{
	[DEVICE0] = {.config_item1 = 60, .config_item2 = 21},
	[DEVICE1] = {.config_item1 = 50, .config_item2 = 22},
	[DEVICE2] = {.config_item1 = 40, .config_item2 = 23},
	[DEVICE3] = {.config_item1 = 30, .config_item2 = 24}
};

/* file operations of the driver */
struct file_operations pcd_fops=
{
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.owner = THIS_MODULE
};

/* Device private data */
driver_private_data driver_data;

/*
 * Struct used for matching a device
 */
static const struct of_device_id pcdev_dt_ids[] = {
    { .compatible = "org,DEVICE0", .data = (void *)DEVICE0},
    { .compatible = "org,DEVICE1", .data = (void *)DEVICE0},
    { .compatible = "org,DEVICE2", .data = (void *)DEVICE0},
    { .compatible = "org,DEVICE3", .data = (void *)DEVICE0},
    { /* sentinel */ }
};

/*
 * Platform driver struct provides information of device driver to the kernel
 */
static struct platform_driver pcd_platform_driver = {
    .driver = {
        .name = "pcd-driver",
        .of_match_table = of_match_ptr(pcdev_dt_ids),
    },
    .remove = pcd_platform_driver_remove,
    .probe = pcd_platform_driver_probe,
};

/*
=======================================================================================
								FUNCTION IMPLEMENTION
=======================================================================================
*/

/*Called when matched platform device is found */
int pcd_platform_driver_probe(struct platform_device *pdev)
{
	int ret;
	struct device_private_data *dev_data;
	struct device_platform_data *data;
	struct device *dev = &pdev->dev;
	int dev_order;
	/* used to store matched entry of 'of_device_id' list of this driver */
	const struct of_device_id *of_id = of_match_device(of_match_ptr(pcdev_dt_ids), dev);
	if (!of_id)
	{
		/* no device tree device. Match will always be NULL if LINUX doesnt support device tree i.e CONFIG_OF is off */
		pr_err("PCD: No device tree binding or device tree is disable");
		return -1;
	}
	else
	{
		data = device_get_platdata_from_dt(dev);
		if(IS_ERR(data))
			return PTR_ERR(data);
		dev_order = (int)of_id->data;
	}

	/* Dynamically allocate memory for the device private data  */
	devm_kzalloc(dev_data, sizeof(*dev_data),GFP_KERNEL);
	if(!dev_data){
		dev_info(dev,"Cannot allocate memory \n");
		return -ENOMEM;
	}

	/*save the device private data pointer in platform device structure */
	dev_set_drvdata(&pdev->dev,dev_data);

	print_device_info(data, pcdev_config[dev_order]);

	/* Dynamically allocate memory for the device buffer using size 
	information from the platform data */
	devm_kzalloc(dev_data->buffer,dev_data->data.size,GFP_KERNEL);
	if(!dev_data->buffer){
		dev_info(dev,"Cannot allocate memory \n");
		return -ENOMEM;
	}

	/* Get the device number */
	dev_data->dev_num = driver_data.device_num_base + driver_data.total_devices;

	/* Do cdev init and cdev add */
	cdev_init(&dev_data->cdev,&pcd_fops);
	
	dev_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&dev_data->cdev,dev_data->dev_num,1);
	if(ret < 0){
		dev_err(dev,"Cdev add failed\n");
		return ret;
	}

	/* Create device file for the detected platform device */
	driver_data.device_dev = device_create(driver_data.class_dev, dev, dev_data->dev_num,NULL,\
								"pcdev-%d",driver_data.total_devices);
	if(IS_ERR(driver_data.device_dev)){
		dev_err(dev,"Device create failed\n");
		ret = PTR_ERR(driver_data.device_dev);
		cdev_del(&dev_data->cdev);
		return ret;
		
	}

	pcdrv_data.total_devices++;

	dev_info(dev,"Probe was successful\n");

	return 0;
}

/*Called when the device is removed from the system */
int pcd_platform_driver_remove(struct platform_device *pdev)
{

#if 1
	struct pcdev_private_data  *dev_data = dev_get_drvdata(&pdev->dev);

	/*1. Remove a device that was created with device_create() */
	device_destroy(driver_data.class_dev,driver_data->dev_num);
	
	/*2. Remove a cdev entry from the system*/
	cdev_del(&dev_data->cdev);


	pcdrv_data.total_devices--;

#endif 
	dev_info(&pdev->dev,"A device is removed\n");
	return 0;
}

MODULE_DEVICE_TABLE(of, pcdev_dt_ids);

static int __init pcd_platform_driver_init(void)
{
  	pr_info("PCD: Init device driver for pseudo driver\n");
  	int ret = -1;

  	/* Allocate device number */
  	ret = alloc_chrdev_region(&driver_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
  	if(ret < 0){
	    pr_err("PCD: Fail to allocate dev num\n");
	    return ret;
  	}

  	/* Register class device */
	driver_data.class_dev = class_create(THIS_MODULE, "pcdclass");
	if(driver_data.class_dev == NULL){
		pr_err("PCD: Fail to create new class\n");
		/*Unregister device number*/
		unregister_chrdev_region(driver_data.device_num_base, MAX_DEVICES);
		return -1;
	}

	/*3. Register a platform driver */
	platform_driver_register(&pcd_platform_driver);
	
	pr_info("PCD: pcd platform driver loaded\n");
	
	return 0;
}

static void __exit pcd_platform_driver_cleanup(void)
{
	/*1.Unregister the platform driver */
	platform_driver_unregister(&pcd_platform_driver);

	/*2.Class destroy */
	class_destroy(driver_data.class_dev);

	/*3.Unregister device numbers for MAX_DEVICES */
	unregister_chrdev_region(driver_data.device_num_base,MAX_DEVICES);
	
	pr_info("PCD: pcd platform driver unloaded\n");

}


module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{

	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;

	int max_size = pcdev_data->pdata.size;
	
	loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);

	switch(whence)
	{
		case SEEK_SET:
			if((offset > max_size) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = max_size + offset;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}
	
	pr_info("New value of the file position = %lld\n",filp->f_pos);

	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;

	int max_size = pcdev_data->pdata.size;

	pr_info("Read requested for %zu bytes \n",count);
	pr_info("Current file position = %lld\n",*f_pos);

	
	/* Adjust the 'count' */
	if((*f_pos + count) > max_size)
		count = max_size - *f_pos;

	/*copy to user */
	if(copy_to_user(buff,pcdev_data->buffer+(*f_pos),count)){
		return -EFAULT;
	}

	/*update the current file postion */
	*f_pos += count;

	pr_info("Number of bytes successfully read = %zu\n",count);
	pr_info("Updated file position = %lld\n",*f_pos);

	/*Return number of bytes which have been successfully read */
	return count;

}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;

	int max_size = pcdev_data->pdata.size;
	
	pr_info("Write requested for %zu bytes\n",count);
	pr_info("Current file position = %lld\n",*f_pos);

	
	/* Adjust the 'count' */
	if((*f_pos + count) > max_size)
		count = max_size - *f_pos;

	if(!count){
		pr_err("No space left on the device \n");
		return -ENOMEM;
	}

	/*copy from user */
	if(copy_from_user(pcdev_data->buffer+(*f_pos),buff,count)){
		return -EFAULT;
	}

	/*update the current file postion */
	*f_pos += count;

	pr_info("Number of bytes successfully written = %zu\n",count);
	pr_info("Updated file position = %lld\n",*f_pos);

	/*Return number of bytes which have been successfully written */
	return count;

}



int pcd_open(struct inode *inode, struct file *filp)
{

	int ret;

	int minor_n;
	
	struct pcdev_private_data *pcdev_data;

	/*find out on which device file open was attempted by the user space */

	minor_n = MINOR(inode->i_rdev);
	pr_info("minor access = %d\n",minor_n);

	/*get device's private data structure */
	pcdev_data = container_of(inode->i_cdev,struct pcdev_private_data,cdev);

	/*to supply device private data to other methods of the driver */
	filp->private_data = pcdev_data;
		
	/*check permission */
	ret = check_permission(pcdev_data->pdata.perm,filp->f_mode);

	(!ret)?pr_info("open was successful\n"):pr_info("open was unsuccessful\n");

	return ret;
}

int pcd_release(struct inode *inode, struct file *flip)
{
	pr_info("release was successful\n");

	return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sinh Tran Van");
MODULE_DESCRIPTION("A pseudo character platform driver which handles n platform pcdevs");