/*
* Intel Sleep Counter Management driver.
* Copyright (c) 2012, Intel Corporation.
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
#include <linux/proc_fs.h>
#include <acpi/acpi_drivers.h>

#define dbg_print(format, arg...) \
        printk(KERN_DEBUG "%s " format, __func__, ##arg)

#define INTEL_CLASS_NAME "Intel"
#define ISCM_NAME "Intel Sleep Counter Management"

/**
 * ISCM AML method name
 */
#define ISCM_AML_METHOD   "SCBC"

/**
 * ISCM proc file name
 */
#define PROC_ISCM "iscm"

/* struct definitions */
/**
 * Sleep Count Management device id
 */
static const struct acpi_device_id iscm_device_ids[] = {
        {"ISCM100", 0},
        {"", 0},
};

MODULE_DEVICE_TABLE(acpi, iscm_device_ids);

/**
 * a pointer to ISCM device
 */
static struct acpi_device *iscm_device = NULL;

/*To store string length of sleepcount*/
static UINT32 iscm_sc_len = 0;
/*Value to compare with proc read bytes count*/
static UINT32 iscm_sc_count = 0;

/*ISCM AML method*/
acpi_status iscm_aml_read_data(UINT8 *iscm_raw_data)
{
        struct acpi_buffer result = {0, NULL};
        union acpi_object * out_obj;
        UINT8 * data_buf;

        acpi_status status;
        status = acpi_evaluate_object(iscm_device->handle, ISCM_AML_METHOD,
                NULL, &result);

        switch (status) {
        case AE_BUFFER_OVERFLOW:
                result.pointer = kmalloc(result.length, GFP_KERNEL);
                if (!result.pointer) {
                        dbg_print("no enough memory\n");
                        return AE_NO_MEMORY;
                }

                status = acpi_evaluate_object(iscm_device->handle,
                        ISCM_AML_METHOD, NULL, &result);

                if (ACPI_SUCCESS(status)) {
                        out_obj = (union acpi_object *)result.pointer;
                        if (ACPI_TYPE_BUFFER == out_obj->type) {
                                data_buf = (UINT8 *)(out_obj->buffer.pointer);
//                           dbg_print("iscm_sleep_count_data length: %d.\n", 
//                                     out_obj->buffer.length);
//                           dbg_print("iscm_sleep_count_data is {%0x%0x%0x}\n", 
//                                         data_buf[0], data_buf[1], data_buf[2]);
                                iscm_raw_data[0] = data_buf[0];
                                iscm_raw_data[1] = data_buf[1];
                                iscm_raw_data[2] = data_buf[2];
                        } else {
                                dbg_print("acpi_evaluate_object return type error for ISCM\n");
                                status = AE_TYPE;
                        }
                } else {
                        dbg_print("acpi_evaluate_object status error: %s\n",
                                acpi_format_exception(status));
                }

                kfree(result.pointer);
                break;

        default:
                dbg_print("acpi_evaluate_object status error: %s\n",
                        acpi_format_exception(status));
        }

        return status;
}

static UINT32 iscm_get_sleepcount(void)
{
	    UINT32 sleep_count = 0;
        UINT8  iscm_data[3];

        if (!ACPI_SUCCESS(iscm_aml_read_data(iscm_data))) {
                dbg_print("iscm_aml_read_data failed\n");
                return -EFAULT;
        }
        sleep_count |= iscm_data[2];
        sleep_count <<= 8;
        sleep_count |= iscm_data[1];
        sleep_count <<= 8;
        sleep_count |= iscm_data[0];
//		dbg_print("ISCM Module returns sleep_count: %u!\n", sleep_count);
	    return sleep_count;
}

/*Caculate the length of a digit*/
static UINT32 iscm_get_digit_length(UINT32 digit)
{
    	UINT32 length = 0;
		
        if (digit < 0) {
                return -1;
    	}
    	do {
                length ++;
                digit = digit / 10;
    	} while(digit);
    	return length;
}

/* When proc_create is called, in its read opration of file_operations, 
 * if read function return value is not 0, it will continue read until 
 * its return value is 0, so compare the read count of bytes with the 
 * string length of sleep count to decide whether the return value is 0,
 * to avoid read many times. 
 */
static UINT32 iscm_proc_read_bytes(size_t count)
{
        UINT32 read_bytes = count;
    	if (read_bytes > iscm_sc_count) {
                read_bytes = iscm_sc_count;
        }
        iscm_sc_count -= read_bytes;
    	if (read_bytes == 0) {
    	    	iscm_sc_count = iscm_sc_len;
    	}
    	return read_bytes;
}

/* proc file ops */
ssize_t proc_read_sleepcount(struct file *filp, char __user *buf, size_t count, loff_t *offp )
{
    	UINT32 sleep_count = 0;
    	UINT32 ret = 0;

    	sleep_count = iscm_get_sleepcount();
    	sprintf(buf,"%d\n", sleep_count);
    	ret = iscm_proc_read_bytes(count);
    	return ret;
}

static const struct file_operations proc_file_fops = {
 .owner = THIS_MODULE,
 .read  = proc_read_sleepcount,
};


/* acpi driver ops */

static int iscm_add(struct acpi_device *device)
{
        iscm_device = device;
    	UINT32 sleep_count = 0;
		
        strcpy(acpi_device_name(device), ISCM_NAME);
        sprintf(acpi_device_class(device), "%s/%s", INTEL_CLASS_NAME,
                ISCM_NAME);
    	sleep_count = iscm_get_sleepcount();
    	iscm_sc_len = iscm_get_digit_length(sleep_count);
    	iscm_sc_count = iscm_sc_len;
        if(NULL == proc_create_data(PROC_ISCM, S_IRUGO, NULL, &proc_file_fops,NULL))
        {
                dbg_print("ISCM can not creat proc entry!\n");
                return 1;
        }
        dbg_print("iscm dev added...\n");
        return 0;
}

static int iscm_remove(struct acpi_device *device, int type)
{
        iscm_device = NULL;
	remove_proc_entry(PROC_ISCM, NULL);
        dbg_print("iscm dev removed...\n");
        return 0;
}
/**
 * Sleep Count Management driver struct
 */
static struct acpi_driver iscm_drv = {
        .owner = THIS_MODULE,
        .name  = ISCM_NAME,
        .class = INTEL_CLASS_NAME,
        .ids   = iscm_device_ids,
        .ops   = {
                .add     = iscm_add,
                .remove  = iscm_remove,
        },
};

static int __init init_iscm(void)
{
        /* register bus driver */
        if (acpi_bus_register_driver(&iscm_drv) < 0) {
                dbg_print("acpi_bus_register_driver failed\n");
                return ENODEV;
        }
        dbg_print("ISCM Module loaded!\n");
        return 0;
}

static void __exit cleanup_iscm(void)
{
        acpi_bus_unregister_driver(&iscm_drv);
        dbg_print("ISCM Module unloaded!\n");
        return;
}

module_init(init_iscm);
module_exit(cleanup_iscm);

MODULE_AUTHOR("James Lu (james.lu@intel.com)");
MODULE_DESCRIPTION("Intel Sleep Counter Management Driver");
MODULE_VERSION("0.9.0.0");
MODULE_LICENSE("GPL");
