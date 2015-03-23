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
 * Filename:	tablet_ioctl.c
 * Description:	tablet sensor driver api.
 * Create By:	kim zhu
 * Create Data:	2015-03-16
 *
----------------------------------------------------*/

#include "tablet_ioctl.h"

/*
 * Function Name :	tablet_opendev
 * Description :	open tablet device
 * Parameters :		none
 * Return :		int(device fd)
 */
int tablet_opendev()
{
	int dev_id;

	dev_id = open(TABLET_DEV_PATH, O_RDWR | O_NONBLOCK | O_SYNC);
	if (-1 == dev_id) {
		return RET_ERR;
	} else {
		return dev_id;
	}
}


/*
 * Function Name :	tablet_read
 * Description :	read data from tablet device
 * Parameters :		int(device fd), int *(pointer tablet mode value)
 * Return :		int(status)
 */
int tablet_read(int fd, int *mode)
{
	int ret = 0;

	ret = read(fd, mode, sizeof(int));
	if (ret >= 0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

