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
 * Filename:	accel_ioctl.c
 * Description:	accelerometer sensor driver api.
 * Create By:	kim zhu
 * Create Data:	2015-03-16
 *
----------------------------------------------------*/

#include "accel_ioctl.h"

/*
 * Function Name :	accel_opendev
 * Description :	open accel device 
 * Parameters :		none
 * Return :		int (device fd)
 */
int accel_opendev()
{
	int dev_id;

	dev_id = open(ACCEL_DEV_PATH, O_RDWR | O_NONBLOCK | O_SYNC);
	if (-1 == dev_id) {
		return RET_ERR;
	} else {
		return dev_id;
	}
}

/*
 * Function Name :	accel_start
 * Description :	accel device start
 * Parameters :		int (device fd)
 * Return :		int (status)
 */
int accel_start(int fd)
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
 * Function Name :	accel_stop
 * Description :	accel device stop
 * Parameters :		int (device fd)
 * Return :		int (status)
 */
int accel_stop(int fd)
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
 * Function Name :	accel_set_sense
 * Description :	accel device set sense
 * Parameters :		int (device fd), int (sense)
 * Return :		int (status)
 */
int accel_set_sense(int fd, int sense)
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
 * Function Name :	accel_set_g_select
 * Description :	accel device set g_select
 * Parameters :		int (device fd), int (g_select)
 * Return :		int (status)
 */
int accel_set_g_select(int fd, int select)
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
 * Function Name :	accel_get_g_select
 * Description :	accel device get g_select
 * Parameters :		int (device fd), int (pointer of g_select)
 * Return :		int (status)
 */
int accel_get_g_select(int fd, int *select)
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
 * Function Name :	accel_read
 * Description :	read data from accel device 
 * Parameters :		int (device fd), struct accel_raw_data *(raw data)
 * Return :		int (status)
 */
int accel_read(int fd, struct accel_raw_data *raw)
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

