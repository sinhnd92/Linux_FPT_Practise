#ifndef PCD_FUNCTION_H
#define PCD_FUNCTION_H

#include "pcd_define.h"

int check_permission(int dev_perm, int acc_mode);
struct device_platform_data* device_get_platdata_from_dt(struct device *dev);

void print_device_info(device_platform_data * dev_plat_data, device_config dev_cfg);

#endif
