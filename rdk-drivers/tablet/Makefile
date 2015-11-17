# Makefile2.6
ifneq ($(KERNELRELEASE),)
#kbuild syntax. dependency relationshsip of files and target modules are listed here.
#mymodule-objs := file1.o file2.o
#obj-m := mymodule.o 
obj-m := tablet.o
else
PWD  := $(shell pwd)
KVER ?= $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build
all:
	$(MAKE) -C $(KDIR) M=$(PWD) 
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions Module.markers Module.symvers modules.order
endif
