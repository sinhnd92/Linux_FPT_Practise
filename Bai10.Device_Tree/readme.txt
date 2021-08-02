Step 1: Build dts file
- Enter BBB Debian linux source code
- Copy am335x-boneblack.dts and am335x-boneblack-pcd.dtsi into ./arch/arm/boot/dts
- run make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am335x-boneblack.dtb

Step 2: Build driver
- Enter to the folder project
- On terminal, run "make"

Step 3: Copy device tree binary and .ko file to BBB
- On the current terminal, run "make copy-dtb"
- On the current terminal, run "make copy-drv"

Step 4: Add new dtb file to boot partition
- run command lsblk => check 2 boot partition mmcblk0p1 and mmcblk0p2
- mount into mnt: mount /dev/mmcblk0p1 /mnt
- remove the current am335x-boneblack.dtb and copy new file
- reboot

Step 5: Running
-sudo insmod file.ko