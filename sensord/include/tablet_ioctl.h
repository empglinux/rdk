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


#ifndef TABLET_IOCTL_H
#define TABLET_IOCTL_H

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "globaldef.h"

#define TABLET_DEV_PATH		"/proc/acpi/tablet"


/* open tablet device */
int tablet_opendev();

/* tablet read */
int tablet_read(int, int *);

#endif
