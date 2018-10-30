obj-m := sendudp.o
KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
default:
	$(MAKE) -C -Wall $(KDIR) M=$(PWD) modules
clean:
	rm -f *.o *.ko *.mod.c