obj-m += inspector.o
KDIR := /lib/modules/`uname -r`/build
PWD := `pwd`

default:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o .i* .m* .M*  .cmd *.ko *.mod.c .tmp_versions *.symvers *.mod *.order
