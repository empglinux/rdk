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


#ifndef __DBUSDEF_H
#define __DBUSDEF_H

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>


#define SENSORD_DBUS_SYNC_OBJ				"/com/intel/empglinux/dbus/sync"
#define SENSORD_DBUS_SYNC_INTERFACE			"com.intel.empglinux.dbus.sync"
#define SENSORD_DBUS_SYNC_ROTATION_SIGNAL		"rotation_now"


#endif
