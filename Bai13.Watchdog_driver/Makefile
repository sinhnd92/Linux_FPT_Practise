obj-m += bbb_fpt_wdt.o

src-file := bbb_fpt_wdt.c
test-file:= main.c

PWD := $(shell pwd)
GCC := /home/sinhnd92/arm-linux-toolchain/bin/arm-linux-gnueabihf-
KERNEL := /home/sinhnd92/BB-Black/linux

all:
	make  ARCH=arm CROSS_COMPILE=$(GCC) -C $(KERNEL) SUBDIRS=$(PWD) modules
	$(GCC)gcc main.c -o main
clean:
	make  -C $(KERNEL) SUBDIRS=$(PWD) clean
	rm main
check:
	./checkpatch.pl --no-tree --ignore CONST_STRUCT --show-types -f $(src-file)
