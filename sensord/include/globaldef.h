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


#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#define RET_OK		0x0
#define RET_ERR		(-1)
#define RET_NODATA	(-2)

/* accel g-select value */
#if 0
#define ACCEL_G_SELECT_1P5G 0
#define ACCEL_G_SELECT_6G 1
#define ACCEL_G_SELECT_2G 0
#define ACCEL_G_SELECT_4G 1
#define ACCEL_G_SELECT_8G 2
#define ACCEL_G_SELECT_16G 3
#else
#define ACCEL_G_SELECT_DEFAULT		3
#define ACCEL_G_SELECT_5G		5
#define ACCEL_G_SELECT_8G		8
#define ACCEL_G_SELECT_12G		12
#endif

#define ACCEL_SAMPLE_INTERVAL		500
#define TABLET_SAMPLE_INTERVAL		300

#define ORIENTATION_SHRESHOLD		2
#define GRAVITY_MEASUREMENT_1P5G	62

#define GRAVITY_DIV_SQRT2_1P5G		(GRAVITY_MEASUREMENT_1P5G / 1.4)
#define SQR_GRAVITY_DIV2_1P5G		(GRAVITY_MEASUREMENT_1P5G * GRAVITY_MEASUREMENT_1P5G / 2)

#define GRAVITY_MEASUREMENT_6G		256
#define GRAVITY_DIV_SQRT2_6G		(int)(GRAVITY_MEASUREMENT_6G / 1.4)
#define SQR_GRAVITY_DIV2_6G		(GRAVITY_MEASUREMENT_6G * GRAVITY_MEASUREMENT_6G / 2)

typedef enum E_ACCEL_ORIENTATION {
	ORI_NORMAL,
	ORI_D90,
	ORI_D180,
	ORI_D270,
	ORI_KEEP,
} AccelOrientation;

typedef enum E_TABLET_MODE {
	CLAMSHELL_MODE,
	TABLET_MODE,
} TabletMode;

#endif
