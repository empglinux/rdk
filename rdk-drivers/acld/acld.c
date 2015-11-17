/*
 * Accelerometer driver implementation
 * Copyright (c) 2008, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/types.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)
#include <linux/acpi.h>
#endif

#include <linux/fs.h>
#include <linux/interrupt.h>
#include <acpi/acpi_drivers.h>
#include <acpi/acnames.h>
#include <asm/uaccess.h>
#include "accelio.h"

#define CMPC_CLASS_NAME    "cmpc"
#define ACCEL_NAME         "acld"

/* Keyboard disable/enable method name */
#define ACCEL_KB_METHOD   "KBCN"

/* Accelerometer AML method name */
#define ACCEL_AML_METHOD   "ACMD"
/* Accelerometer notify value */
#define ACCEL_NOTIFY        0x81
/* Accelerometer notify a  free-fall or activity event */
#define ACCEL_FREEFALL        0x82
/* AML method first parameter value to read data */
#define ACCEL_AML_READ_DATA 0x01
/* AML method first parameter value to set sensitivity */
#define ACCEL_AML_SET_SENSE 0x02
/* AML method first parameter value to start accelerometer */
#define ACCEL_AML_START     0x03
/* AML method first parameter value to stop accelerometer */
#define ACCEL_AML_STOP      0x04
/* AML method first parameter value to set g-select */
#define ACCEL_AML_SET_G_SELECT 0x05
/* AML method first parameter value to get g-select */
#define ACCEL_AML_GET_G_SELECT 0x06
/* AML method first parameter value to  set axises offset for calibration */
#define ACCEL_AML_SET_G_OFFSET 0x07
/* AML method first parameter value to  set Activity Thresh */
#define ACCEL_AML_SET_ACTIVITY_THRESH 0x08
/* AML method first parameter value to  set Freefall Thresh */
#define ACCEL_AML_SET_FREEFALL_THRESH 0x09

#define ACCEL_NOUSER		0
#define ACCEL_ONEUSER		1


