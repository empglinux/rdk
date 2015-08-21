OS_VER=Debian7.1
TODAY=$(shell expr `date +%Y%m%d` + 0 )
BIOS_VER=2012.1109.1952

PWD:=$(shell pwd)
BASEDIR:=$(shell basename `pwd`)
ROOT_DRV_DIR:=${PWD}
#DESTDIR=${PWD}/tmp
DESTDIR=
KVER?=$(shell uname -r)
KDIR=/lib/modules/$(KVER)/build
MODULE_DIR=/lib/modules/${KVER}/kernel/drivers/acpi
CONFLITDRV=/lib/modules/${KVER}/kernel/drivers/platform/x86/classmate-laptop.ko

VKD_DIR:=$(ROOT_DRV_DIR)/vkd
ACCEL_DIR:=$(ROOT_DRV_DIR)/accel
ISCM_DIR:=$(ROOT_DRV_DIR)/iscm
PM_DIR:=$(ROOT_DRV_DIR)/pm
TABLET_DIR:=$(ROOT_DRV_DIR)/tablet

RDK_DRIVERS_VER=1.0.1

#all: accel cmpc_pm cmpc_vkd iscm tablet
all: accel cmpc_pm cmpc_vkd tablet
	@echo ">>>>>>>>>>> rdk drivers install completely. <<<<<<<<<<<<"
	@rm -f ${CONFLITDRV} if [ -f ${CONFLITDRV} ]
	@depmod -a

${ACCEL_DIR}/accel.ko:
	$(MAKE) -f ${ACCEL_DIR}/Makefile -C ${ACCEL_DIR} KDIR=${KDIR}
${PM_DIR}/cmpc_pm.ko:
	$(MAKE) -f ${PM_DIR}/Makefile -C ${PM_DIR} KDIR=${KDIR}
${VKD_DIR}/cmpc_vkd.ko:
	$(MAKE) -f ${VKD_DIR}/Makefile -C ${VKD_DIR} KDIR=${KDIR}
${ISCM_DIR}/iscm.ko:
	$(MAKE) -C $(ISCM_DIR) KDIR=$(KDIR)
${TABLET_DIR}/tablet.ko:
	$(MAKE) -f ${TABLET_DIR}/Makefile -C ${TABLET_DIR} KDIR=${KDIR}

accel:  ${ACCEL_DIR}/accel.ko
	@install -d ${DESTDIR}${MODULE_DIR}
	@install -m 644 $< ${DESTDIR}${MODULE_DIR}

cmpc_pm: ${PM_DIR}/cmpc_pm.ko
	@install -d ${DESTDIR}${MODULE_DIR}
	@install -m 644 $< ${DESTDIR}${MODULE_DIR}

cmpc_vkd: ${VKD_DIR}/cmpc_vkd.ko
	@install -d ${DESTDIR}${MODULE_DIR}
	@install -m 644 $< ${DESTDIR}${MODULE_DIR}

iscm: ${ISCM_DIR}/iscm.ko
	@install -d ${DESTDIR}${MODULE_DIR}
	@install -m 644 $< ${DESTDIR}${MODULE_DIR}

tablet: ${TABLET_DIR}/tablet.ko
	@install -d ${DESTDIR}${MODULE_DIR}
	@install -m 644 $< ${DESTDIR}${MODULE_DIR}

#fnkey_drivers_install: accel cmpc_pm cmpc_vkd

uninstall:
	@echo "uninstall function key drivers..."
	@$(shell [ -f ${MODULE_DIR}/accel.ko ] && rm -f ${MODULE_DIR}/accel.ko)
	@$(shell [ -f ${MODULE_DIR}/cmpc_vkd.ko ] && rm -f ${MODULE_DIR}/cmpc_vkd.ko)
	@$(shell [ -f ${MODULE_DIR}/cmpc_pm.ko ] && rm -f ${MODULE_DIR}/cmpc_pm.ko)
	@$(shell [ -f ${MODULE_DIR}/iscm.ko ] && rm -f ${MODULE_DIR}/iscm.ko)
	@$(shell [ -f ${MODULE_DIR}/tablet.ko ] && rm -f ${MODULE_DIR}/tablet.ko)
	@echo "function key drivers have been uninstalled."

dist-bzip2: ${ACCEL_DIR} ${PM_DIR} ${VKD_DIR}
	@make -C ${ACCEL_DIR} clean 
	@make -C ${PM_DIR} clean 
	@make -C ${VKD_DIR} clean 
#	@make -C ${ISCM_DIR} clean 
	@mkdir ${BASEDIR}
	@cp -rf ${PWD}/Makefile ${BASEDIR}
	@cp -rf ${ACCEL_DIR} ${BASEDIR}
	@cp -rf ${PM_DIR} ${BASEDIR}
	@cp -rf ${VKD_DIR} ${BASEDIR}
	@tar czf ${BASEDIR}.tar.gz ${BASEDIR}
	@rm -rf ${BASEDIR}

clean:
	@rm -rf tmp 
	@rm -rf ${BASEDIR}.tar.gz
	@make -C ${ACCEL_DIR} clean 
	@make -C ${PM_DIR} clean 
	@make -C ${VKD_DIR} clean 
	@make -C ${ISCM_DIR} clean 
	@make -C ${TABLET_DIR} clean 

.PHONY: accel cmpc_pm cmpc_vkd iscm tablet fnkey_drivers_install dist-bzip2 clean
