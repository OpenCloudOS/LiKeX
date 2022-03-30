obj-m:=mem_read.o


PWD:=$(shell pwd)


KERNELPATH:=/lib/modules/$(shell uname -r)/build


all:
	make -C $(KERNELPATH) M=$(PWD) modules
clean:
	make -C $(KERNELPATH) M=$(PWD) clean