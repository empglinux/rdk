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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "globaldef.h"
#include "dbusdef.h"
#include "accel_ioctl.h"
#include "tablet_ioctl.h"

#define LOCKFILE	"/var/lock/sensord"

static int accel_dev = -1;
static int tablet_dev = -1;
static int tablet_mode = CLAMSHELL_MODE;
static int t_mode;
static int old_orientation = ORI_NORMAL;

DBusConnection *dbus;

static pthread_t accel_thread;
static pthread_t tablet_thread;

static struct accel_raw_data accel_data;


static void sd_send_dbus_signal(int *dir)
{
	DBusMessage *mess;

	mess = dbus_message_new_signal(SENSORD_DBUS_SYNC_OBJ, SENSORD_DBUS_SYNC_INTERFACE, SENSORD_DBUS_SYNC_ROTATION_SIGNAL);
	dbus_message_append_args(mess, DBUS_TYPE_INT32, dir, DBUS_TYPE_INVALID);
	dbus_connection_send(dbus, mess, NULL);
	dbus_message_unref(mess);
}

int getOrientation(struct accel_raw_data *raw)
{
	short x,y,z;

	x = raw->accel_raw_x;
	y = raw->accel_raw_y;
	z = raw->accel_raw_z;

	if (z <= GRAVITY_DIV_SQRT2_6G && (x*x + y*y) >= SQR_GRAVITY_DIV2_6G) {
		if (abs(x) >= abs(y) * ORIENTATION_SHRESHOLD) {
			if (x > 0) {
				return ORI_D90;
			} else {
				return ORI_D270;
			}
		} else if (abs(y) >= abs(x) * ORIENTATION_SHRESHOLD) {
			if (y > 0) {
				return ORI_NORMAL;
			} else {
				return ORI_D180;
			}
		}
	}
	return ORI_KEEP;
}

void handle_accel_data(struct accel_raw_data *data)
{
	int ori, dbus_sig;
	int ret;

	ori = getOrientation(data);
	syslog(LOG_INFO, "orientation is %d, old_orientation is %d, tablet mode is %d.\n", 
			ori, old_orientation, tablet_mode);

	if (ori == old_orientation || tablet_mode == CLAMSHELL_MODE) {
		return;
	} else {
		old_orientation = ori;

		switch (ori) {
		case ORI_NORMAL:
			ret = system("xrandr -o 2");
			dbus_sig = ORI_D180;
			sd_send_dbus_signal(&dbus_sig);
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		case ORI_D90:
			ret = system("xrandr -o 1");
			dbus_sig = ORI_D270;
			sd_send_dbus_signal(&dbus_sig);
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		case ORI_D180:
			ret = system("xrandr -o 0");
			dbus_sig = ORI_NORMAL;
			sd_send_dbus_signal(&dbus_sig);
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		case ORI_D270:
			ret = system("xrandr -o 3");
			dbus_sig = ORI_D90;
			sd_send_dbus_signal(&dbus_sig);
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		default:
			syslog(LOG_INFO, "default, orientation is %d.\n", ori);
			break;
		}
	}	
	sleep(1);
}

void handle_tablet_data(int mode)
{
	int ret;
	int dbus_sig;

	if (1 == mode && tablet_mode == TABLET_MODE) {
		tablet_mode = CLAMSHELL_MODE;
		ret = system("xrandr -o 0");
		dbus_sig = ORI_NORMAL;
		sd_send_dbus_signal(&dbus_sig);
		syslog(LOG_INFO, "system ret is %d, tablet mode is %d\n.", ret, mode);
	} else if (0 == mode && tablet_mode != TABLET_MODE) {
		tablet_mode = TABLET_MODE;
		ret = system("xrandr -o 2");
		dbus_sig = ORI_D180;
		sd_send_dbus_signal(&dbus_sig);
		syslog(LOG_INFO, "system ret is %d, tablet mode is %d\n.", ret, mode);
	}
	sleep(1);
}

void *accel_handler(void *arg)
{
	int ret;

	while (1) {
		ret = accel_read(accel_dev, &accel_data);
		if (RET_ERR == ret) {
			syslog(LOG_ERR, "accel read data failed.\n");
		}	
		handle_accel_data(&accel_data);
		usleep(ACCEL_SAMPLE_INTERVAL * 1000);
	}
}

void * tablet_handler(void *arg)
{
	int ret;

	while (1) {
		ret = tablet_read(tablet_dev, &t_mode);
		if (RET_ERR == ret) {
			syslog(LOG_ERR, "tablet read data failed.\n");
		}	
		handle_tablet_data(t_mode);
		usleep(TABLET_SAMPLE_INTERVAL * 1000);
	}
}

static void init_accel(int fd)
{
	accel_stop(fd);

	if (accel_start(fd) == RET_OK) {
		syslog(LOG_INFO, "start accel device successfully.\n");
	} else {
		syslog(LOG_ERR, "start accel device failed.\n");
	}

	accel_set_g_select(fd, ACCEL_G_SELECT_1P5G);		
}

