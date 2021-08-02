#include <stdio.h>
#include <stdlib.h>
#include <iolib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	static unsigned int gpioLED = 23;
	iolib_init();
	iolib_setdir(9, gpioLED, DIR_OUT);
	while(1)
	{
		pin_high(9, gpioLED);
		usleep (1000000);
		pin_low(9, gpioLED);
		usleep (1000000);
	}
	iolib_free();
	return 0;
}