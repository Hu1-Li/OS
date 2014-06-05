obj-m := 3.o
KERNELBUILD := /lib/modules/2.6.32-38-generic/build
default:
	make -C $(KERNELBUILD) M=$(shell pwd) modules
.PHONY:clean
clean:
	-rm -f $(OBJS) *.o *~ *.order *.symvers *.mod.c *.ko