void sig_handler(int sig)
{
	int ret;
	char cmd[80];

	strcpy(cmd, "rm -f ");
	strcat(cmd, LOCKFILE);

	if ((SIGHUP == sig) || (SIGINT == sig) || (SIGQUIT == sig) ||
		(SIGILL == sig) || (SIGABRT == sig) || (SIGSEGV == sig) ||
		(SIGTERM == sig) || (SIGTSTP == sig) || (SIGPIPE == sig)) {
		printf("the process is terminated and delete pipe file.\n");
		ret = system(cmd);
		if (ret < 0) {
			printf("system call failed.\n");
		}
		if (accel_dev != -1) close(accel_dev);
		if (tablet_dev != -1) close(tablet_dev);
		accel_dev = -1;
		tablet_dev = -1;
		exit(1);
	}
}

static int deal_signal()
{
	if ((SIG_ERR == signal(SIGHUP, sig_handler)) ||
		(SIG_ERR == signal(SIGINT, sig_handler)) ||
		(SIG_ERR == signal(SIGQUIT, sig_handler)) ||
		(SIG_ERR == signal(SIGILL, sig_handler)) ||
		(SIG_ERR == signal(SIGABRT, sig_handler)) ||
		(SIG_ERR == signal(SIGFPE, sig_handler)) ||
		(SIG_ERR == signal(SIGSEGV, sig_handler)) ||
		(SIG_ERR == signal(SIGPIPE, sig_handler)) ||
		(SIG_ERR == signal(SIGALRM, sig_handler)) ||
		(SIG_ERR == signal(SIGTERM, sig_handler)) ||
		(SIG_ERR == signal(SIGUSR1, sig_handler)) ||
		(SIG_ERR == signal(SIGUSR2, sig_handler)) ||
		(SIG_ERR == signal(SIGTSTP, sig_handler)) ||
		(SIG_ERR == signal(SIGTTIN, sig_handler)) ||
		(SIG_ERR == signal(SIGTTOU, sig_handler))) {
		return RET_ERR;
	} else {
		return RET_OK;
	}
}

void sensord_daemon(const char *pn)
{
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	int i, fd0, fd1, fd2;

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		printf("[%s]: can't get file limit.\n", pn);
	}

	umask(0);

	pid = fork();
	if (pid > 0) {
		exit(0);
	} else if (pid < 0) { 
		printf("[%s]: can't fork.\n", pn);
		exit(1);
	}
	
	setsid();
	
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		printf("[%s]: can't not ignore SIGHUP.\n", pn);
	}

	pid = fork();
	if (pid > 0) {
		exit(0);
	} else if (pid < 0) { 
		printf("[%s]: can't fork.\n", pn);
		exit(1);
	}
		
	if (chdir("/") < 0) {
		printf("[%s]: can't change directory to /.\n", pn);
	}

	if (rl.rlim_max == RLIM_INFINITY) {
		rl.rlim_max = 1024;
	}
	for (i = 0; (unsigned int)i < rl.rlim_max; i++) {
		close(i);
	}
		
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	openlog(pn, LOG_PID, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d.\n", fd0, fd1, fd2);
		exit(1);
	}
}

void dbus_init()
{
	DBusError err;

	dbus_error_init(&err);
	dbus = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!dbus) {
		syslog(LOG_ERR, "Failed to connect to D-BUS daemon: %s.\n", err.message);
		dbus_error_free(&err);
	}
	dbus_connection_setup_with_g_main(dbus, NULL);
}

int main(int argc, char* argv[])
{
	int ret;

	if (RET_OK != deal_signal()) {
		printf("Install signal handlers failed.\n");
		return RET_ERR;
	}

	ret = mkfifo(LOCKFILE, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
	if (ret) {
		printf("Sensord lock file exists under /var/lock/, please check if sensord is already started.\n");
		return RET_ERR;
	}

#ifndef DEBUG
	sensord_daemon("Sensord");
#endif

	dbus_init();

	accel_dev = accel_opendev();
	if (accel_dev < 0) {
		syslog(LOG_ERR, "open accelerometer device failed.\n");
		goto err1;
	}
	init_accel(accel_dev);

	tablet_dev = tablet_opendev();
	if (tablet_dev < 0) {
		syslog(LOG_ERR, "open tablet device failed.\n");
		goto err1;
	}

	old_orientation = ORI_NORMAL;
	tablet_mode = CLAMSHELL_MODE;

	/* accelerometer handle thread */
	if (pthread_create(&accel_thread, NULL, accel_handler, NULL))
		goto err1;
	/* tablet handle thread */
	if (pthread_create(&tablet_thread, NULL, tablet_handler, NULL))
		goto err1;
	

	while (1) {
		sleep(3);
	}

err1:
	close(accel_dev);
	close(tablet_dev);
	exit(1);
	return 0;
}
