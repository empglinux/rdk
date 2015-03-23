/*
*  /file tablet.c
* Copyright (C) 2008  Intel Corporation
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* zhu kaiyue <yuex.k.zhu@intel.com> 
*/
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#include <acpi/acpi_drivers.h>
#include <acpi/acnames.h>
#include <asm/uaccess.h>

#define CMPC_CLASS_NAME "cmpc"
#define CMPC_TSD_NAME   "tablet"

#define TABLET_SENSOR_MAGIC 0xfc
#define REGISTER_EVENT_CMD 0x0
#define DBG_SIGNAL_EVENT_CMD 0x01
#define UNREGISTER_EVENT_CMD 0x10

// Tablet sensor status change notification value
#define TSD_DEVICE_INFO_CHANGE     0x81

#define IOCTL_REGISTER_EVENT _IO( TABLET_SENSOR_MAGIC, REGISTER_EVENT_CMD )

#define IOCTL_UNREGISTER_EVENT _IO( TABLET_SENSOR_MAGIC, UNREGISTER_EVENT_CMD )

#define IOCTL_DBG_SIGNAL_EVENT _IOR( TABLET_SENSOR_MAGIC, DBG_SIGNAL_EVENT_CMD, 1)


#ifdef DEBUG_TABLET
#define dbg_print(format, arg...) \
	printk(KERN_DEBUG "%s " format, __func__, ##arg)
#else
#define dbg_print(format, arg...)
#endif

/** function definitions */
/** common driver functions */
static int cmpc_tsd_init(void);
static void cmpc_tsd_exit(void);
static int cmpc_tsd_open(struct inode *inode, struct file *file);
static int cmpc_tsd_release(struct inode *inode, struct file *file);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long cmpc_tsd_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int cmpc_tsd_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static ssize_t cmpc_tsd_read(struct file *file, char __user *buf,
	size_t size, loff_t *ppos);
static ssize_t cmpc_tsd_write(struct file *file, const char __user *buf,
	size_t size, loff_t *ppos);
static loff_t cmpc_tsd_llseek(struct file *file, loff_t offset, int orig);

/** acpi notify handler */
static void cmpc_tsd_handler(acpi_handle handle, u32 event, void *data);

/** acpi driver ops */
static int cmpc_tsd_add(struct acpi_device *device);
#if 0
static int cmpc_tsd_resume(struct acpi_device *device);
#endif
#if 0
static int cmpc_tsd_remove(struct acpi_device *device, int type);
#else
static int cmpc_tsd_remove(struct acpi_device *device);
#endif

static int cmpc_tsd_get_status(int *res);

static const struct acpi_device_id tsd_device_ids[] = {
	{"TBLT0000", 0},
	{"", 0},
};

MODULE_DEVICE_TABLE(acpi, tsd_device_ids);

static struct acpi_driver tsd_drv = {
	.owner = THIS_MODULE,
	.name  = CMPC_TSD_NAME,
	.class = CMPC_CLASS_NAME,
	.ids   = tsd_device_ids,
	.ops   = {
		.add    = cmpc_tsd_add,
		.remove = cmpc_tsd_remove,
#if 0
		.resume = cmpc_tsd_resume,
#endif
	},
};

/** CMPC Virtual Key Drivercmpc_tsd_ struct */
struct cmpc_tsd_dev {
	struct acpi_device *device;	/** acpi bus device */
	u32                event;	/** acpi event */
};

static struct cmpc_tsd_dev *tsd_dev;

static const struct file_operations tsd_fops = {
	.owner   = THIS_MODULE,
	.open    = cmpc_tsd_open,
	.release = cmpc_tsd_release,
	.read    = cmpc_tsd_read,
	.write   = cmpc_tsd_write,
	.llseek  = cmpc_tsd_llseek,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl   = cmpc_tsd_ioctl,
#else
	.ioctl = cmpc_tsd_ioctl,
#endif
};



static int __init cmpc_tsd_init(void)
{
	dbg_print("----->\n");
	
	/** get memory forstruct input_dev *input; device struct */
	tsd_dev = kzalloc(sizeof(struct cmpc_tsd_dev), GFP_KERNEL);
	if (!tsd_dev) {
		dbg_print("kmalloc for device struct failed\n");
		dbg_print("<-----\n");
		goto fail0;
	}

	/** register bus driver */
	if (acpi_bus_register_driver(&tsd_drv) < 0) {
		dbg_print("acpi_bus_register_driver failed\n");
		goto fail1;
	}

	dbg_print("<-----\n");
	return 0;

fail1:
	kfree(tsd_dev);

fail0:
	dbg_print("<-----\n");
	return -1;
}

static void __exit cmpc_tsd_exit(void)
{
	dbg_print("----->\n");

	acpi_bus_unregister_driver(&tsd_drv);
	kfree(tsd_dev);
	
	dbg_print("<-----\n");
}

static int cmpc_tsd_open(struct inode *inode, struct file *file)
{
	dbg_print("----->\n");
	dbg_print("<-----\n");
	return 0;
}

static int cmpc_tsd_release(struct inode *inode, struct file *file)
{
	dbg_print("----->\n");
	dbg_print("<-----\n");
	return 0;
}

#if 0
static int cmpc_tsd_resume(struct acpi_device *device) 
{
	dbg_print("----->\n");
	dbg_print("<-----\n");
	return 0;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long cmpc_tsd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int cmpc_tsd_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	int result=0;
	dbg_print("----->\n");
	switch (cmd)
	{
	case REGISTER_EVENT_CMD:
		/** install acpi notify handler */
		if (!ACPI_SUCCESS(acpi_install_notify_handler(tsd_dev->device->handle,
			ACPI_DEVICE_NOTIFY, cmpc_tsd_handler, tsd_dev))) {
			dbg_print("acpi_install_notify_handler failed\n");
		}
		break;
	case UNREGISTER_EVENT_CMD:
		if (!ACPI_SUCCESS(acpi_remove_notify_handler(tsd_dev->device->handle, ACPI_DEVICE_NOTIFY, 
		cmpc_tsd_handler))) {
			dbg_print("acpi_remove_notify_handler failed\n");
		}
		break;
	case DBG_SIGNAL_EVENT_CMD:
		cmpc_tsd_get_status(&result);
		if (put_user(result,(int *) arg))
		{
			return - EFAULT;
		}
		break;
	default:
		dbg_print("not supported cmd\n");

		return -EINVAL;
	}
	dbg_print("<-----\n");
	return 0;
}


/** get current tablet sensor status: clamshell or tablet mode */
static int cmpc_tsd_get_status(int *res) 
{

	struct acpi_object_list params;
	union acpi_object in_objs;
	union acpi_object out_obj;
	struct acpi_buffer result, *resultp;
	acpi_status status;
	int success = 0;

	dbg_print("----->\n");

	*res = 0;
	result.length = sizeof(out_obj);
	result.pointer = &out_obj;
	resultp = &result;

	params.count = 1;
	params.pointer = &in_objs;
	in_objs.integer.value = 1;
	in_objs.type = ACPI_TYPE_INTEGER;
	if (!ACPI_SUCCESS(status = acpi_evaluate_object(
		tsd_dev->device->handle, "TCMD", &params, resultp))) {
		dbg_print("acpi_evaluate_object failed\n");
		goto fail;
	}
	if (res)
		*res = out_obj.integer.value;

	success = status == AE_OK && out_obj.type == ACPI_TYPE_INTEGER;
	if (!success)
		dbg_print("acpi_evaluate_object failed\n");
	return success;

fail:
	dbg_print("<-----\n");
	return -1;
}

static ssize_t cmpc_tsd_read(struct file *file, char __user *buf,
	size_t size, loff_t *ppos)
{
	int result = 0;
	dbg_print("----->\n");

	cmpc_tsd_get_status(&result);

	if (copy_to_user(buf, &result, sizeof(int)))
	{
		return - EFAULT;
	}

	dbg_print("<-----\n");
	return 0;
}

static ssize_t cmpc_tsd_write(struct file *file, const char __user *buf,
	size_t size, loff_t *ppos)
{
	dbg_print("----->\n");
	dbg_print("<-----\n");
	return 0;
}

static loff_t cmpc_tsd_llseek(struct file *file, loff_t offset, int orig)
{
	dbg_print("----->\n");
	dbg_print("<-----\n");
	return 0;
}


static void cmpc_tsd_handler(acpi_handle handle, u32 event, void *data)
{
	dbg_print("----->\n");
	dbg_print("acpi event: %d\n", event);
	dbg_print("<-----\n");
}


static int cmpc_tsd_add(struct acpi_device *device) 
{
	struct proc_dir_entry *tsd_entry;
	
	dbg_print("----->\n");

#if 0
	tsd_entry = create_proc_entry(CMPC_TSD_NAME, 0777, acpi_root_dir);
#else
	tsd_entry = proc_create_data(CMPC_TSD_NAME, 0777, acpi_root_dir, &tsd_fops, tsd_dev);
#endif
	if (!tsd_entry) {
		dbg_print("create_proc_entry failed\n");
		goto fail0;
	}	

#if 0
	tsd_entry->data = tsd_dev;
	tsd_entry->proc_fops = &tsd_fops;
#endif
	
	tsd_dev->device = device;
	
	strcpy(acpi_device_name(device), CMPC_TSD_NAME);
	sprintf(acpi_device_class(device), "%s/%s", CMPC_CLASS_NAME,
		CMPC_TSD_NAME);

	/** install acpi notify handler */
	if (!ACPI_SUCCESS(acpi_install_notify_handler(device->handle,
		ACPI_DEVICE_NOTIFY, cmpc_tsd_handler, tsd_dev))) {
		dbg_print("acpi_install_notify_handler failed\n");
		goto fail1;
	}

	dbg_print("<-----\n");
	return 0;

fail1:
	remove_proc_entry(CMPC_TSD_NAME, acpi_root_dir);
	
fail0:
	dbg_print("<-----\n");
	return -ENODEV;
}


#if 0
static int cmpc_tsd_remove(struct acpi_device *device, int type) 
#else
static int cmpc_tsd_remove(struct acpi_device *device) 
#endif
{
	dbg_print("----->\n");

	acpi_remove_notify_handler(tsd_dev->device->handle, ACPI_DEVICE_NOTIFY, 
		cmpc_tsd_handler);
	remove_proc_entry(CMPC_TSD_NAME, acpi_root_dir);

	dbg_print("<-----\n");
	return 0;
}

module_init(cmpc_tsd_init);
module_exit(cmpc_tsd_exit);

ACPI_MODULE_NAME("cmpc_tablet");
MODULE_AUTHOR("Cui Yi");
MODULE_DESCRIPTION("CMPC Tablet Sensor Driver");
MODULE_VERSION("1.0.0.0");
MODULE_LICENSE("GPL");