#ifdef DEBUG_ACCEL
#define dbg_print(format, arg...) \
	printk(KERN_DEBUG "%s " format, __func__, ##arg)
#else
#define dbg_print(format, arg...)
#endif

/* Accelerometer device state struct */
enum accel_state {
	ACCEL_NOT_STARTED = 0,
	ACCEL_STARTED,
	ACCEL_SUSPENDED,
	ACCEL_STOPPED,
	ACCEL_REMOVED,
};

#define INIT_ACCEL_STATE(adev) \
	(adev)->curr_state = ACCEL_NOT_STARTED;\
	(adev)->prev_state = ACCEL_NOT_STARTED;
#define SET_NEW_ACCEL_STATE(adev, ACCEL_STATE) \
	(adev)->prev_state = (adev)->curr_state;\
	(adev)->curr_state = ACCEL_STATE;
#define REST_PREV_ACCEL_STATE(adev) \
	(adev)->curr_state = (adev)->prev_state;\

/* common driver functions */
static int accel_open(struct inode *inode, struct file *file);
static int accel_release(struct inode *inode, struct file *file);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long accel_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int accel_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static ssize_t accel_read(struct file *file, char __user *buf,
	size_t size, loff_t *ppos);

/* acpi driver ops */
static int accel_add(struct acpi_device *device);
#if 0
static int accel_remove(struct acpi_device *device, int type);
#else
static int accel_remove(struct acpi_device *device);
#endif
#if 0
static int accel_start(struct acpi_device *device);
#endif
#ifdef CONFIG_PM_SLEEP
static int accel_suspend(struct device *dev);
#else
static int accel_suspend(struct acpi_device *device, pm_message_t state);
#endif
static int accel_resume(struct device *dev);

/* CMPC accel AML commands */
acpi_status accel_aml_start(void);
acpi_status accel_aml_stop(void);
acpi_status accel_aml_set_sense(int sense);
acpi_status accel_aml_read_data(struct accel_raw_data *data);
acpi_status accel_aml_set_g_select(int select);
acpi_status accel_aml_get_g_select(int *select);
acpi_status accel_aml_set_g_offset(struct accel_raw_data *data);
acpi_status accel_aml_set_g_activity_thresh(int select);
acpi_status accel_aml_set_g_freefall_thresh(int select);

/* CMPC accel Keyboard command */
acpi_status accel_kb_control(int flag);

/* Accelerometer device struct */
struct accel_device {
	struct acpi_device *device;				/* acpi bus device */
	int                freefall;		 	/* freeball state value */
	unsigned int       accel_cnt;		 	/* reference count for accel device access */
	enum accel_state   curr_state;			/* accel current state */
	enum accel_state   prev_state;			/* accel previous state */
	wait_queue_head_t  freefall_wq;	/* < wait queue for notify */
	wait_queue_head_t  waitq;	/* < wait queue for read */
	int                freefall_wf;	/* < flag for freefall wait interruptible */
	int                waitf;	/* < flag for read wait interruptible */
};


static unsigned long s_three[4] = {0};
static struct accel_device *adev;

static const struct file_operations accel_fops = {
	.owner   = THIS_MODULE,
	.open    = accel_open,
	.release = accel_release,
	.read    = accel_read,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl   = accel_ioctl,
#else
	.ioctl = accel_ioctl,
#endif
};


static int accel_open(struct inode *inode, struct file *file)
{
	dbg_print("----->\n");

	if (!adev) {
		printk("can not open accel device file\n");
		return -ENODEV;
	}

	if ((ACCEL_NOT_STARTED == adev->curr_state) && (ACCEL_NOUSER == adev->accel_cnt)) {
		if (ACPI_SUCCESS(accel_aml_start())) {
			SET_NEW_ACCEL_STATE(adev, ACCEL_STARTED);
		} else {
			dbg_print("start accel failed\n");
			return -EFAULT;
		}
	}

	adev->accel_cnt++;

	dbg_print("<-----\n");
	return 0;
}


static int accel_release(struct inode *inode, struct file *file)
{
	dbg_print("----->\n");
	
	/* stop accel */
	if ((ACCEL_STARTED == adev->curr_state) && (ACCEL_ONEUSER == adev->accel_cnt)) {
		if (ACPI_SUCCESS(accel_aml_stop())) {
			SET_NEW_ACCEL_STATE(adev, ACCEL_NOT_STARTED);
			/* wake up blocked read */
			adev->freefall_wf = 1;
			wake_up_interruptible(&adev->freefall_wq);
		} else {
			dbg_print("stop accel failed\n");
			return -EFAULT;
		}
	}

	adev->accel_cnt--;

	dbg_print("<-----\n");
	return 0;
}


/* Call AML method to set activity-thresh.
 * @param set a activity-thresh
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_set_g_activity_thresh(int select)
{
	struct acpi_object_list params;
	union acpi_object in_objs[4];

	params.count = 4;
	params.pointer = &in_objs[0];
	in_objs[0].integer.value = ACCEL_AML_SET_ACTIVITY_THRESH;
	in_objs[0].type = ACPI_TYPE_INTEGER;
	in_objs[1].integer.value = select;
	in_objs[1].type = ACPI_TYPE_INTEGER;
	in_objs[2].type = ACPI_TYPE_INTEGER;
	in_objs[2].integer.value = 0;
	in_objs[3].type = ACPI_TYPE_INTEGER;
	in_objs[3].integer.value = 0;

	return acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, NULL);
}


/* Call AML method to set freefall-thresh.
 * @param set a freefall-thresh
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_set_g_freefall_thresh(int select)
{
	struct acpi_object_list params;
	union acpi_object in_objs[4];

	params.count = 4;
	params.pointer = &in_objs[0];
	in_objs[0].integer.value = ACCEL_AML_SET_FREEFALL_THRESH;
	in_objs[0].type = ACPI_TYPE_INTEGER;
	in_objs[1].integer.value = select;
	in_objs[1].type = ACPI_TYPE_INTEGER;
	in_objs[2].type = ACPI_TYPE_INTEGER;
	in_objs[2].integer.value = 0;
	in_objs[3].type = ACPI_TYPE_INTEGER;
	in_objs[3].integer.value = 0;

	return acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, NULL);
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long accel_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int accel_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct accel_raw_data data;
	int select;

	dbg_print("----->\n");

	/* IOCTL START/STOP only set accel state to STARTED/NOT_STARTED.
	 * Only system calls can set accel state to SUSPENDED/STOPPED/REMOVED.
	 * If accel is in SUSPENDED/STOPPED/REMOVED, it cannot be started.
	 * Only system calls can resume or start it. But the start system
	 * call cannot start accel really. It only set it to initial state. 
	 */
	switch (cmd) {
	case IOCTL_ACCEL_START:
		if (ACCEL_NOT_STARTED == adev->curr_state) {
			if (ACPI_SUCCESS(accel_aml_start())) {
				SET_NEW_ACCEL_STATE(adev, ACCEL_STARTED);
			} else {
				dbg_print("start accel failed\n");
				return -EFAULT;
			}
		} else if (ACCEL_STARTED != adev->curr_state) {
			dbg_print("accel state error: %d\n", adev->curr_state);
			return -EFAULT;
		}	
		break;

	case IOCTL_ACCEL_STOP:
		if (ACCEL_STARTED == adev->curr_state) {
			if (ACPI_SUCCESS(accel_aml_stop())) {
				SET_NEW_ACCEL_STATE(adev, ACCEL_NOT_STARTED);
			} else {
				dbg_print("stop accel failed\n");
				return -EFAULT;
			}
		} else if (ACCEL_STOPPED != adev->curr_state) {
			dbg_print("accel state error: %d\n", adev->curr_state);
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_SET_SENSE:
		/** the sensitivity should never be greater than 128 */
#if 0
		if (arg > 128) {
			dbg_print("the sensitivity %lx is not correct\n", arg);
			return -EINVAL;
		}
#endif
		s_three[0] = arg;

		if (!ACPI_SUCCESS(accel_aml_set_sense(arg))) {
			dbg_print("set sensitivity failed\n");
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_SET_G_SELECT:
#if 0
		/** the g-select should be 0\1\2\3 **/
		if ((0 != arg) && (1 != arg)) {
			dbg_print("the g-select %lx is not correct\n", arg);
			return -EINVAL;
		}
#endif
		dbg_print("set g-select %lx \n", arg);
		s_three[1] = arg;
		if (!ACPI_SUCCESS(accel_aml_set_g_select(arg))) {
			dbg_print("set g-select failed\n");
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_GET_G_SELECT:
		if (!ACPI_SUCCESS(accel_aml_get_g_select(&select))) {
			dbg_print("get g-select failed\n");
			return -EFAULT;
		}
		if (copy_to_user((void __user *)arg, &select, sizeof(select))) {
			dbg_print("copy_to_user for g-select failed\n");
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_SET_G_OFFSET:
		/** the g-offset should be x,y,z data */
		
		if (copy_from_user(&data,(void  __user *) arg, sizeof(data))) {
			dbg_print("copy_from_user for g-offset failed\n");
			return -EFAULT;
		}

		if (!ACPI_SUCCESS(accel_aml_set_g_offset(&data))) {
			dbg_print("set g-offset failed\n");
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_SET_G_ACTIVITY_THRESH:
	/** the g-activity should be 0,1-255 */
#if 0
		if (arg > 255) {
			dbg_print("the activity %lx is not correct\n", arg);
			return -EINVAL;
		}
#endif
		s_three[2] = arg;
		if (!ACPI_SUCCESS(accel_aml_set_g_activity_thresh(arg))) {
			dbg_print("set g-activity failed\n");
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_SET_G_FREEFALL_THRESH:
	/**set the g-freefall sensetivity should be 255-1 */
#if 0
		if (arg > 255) {
			dbg_print("the freefall  %lx is not correct\n", arg);
			return -EINVAL;
		}
#endif
		dbg_print("the freefall  %lx \n", arg);
		s_three[3] = arg;
		if (!ACPI_SUCCESS(accel_aml_set_g_freefall_thresh(arg))) {
			dbg_print("set g-freefall failed\n");
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_GET_G_FREEFALL_THRESH:
	/** the g-freefall event 0 or 1 */
		dbg_print("ioctl get g freefall. --> before block\n");
		if (wait_event_interruptible(adev->freefall_wq, adev->freefall_wf != 0) != 0) {
			dbg_print("waiting is interrupted by signal\n");
			return -ERESTARTSYS;
		}
		dbg_print("ioctl get g freefall. --> after block\n");
		adev->freefall_wf = 0;
		if (ACCEL_STARTED == adev->curr_state) {
			    if (copy_to_user((void __user *)arg, &adev->freefall, sizeof(adev->freefall))) {
					dbg_print("copy_to_user for freefall failed\n");
					return -EFAULT;
				}
		} else {
			dbg_print("accel state error: %d\n", adev->curr_state);
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_SUSPEND:		
		/* stop accel if it is started */
		if (ACCEL_STARTED == adev->curr_state) {
			SET_NEW_ACCEL_STATE(adev, ACCEL_SUSPENDED);
		} else if (ACCEL_SUSPENDED == adev->curr_state) {
			dbg_print("accel state error: %d\n", adev->curr_state);
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_RESUME:		
		/* start accel if it is stoped */
		if (ACCEL_SUSPENDED == adev->curr_state) {
			SET_NEW_ACCEL_STATE(adev, ACCEL_STARTED);
		} else if (ACCEL_STARTED == adev->curr_state) {
			dbg_print("accel state error: %d\n", adev->curr_state);
			return -EFAULT;
		}
		break;

	case IOCTL_ACCEL_KBCONTROL:
		/* enable or disable keyboard */
		if (!ACPI_SUCCESS(accel_kb_control(arg))) {
			dbg_print("set keyboard status: %d\n", arg);
			return -EFAULT;
		}
		break;

	default:
		dbg_print("invalid ioctl commands\n");
		return -ENOTTY;
	}
	
	dbg_print("<-----\n");
	return 0;
}


static ssize_t accel_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	struct accel_raw_data data;

	dbg_print("----->%s() line %d\n", __FUNCTION__, __LINE__);
	
	if (sizeof(data) > size) {
		dbg_print("data size is too small: %d/%d\n", sizeof(data), size);
		return -EINVAL;
	}

	if (!(file->f_flags & O_NONBLOCK)) {
		if (wait_event_interruptible(adev->waitq, adev->waitf != 0) != 0) {
			dbg_print("waiting is interrupted by signal\n");
			return -ERESTARTSYS;
		}
		adev->waitf = 0;
	}
	dbg_print("----->%s() line %d\n", __FUNCTION__, __LINE__);
	
	if (adev->curr_state == ACCEL_STARTED) {
		if (!ACPI_SUCCESS(accel_aml_read_data(&data))) {
			dbg_print("accel_aml_read_data failed\n");
			return -EFAULT;
		}
		if (copy_to_user(buf, &data, sizeof(data)) != 0) {
			printk("copy_to_user failed\n");
			return -EIO;
		}

		dbg_print("----->%s() line %d\n", __FUNCTION__, __LINE__);
		return sizeof(data);
	} else {
		dbg_print("accel read state error: %d\n", adev->curr_state);
		return 0;
	}
}


static void accel_handler(acpi_handle handle, u32 event, void *data)
{
//	dbg_print("accel handler----->event : %x\n", event);

	switch (event) {
	case ACCEL_NOTIFY:
		adev->waitf = 1;
		wake_up_interruptible(&adev->waitq);
		break;
	case ACCEL_FREEFALL:
		dbg_print("ACCEL freefall\n");
		adev->freefall_wf = 1;
		adev->freefall = 1;
		wake_up_interruptible(&adev->freefall_wq);
		break;
	default:
		dbg_print("accel notify type error\n");
		break;
	}

//	dbg_print("<-----\n");
}


static int accel_add(struct acpi_device *device)
{
	struct proc_dir_entry *accel_entry;

	dbg_print("----->\n");

	/* get memory for device struct */
	adev = kzalloc(sizeof(struct accel_device), GFP_KERNEL);
	if (!adev) {
		printk("accel dev malloc failed\n");
		dbg_print("<-----\n");
		return -ENOMEM;
	}

#if 0
	accel_entry = create_proc_entry(ACCEL_NAME, 0777, acpi_root_dir);
#else
	accel_entry = proc_create_data(ACCEL_NAME, 0777, acpi_root_dir, &accel_fops, adev);
#endif
	if (!accel_entry) {
		printk("create_proc_entry failed\n");
		goto fail0;
	}	

#if 0
	accel_entry->data = adev;
	accel_entry->proc_fops = &accel_fops;
#endif
	
	adev->device = device;
	
	strcpy(acpi_device_name(device), ACCEL_NAME);
	sprintf(acpi_device_class(device), "%s/%s", CMPC_CLASS_NAME,
		ACCEL_NAME);

	adev->freefall = 0;	
	adev->accel_cnt = 0;
	adev->freefall_wf = 0;
	adev->waitf = 0;
	/** set accel initial state */
	INIT_ACCEL_STATE(adev);
	init_waitqueue_head(&adev->freefall_wq);
	init_waitqueue_head(&adev->waitq);

	/* install acpi notify handler */
	if (!ACPI_SUCCESS(acpi_install_notify_handler(device->handle,
		ACPI_DEVICE_NOTIFY, accel_handler, adev))) {
		printk("acpi_install_notify_handler failed\n");
		goto fail1;
	}

	dbg_print("<-----\n");
	return 0;

fail1:
	remove_proc_entry(ACCEL_NAME, acpi_root_dir);

fail0:
	kfree(adev);
	dbg_print("<-----\n");
	return -ENODEV;
}


#if 0
static int accel_remove(struct acpi_device *device, int type)
#else
static int accel_remove(struct acpi_device *device)
#endif
{
	dbg_print("----->\n");

	/* stop accel if it is started */
	if (ACCEL_STARTED == adev->curr_state) {
		if (!ACPI_SUCCESS(accel_aml_stop())) {
			dbg_print("stop accel failed\n");
			return -EFAULT;
		}
	}

	SET_NEW_ACCEL_STATE(adev, ACCEL_REMOVED);
	acpi_remove_notify_handler(device->handle, ACPI_DEVICE_NOTIFY,
		accel_handler);
	remove_proc_entry(ACCEL_NAME, acpi_root_dir);
	kfree(adev);

	dbg_print("<-----\n");
	return 0;
}


#if 0
static int accel_start(struct acpi_device *device) 
{
	dbg_print("----->\n");
	
	/* Only set accel state to initial state since only IOCTL
	 * START can really start accel. But system call can stop
	 * accele. If accel is stopped by system call, accel plugin
	 * cannot start it again, until accel_start set the initial
	 * state again.
	 */
	if (ACCEL_STOPPED == adev->curr_state) {
		INIT_ACCEL_STATE(adev);
	} else if (ACCEL_NOT_STARTED != adev->curr_state) {
		dbg_print("accel state error: %d\n", adev->curr_state);
		return -EFAULT;
	}

	dbg_print("<-----\n");
	return 0;
}
#endif


#ifdef CONFIG_PM_SLEEP
static int accel_suspend(struct device *dev)
#else
static int accel_suspend(struct acpi_device *device, pm_message_t state)
#endif
{
	dbg_print("----->\n");

	/* stop accel if it is started */
	if (ACCEL_STARTED == adev->curr_state) {
		if (!ACPI_SUCCESS(accel_aml_stop())) {
			dbg_print("stop accel failed\n");
			return -EFAULT;
		}
	}

	SET_NEW_ACCEL_STATE(adev, ACCEL_SUSPENDED);

	dbg_print("<-----\n");
	return 0;
}


static int accel_resume(struct device *dev)
{
	int arg;
	dbg_print("----->\n");

	/* When accel resume, restore the state to previous. If it was
	 * started by IOCTL START before, start it again.
	 */
	REST_PREV_ACCEL_STATE(adev);

	if (ACCEL_STARTED == adev->curr_state) {
		if (!ACPI_SUCCESS(accel_aml_start())) {
			dbg_print("stop accel failed\n");
			return -EFAULT;
		}
	}

	arg = s_three[0];
	if (!ACPI_SUCCESS(accel_aml_set_sense(arg))) {
		dbg_print("set sensitivity failed\n");
		return -EFAULT;
	}
	arg = s_three[1];
	if (!ACPI_SUCCESS(accel_aml_set_g_select(arg))) {
		dbg_print("set g-select failed\n");
		return -EFAULT;
	}
	arg = s_three[2];
	if (!ACPI_SUCCESS(accel_aml_set_g_activity_thresh(arg))) {
		dbg_print("set g-activity failed\n");
		return -EFAULT;
	}
	arg = s_three[3];
	if (!ACPI_SUCCESS(accel_aml_set_g_freefall_thresh(arg))) {
		dbg_print("set g-freefall failed\n");
		return -EFAULT;
	}

	dbg_print("<-----\n");
	return 0;
}

/**
 * Call KB method to enable/disable keyboard.
 *
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_kb_control(int flag)
{
	struct acpi_object_list params;
	union acpi_object in_obj[4];

	dbg_print("-----> set keyboard status: %d\n", flag);
	params.count = 4;
	params.pointer = &in_obj[0];
	in_obj[0].integer.value = flag;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = 0;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[2].integer.value = 0;
	in_obj[2].type = ACPI_TYPE_INTEGER;
	in_obj[3].integer.value = 0;
	in_obj[3].type = ACPI_TYPE_INTEGER;
	dbg_print("<-----\n");

	return acpi_evaluate_object(adev->device->handle, ACCEL_KB_METHOD,
		&params, NULL);
}

/**
 * Call AML method to start accelerometer.
 *
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_start(void)
{
	struct acpi_object_list params;
	union acpi_object in_obj[4];

	params.count = 4;
	params.pointer = &in_obj[0];
	in_obj[0].integer.value = ACCEL_AML_START;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = 0;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[2].integer.value = 0;
	in_obj[2].type = ACPI_TYPE_INTEGER;
	in_obj[3].integer.value = 0;
	in_obj[3].type = ACPI_TYPE_INTEGER;

	return acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, NULL);
}


/* Call AML method to stop accelerometer.
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_stop(void)
{
	struct acpi_object_list params;
	union acpi_object in_obj[4];

	params.count = 4;
	params.pointer = &in_obj[0];
	in_obj[0].integer.value = ACCEL_AML_STOP;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = 0;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[2].integer.value = 0;
	in_obj[2].type = ACPI_TYPE_INTEGER;
	in_obj[3].integer.value = 0;
	in_obj[3].type = ACPI_TYPE_INTEGER;

	return acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, NULL);
}


/**
 * Call AML method to set accelerometer sensitivity.
 *
 * @param sense	sensitivity wanted
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_set_sense(int sense)
{
	struct acpi_object_list params;
	union acpi_object in_objs[4];

	params.count = 4;
	params.pointer = &in_objs[0];
	in_objs[0].integer.value = ACCEL_AML_SET_SENSE;
	in_objs[0].type = ACPI_TYPE_INTEGER;
	in_objs[1].integer.value = sense;
	in_objs[1].type = ACPI_TYPE_INTEGER;
	in_objs[2].integer.value = 0;
	in_objs[2].type = ACPI_TYPE_INTEGER;
	in_objs[3].integer.value = 0;
	in_objs[3].type = ACPI_TYPE_INTEGER;

	return acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, NULL);
}


/**
 * Call AML method to read acceleration data.
 *
 * @param data	a pointer to acceleration data
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_read_data(struct accel_raw_data *data)
{
	struct acpi_object_list params;
	union acpi_object in_obj[4];
	struct acpi_buffer result = {ACPI_ALLOCATE_BUFFER, NULL};
	union acpi_object* out_obj;
	acpi_status status;
	
	dbg_print("----->%s() line %d\n", __FUNCTION__, __LINE__);
	params.count = 4;
	params.pointer = &in_obj[0];
	in_obj[0].integer.value = ACCEL_AML_READ_DATA;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = 0;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[2].integer.value = 0;
	in_obj[2].type = ACPI_TYPE_INTEGER;
	in_obj[3].integer.value = 0;
	in_obj[3].type = ACPI_TYPE_INTEGER;
	
	status = acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, &result);
		
	if (ACPI_SUCCESS(status)) {
		out_obj = (union acpi_object *)result.pointer;
		*data = *((struct accel_raw_data *)(out_obj->buffer.pointer));
		dbg_print("accel_raw_data is {%d, %d, %d}\n", data->accel_raw_x,
			data->accel_raw_y, data->accel_raw_z);
	} else {
		dbg_print("acpi_evaluate_object status error: %s\n",
			acpi_format_exception(status));
	}
	kfree(result.pointer);

	return status;
}


/**
 * Call AML method to set g-select.
 *
 * @param select	g-select wanted
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_set_g_select(int select)
{
	struct acpi_object_list params;
	union acpi_object in_objs[4];

	params.count = 4;
	params.pointer = &in_objs[0];
	in_objs[0].integer.value = ACCEL_AML_SET_G_SELECT;
	in_objs[0].type = ACPI_TYPE_INTEGER;
	in_objs[1].integer.value = select;
	in_objs[1].type = ACPI_TYPE_INTEGER;
	in_objs[2].type = ACPI_TYPE_INTEGER;
	in_objs[2].integer.value = 0;
	in_objs[3].type = ACPI_TYPE_INTEGER;
	in_objs[3].integer.value = 0;

	return acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, NULL);
}


/**
 * Call AML method to read g-select.
 *
 * @param select	a pointer to g-select
 * @return	ACPI_SUCCESS if success; other values if error
 */
acpi_status accel_aml_get_g_select(int *select)
{
	struct acpi_object_list params;
	union acpi_object in_obj[4];
	struct acpi_buffer result;
	union acpi_object out_obj;
	acpi_status status;
	
	params.count = 4;
	params.pointer = &in_obj[0];
	in_obj[0].integer.value = ACCEL_AML_GET_G_SELECT;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = 0;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[2].integer.value = 0;
	in_obj[2].type = ACPI_TYPE_INTEGER;
	in_obj[3].integer.value = 0;
	in_obj[3].type = ACPI_TYPE_INTEGER;

	result.length = sizeof(out_obj);
	result.pointer = &out_obj;

	status = acpi_evaluate_object(adev->device->handle, ACCEL_AML_METHOD,
		&params, &result);
	if (ACPI_SUCCESS(status)) {
		if (ACPI_TYPE_INTEGER == out_obj.type) {
			*select = out_obj.integer.value;
			dbg_print("g-select is %d\n", *select);
		} else {
			dbg_print("acpi_evaluate_object return type error for g-select\n");
			status = AE_TYPE;
		}
	}

	return status;
}


acpi_status accel_aml_set_g_offset(struct accel_raw_data *data)
{
	struct acpi_object_list params;
	union acpi_object in_obj[4];
	acpi_status status;
	
	params.count = 4;
	params.pointer = in_obj;
	
	in_obj[0].integer.value = ACCEL_AML_SET_G_OFFSET;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = data->accel_raw_x;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[2].integer.value = data->accel_raw_y;
	in_obj[2].type = ACPI_TYPE_INTEGER;
	in_obj[3].integer.value = data->accel_raw_z;
	in_obj[3].type = ACPI_TYPE_INTEGER;

	
	status = acpi_evaluate_object(adev->device->handle, "ACMD",
		&params, NULL);
		
	return status;
}


static const struct acpi_device_id adevice_ids[] = {
	{"ACLD0000", 0},
	{"", 0},
};

MODULE_DEVICE_TABLE(acpi, adevice_ids);

static SIMPLE_DEV_PM_OPS(accel_pm, accel_suspend, accel_resume);

static struct acpi_driver accel_drv = {
	.owner = THIS_MODULE,
	.name  = ACCEL_NAME,
	.class = CMPC_CLASS_NAME,
	.ids   = adevice_ids,
	.ops   = {
		.add     = accel_add,
		.remove  = accel_remove,
#if 0
		.start   = accel_start,
		.suspend = accel_suspend,
		.resume  = accel_resume,
#endif
	},
	.drv.pm = &accel_pm,
};


static int __init accel_init(void)
{
	int ret;

	dbg_print("----->\n");
	
	ret = acpi_bus_register_driver(&accel_drv);
	if (ret) {
		dbg_print("acpi_bus_register_driver failed\n");
		return ret;
	}


	dbg_print("<-----\n");
	return 0;
}


static void __exit accel_exit(void)
{
	dbg_print("----->\n");
	acpi_bus_unregister_driver(&accel_drv);
	dbg_print("<-----\n");
}


module_init(accel_init);
module_exit(accel_exit);

ACPI_MODULE_NAME("acld");
MODULE_DESCRIPTION("CMPC Gsensor Lid ACPI Driver");
MODULE_VERSION("1.0.0.0");
MODULE_LICENSE("GPL");
