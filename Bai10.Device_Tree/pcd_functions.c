
#include "pcd_functions.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

int check_permission(int dev_perm, int acc_mode)
{

	if(dev_perm == RDWR)
		return 0;
	
	//ensures readonly access
	if( (dev_perm == RDONLY) && ( (acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE) ) )
		return 0;
	
	//ensures writeonly access
	if( (dev_perm == WRONLY) && ( (acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ) ) )
		return 0;

	return -EPERM;
}

struct device_platform_data* device_get_platdata_from_dt(struct device *dev)
{
	struct device_node *dev_node = dev->of_node;
	struct device_platform_data *pdata;

	if(!dev_node)
		/* this probe didnt happen because of device tree node */
		return NULL;

	pdata = devm_kzalloc(dev,sizeof(*pdata),GFP_KERNEL);

	if(!pdata){
		dev_info(dev,"Cannot allocate memory \n");
		return ERR_PTR(-ENOMEM);
	}

	if(of_property_read_string(dev_node,"org,device-serial-num",&pdata->serial_number) ){
		dev_info(dev,"Missing serial number property\n");
		return ERR_PTR(-EINVAL);

	}


	if(of_property_read_u32(dev_node,"org,size",&pdata->size) ){
		dev_info(dev,"Missing size property\n");
		return ERR_PTR(-EINVAL);
	}

	if(of_property_read_u32(dev_node,"org,perm",&pdata->perm) ){
		dev_info(dev,"Missing permission property\n");
		return ERR_PTR(-EINVAL);
	}


	return pdata;
}

void print_device_info(device_platform_data * dev_plat_data, device_config dev_cfg)
{
	pr_info("PCD: Device serial number = %s\n",dev_plat_data->serial_number);
	pr_info("PCD: Device size = %d\n", dev_plat_data->size);
	pr_info("PCD: Device permission = %d\n",dev_plat_data->permission);

	pr_info("PCD: Config item 1 = %d\n",dev_cfg.config_item1 );
	pr_info("PCD: Config item 2 = %d\n",dev_cfg.config_item2 );
}