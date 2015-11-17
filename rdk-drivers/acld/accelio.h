/*
 * Accelrometer driver IO Interface
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

#ifndef ACCELIO_H
#define ACCELIO_H

/**
 * To define IOCTL
 */
#define ACCEL_MAGIC_NUMBER 0xFB

/**
 * Acceleration data of x, y and z axes used in driver.
 */
#if 0
struct accel_raw_data {
	unsigned char accel_raw_x;
	unsigned char accel_raw_xs;
	unsigned char accel_raw_y;
	unsigned char accel_raw_ys;
	unsigned char accel_raw_z;
	unsigned char accel_raw_zs;
};
#else
struct accel_raw_data {
	short accel_raw_x;
	short accel_raw_y;
	short accel_raw_z;
};
#endif

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
/**
 * IOCTL to set g-offset.
*/
#define IOCTL_ACCEL_SET_G_OFFSET \
	_IOW(ACCEL_MAGIC_NUMBER, 0x06,int)
/**
* IOCTL to set g-activity-thresh. The g-select should be 0 or 1.
*/
#define IOCTL_ACCEL_SET_G_ACTIVITY_THRESH \
	_IOW(ACCEL_MAGIC_NUMBER, 0x07,int)
/**
 * IOCTL to set g-freefall.The g-freefall should be 1-255.
*/
#define IOCTL_ACCEL_SET_G_FREEFALL_THRESH \
	_IOW(ACCEL_MAGIC_NUMBER, 0x08,int)
/**
 * IOCTL to get g-freefall.
*/		
#define IOCTL_ACCEL_GET_G_FREEFALL_THRESH \
	_IOR(ACCEL_MAGIC_NUMBER, 0x09,int)
/**
 * IOCTL to set suspend.
*/
#define IOCTL_ACCEL_SUSPEND \
	_IOR(ACCEL_MAGIC_NUMBER, 0xa,int)
/**
 * IOCTL to set resume.
*/
#define IOCTL_ACCEL_RESUME \
	_IOR(ACCEL_MAGIC_NUMBER, 0xb,int)
/**
 * IOCTL to set keyboard status
*/
#define IOCTL_ACCEL_KBCONTROL \
	_IOW(ACCEL_MAGIC_NUMBER, 0xc,int)

#endif /* ACCELIO_H */
