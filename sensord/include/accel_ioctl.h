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


#ifndef ACCEL_IOCTL_H
#define ACCEL_IOCTL_H

#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "globaldef.h"

#define ACCEL_DEV_PATH		"/proc/acpi/accel"

/**
 * Acceleration data of x, y and z axes used in driver.
 */
struct accel_raw_data {
	short accel_raw_x;
	short accel_raw_y;
	short accel_raw_z;
};


/* open accel device */
int accel_opendev();

/* accel start */
int accel_start(int fd);

/* accel stop */
int accel_stop(int fd);

/* accel set sense */
int accel_set_sense(int fd, int sense);

/* accel set g_select */
int accel_set_g_select(int fd, int select);

/* accel get g_select */
int accel_get_g_select(int fd, int *select);

/* accel read */
int accel_read(int fd, struct accel_raw_data *raw);


/**
 * To define IOCTL
 */
#define ACCEL_MAGIC_NUMBER 0xFB

/**
 * IOCTL to set sensitivity. The sensitivity should be an int number from
 * 1 to 127.
 */
#define IOCTL_ACCEL_SET_SENSE \
	_IOW(ACCEL_MAGIC_NUMBER, 0x01, int)
/**
 * IOCTL to start accelerometer.
 */
#define IOCTL_ACCEL_START \
	_IO(ACCEL_MAGIC_NUMBER, 0x02)
/**
 * IOCTL to stop accelerometer.
 */
#define IOCTL_ACCEL_STOP \
	_IO(ACCEL_MAGIC_NUMBER, 0x03)
/**
 * IOCTL to set g-select. The g-select should be 0 or 1.
 */
#define IOCTL_ACCEL_SET_G_SELECT \
	_IOW(ACCEL_MAGIC_NUMBER, 0x04, int)
/**
 * IOCTL to get g-select. The g-select should be 0 or 1.
 */
#define IOCTL_ACCEL_GET_G_SELECT \
	_IOR(ACCEL_MAGIC_NUMBER, 0x05, int)

#endif
