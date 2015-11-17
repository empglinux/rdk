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
#include <math.h>

#include "globaldef.h"
#include "accel_ioctl.h"
#include "AngleSensor.h"

#define LOCKFILE	"/var/lock/sensord"

static int accel_dev = -1;
static int acld_dev = -1;
static int tablet_mode = CLAMSHELL_MODE;
static int old_orientation = ORI_NORMAL;

static pthread_t accel_thread;
static pthread_t angle_thread;

static struct accel_raw_data accel_data;


int getOrientation(struct accel_raw_data *raw)
{
	short x,y;

	x = (char)raw->accel_raw_x;
	y = (char)raw->accel_raw_y;

#if 0
	if (z <= GRAVITY_DIV_SQRT2_6G && (x*x + y*y) >= SQR_GRAVITY_DIV2_6G) {
		if (abs(x) >= abs(y) * ORIENTATION_SHRESHOLD) {
			if (x > 0) {
				return ORI_D90;
			} else {
				return ORI_D270;
			}
		} else if (abs(y) >= abs(x) * ORIENTATION_SHRESHOLD) {
			if (y > 0) {
				return ORI_D180;
			} else {
				return ORI_NORMAL;
			}
		}
	}
#else
	if (abs(y) >= abs(x) * ORIENTATION_SHRESHOLD) {
		if (y > 0) {
			return ORI_D90;
		} else {
			return ORI_D270;
		}
	} else if (abs(x) >= abs(y) * ORIENTATION_SHRESHOLD) {
		if (x > 0) {
			return ORI_D180;
		} else {
			return ORI_NORMAL;
		}
	}
#endif
	return ORI_KEEP;
}

void handle_accel_data(struct accel_raw_data *data)
{
	int ori;
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
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		case ORI_D90:
			ret = system("xrandr -o 1");
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		case ORI_D180:
			ret = system("xrandr -o 0");
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		case ORI_D270:
			ret = system("xrandr -o 3");
			syslog(LOG_INFO, "system ret is %d, orientation is %d.\n", ret, ori);
			break;
		default:
			syslog(LOG_INFO, "default, orientation is %d.\n", ori);
			break;
		}
	}	
	sleep(1);
}

void handle_angle_data(double angle)
{
	if (angle < (20.0) && tablet_mode == TABLET_MODE) {
		return;
	}

	if (angle < (140.0) && tablet_mode == TABLET_MODE) {
		tablet_mode = CLAMSHELL_MODE;
		system("xrandr -o 0");
		/* enable keyboard */
		if (-1 == ioctl(acld_dev, IOCTL_ACCEL_KBSET, 0)) {
			syslog(LOG_INFO, "Enable keyboard failed.\n");
		} else {
			syslog(LOG_INFO, "Enable keyboard success.\n");
		}
	} else if (angle > 200 && tablet_mode != TABLET_MODE) {
		tablet_mode = TABLET_MODE;
		/* disable keyboard */
		if (-1 == ioctl(acld_dev, IOCTL_ACCEL_KBSET, 1)) {
			syslog(LOG_INFO, "Disable keyboard failed.\n");
		} else {
			syslog(LOG_INFO, "Disable keyboard success.\n");
		}
	}
}

void *angle_handler(void *arg)
{
	int ret;
	double angle;
	struct accel_raw_data dk_data;
	struct accel_raw_data ld_data;
	short lx, ly, lz;
	short dx, dy, dz;

	while (1) {
		ret = accel_read(accel_dev, &dk_data);
		if (RET_ERR == ret) {
			syslog(LOG_ERR, "[angle_handler] accel read data failed.\n");
		}	
		ret = acld_read(acld_dev, &ld_data);
		if (RET_ERR == ret) {
			syslog(LOG_ERR, "[angle_handler] acld read data failed.\n");
		}	

		lx = (char)ld_data.accel_raw_x;	
		ly = (char)ld_data.accel_raw_y;	
		lz = (char)ld_data.accel_raw_z;	
		dx = (char)dk_data.accel_raw_x;
		dy = (char)dk_data.accel_raw_y;
		dz = (char)dk_data.accel_raw_z;

		CAngleSensor as;
		as.SetRawData_Dock(dx, dy, dz);
		as.SetRawData_Lid(lx, ly, lz);
		as.GetAngleDegrees(angle);

		handle_angle_data(angle);
		usleep(TABLET_SAMPLE_INTERVAL * 1000);
	}
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

static void init_accel(int fd)
{
	accel_stop(fd);

	if (accel_start(fd) == RET_OK) {
		syslog(LOG_INFO, "start accel device successfully.\n");
	} else {
		syslog(LOG_ERR, "start accel device failed.\n");
	}

	accel_set_g_select(fd, 8);		
}

static void init_acld(int fd)
{
	acld_stop(fd);

	if (acld_start(fd) == RET_OK) {
		syslog(LOG_INFO, "start acld device successfully.\n");
	} else {
		syslog(LOG_ERR, "start acld device failed.\n");
	}

	acld_set_g_select(fd, 8);		
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
		if (acld_dev != -1) close(acld_dev);
		accel_dev = -1;
		acld_dev = -1;
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

	accel_dev = accel_opendev();
	if (accel_dev < 0) {
		syslog(LOG_ERR, "open accelerometer device failed.\n");
		goto err1;
	}
	init_accel(accel_dev);

	acld_dev = acld_opendev();
	if (acld_dev < 0) {
		syslog(LOG_ERR, "open lid device failed.\n");
		goto err1;
	}
	init_acld(acld_dev);

	old_orientation = ORI_NORMAL;
	tablet_mode = CLAMSHELL_MODE;

	/* accelerometer handle thread */
	if (pthread_create(&accel_thread, NULL, accel_handler, NULL))
		goto err1;

	/* angle handle thread */
	if (pthread_create(&angle_thread, NULL, angle_handler, NULL))
		goto err1;


	while (1) {
		sleep(3);
	}

err1:
	close(accel_dev);
	close(acld_dev);
	exit(1);
	return 0;
}
