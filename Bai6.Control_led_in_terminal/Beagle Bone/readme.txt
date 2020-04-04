In this case, I wrote a program that make the beagle bone control gpio in P9, pin 23 (GPIO1_17) (gpio49) by writing to the device file created by LKM module.
You can control gpio by writing 1 or 0 to device file in /dev/gpio_ctrl.
how to build:
	run make
how to test:
Change the priviledge to super user by cmd: super su
Turn on: echo 1 > /dev/gpio_ctrl
Turn off: echo 0 > /dev/gpio_ctrl

You can switch the device file priviledge, that user can access directly, reference http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/