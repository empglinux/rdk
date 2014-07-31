ifneq ($(KERNELRELEASE),)
	obj-m := cmpc_pm.o
else
PWD  := $(shell pwd)
KVER ?= $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) 

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.markers Module.symvers modules.order
endif
