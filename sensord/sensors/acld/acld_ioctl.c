/*
 * sensord for intel CMPC
 * Copyright (c) 2015, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 */


/*----------------------------------------------------
 *
 * Filename:	acld_ioctl.c
 * Description:	aclderometer lid sensor driver api.
 * Create By:	kim zhu
 * Create Data:	2015-04-16
 *
----------------------------------------------------*/

#include "accel_ioctl.h"

/*
 * Function Name :	acld_opendev
 * Description :	open acld device 
 * Parameters :		none
 * Return :		int (device fd)
 */
int acld_opendev()
{
	int dev_id;

	dev_id = open(ACLD_DEV_PATH, O_RDWR | O_NONBLOCK | O_SYNC);
	if (-1 == dev_id) {
		return RET_ERR;
	} else {
		return dev_id;
	}
}

/*
 * Function Name :	acld_start
 * Description :	acld device start
 * Parameters :		int (device fd)
 * Return :		int (status)
 */
int acld_start(int fd)
{
	int ret = 0;

	ret = ioctl(fd, IOCTL_ACCEL_START);
	if (ret == 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

/*
 * Function Name :	acld_stop
 * Description :	acld device stop
 * Parameters :		int (device fd)
 * Return :		int (status)
 */
int acld_stop(int fd)
{
	int ret = 0;

	ret = ioctl(fd, IOCTL_ACCEL_STOP);
	if (ret == 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

/*
 * Function Name :	acld_set_sense
 * Description :	acld device set sense
 * Parameters :		int (device fd), int (sense)
 * Return :		int (status)
 */
int acld_set_sense(int fd, int sense)
{
	int ret = 0;

	ret = ioctl(fd, IOCTL_ACCEL_SET_SENSE, sense);
	if (ret == 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

/*
 * Function Name :	acld_set_g_select
 * Description :	acld device set g_select
 * Parameters :		int (device fd), int (g_select)
 * Return :		int (status)
 */
int acld_set_g_select(int fd, int select)
{
	int ret = 0;

	ret = ioctl(fd, IOCTL_ACCEL_SET_G_SELECT, select);
	if (ret == 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

/*
 * Function Name :	acld_get_g_select
 * Description :	acld device get g_select
 * Parameters :		int (device fd), int (pointer of g_select)
 * Return :		int (status)
 */
int acld_get_g_select(int fd, int *select)
{
	int ret = 0;

	ret = ioctl(fd, IOCTL_ACCEL_GET_G_SELECT, select);
	if (ret == 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}

}

/*
 * Function Name :	acld_read
 * Description :	read data from acld device 
 * Parameters :		int (device fd), struct accel_raw_data *(raw data)
 * Return :		int (status)
 */
int acld_read(int fd, struct accel_raw_data *raw)
{
	int ret = 0;

	ret = read(fd, (void *)raw, sizeof(struct accel_raw_data));

	if (ret > 0) {
		return RET_OK;
	} else if (ret == 0) {
		return RET_NODATA;
	} else {
		return RET_ERR;
	}
}

/*
 * Function Name :	acld_setkb
 * Description :	enable or disable keyboard
 * Parameters :		int (device fd), int (flag)
 * Return :		int (status)
 */
int acld_setkb(int fd, int flag)
{
	int ret = 0;

	ret = ioctl(fd, IOCTL_ACCEL_KBSET, flag);
	if (ret == 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}
