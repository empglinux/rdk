# Makefile2.6
ifneq ($(KERNELRELEASE),)
obj-m := iscm.o
else
PWD  := $(shell pwd)
KVER ?= $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD)
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions Module.markers Module.symvers modules.order
endif
