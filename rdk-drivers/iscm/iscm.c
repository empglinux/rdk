/*
 * Copyright (c) 2012 Intel Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file iscm.c
 *
 * Intel Sleep Counter Management driver
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/acpi.h>

#define dbg_print(format, arg...) \
        printk(KERN_DEBUG "%s " format, __func__, ##arg)

#define INTEL_CLASS_NAME "Intel"
#define ISCM_NAME "Intel Sleep Counter Management"

#ifndef UINT8
#define UINT8 uint8_t
#endif

#ifndef UINT32
#define UINT32 uint32_t
#endif

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

/* proc file ops */
//static int proc_read_sleepcount(char *page, char **start, off_t off, int count, int *eof, void *data)
static int proc_read_sleepcount(struct file *filp, char *buf, size_t count, loff_t *offp ) 
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
 //       dbg_print("ISCM Module returns sleep_count: %u!\n", sleep_count);
//        return sprintf(page,"%d\n", sleep_count);
        return sprintf(buf, "%d\n", sleep_count); 
}

static const struct file_operations td_proc_sleepcount_fops = {
 .owner = THIS_MODULE,
 .read  = proc_read_sleepcount,
};


/* acpi driver ops */

static int iscm_add(struct acpi_device *device)
{
        iscm_device = device;
        strcpy(acpi_device_name(device), ISCM_NAME);
        sprintf(acpi_device_class(device), "%s/%s", INTEL_CLASS_NAME,
                ISCM_NAME);
        
        //if(NULL == create_proc_read_entry(PROC_ISCM, 0, NULL,  proc_read_sleepcount, NULL))
        if (NULL == proc_create(PROC_ISCM, S_IRUGO|S_IFREG, NULL, &td_proc_sleepcount_fops))
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
		    //remove_proc_entry(PROC_ISCM, NULL);
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
                .remove  = iscm_remove
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
