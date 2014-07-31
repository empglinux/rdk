/*
 * fnkey.cpp for intel CMPC 
 * Copyright (c) 2009, Intel Corporation.
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

#include "button.h"
#include "cmpc_pm.h"
#include "OnScrDsp.h"


/* status: camera exist and camera alive */
#define CAMERA_STATUS(param) WEXITSTATUS  (ret = system(param))

#ifdef DEBUG
#define DEBUG_PRINT(format, arg...) \
	printf("%s " format, __func__, ##arg)
#else
#define DEBUG_PRINT(format, arg...)
#endif

#define SPACE_KEY_CODE     65
#define SHIFT_KEY_CODE     50
#define PAGE_UP_KEY_CODE   112
#define PAGE_DOWN_KEY_CODE 117

#define CTRL_L_KEY_CODE 	37
#define ALT_L_KEY_CODE 		64
#define KEY_CODE_OF_D 		40
#define KEY_CODE_OF_SUPER_L 	133

#define VKBD_CAMERA_KEY_PRESSED 	0x87
#define VKBD_CAMERA_KEY_RELEASED 	0x97
#define VKBD_PAGEUP_FN_PRESSED 		0x88
#define VKBD_PAGEUP_FN_RELEASED 	0x98
#define VKBD_PAGEDOWN_FN_PRESSED 	0x89
#define VKBD_PAGEDOWN_FN_RELEASED 	0x99
#define VKBD_SHOW_DESKTOP_FN_PRESSED	0x85
#define VKBD_SHOW_DESKTOP_FN_RELEASED 	0x95
#ifdef CDV
#define VKBD_KEY_FNF1_PRESS		0x8b
#else
#define VKBD_KEY_FNF1_PRESS		0x81
#endif
#define VKBD_KEY_FNF6_PRESS		0x82
#define VKBD_KEY_FNF7_PRESS		0x83
#define VKBD_KEY_FNF8_PRESS		0x84
#ifdef CDV
#define VKBD_KEY_FNF1_RELEASE		0x9b
#else
#define VKBD_KEY_FNF1_RELEASE		0x91
#endif
#define VKBD_KEY_FNF6_RELEASE		0x92
#define VKBD_KEY_FNF7_RELEASE		0x93
#define VKBD_KEY_FNF8_RELEASE		0x94

#define CMAX_BUF			80
#define CAMERA_CHARMAX			30

#define WIFI_DEV			"/usr/bin/fnkey_debug/wifi.dev"
#define WIFI_STATUS_OPEN		1
#define WIFI_STATUS_CLOSE		0
#define FNF11_DEFAULT_LAUNCH_PROCESS	"cheese"
//#define KBD_DEV				"/dev/input/event0"
#define KBD_DEV				"/dev/input/event13"
#define KBD_HOME_KEY_CODE		0xac

#define MAX_STRING_LEN	256
#define LOCKFILE	"/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

#define WIFI_LED_ON 0x1

#define CMPC_VKD_ACPI_MAGIC		'L'	
#define CMPC_ACPI_GET_LED_STATUS	_IO(CMPC_VKD_ACPI_MAGIC, 0)
#define CMPC_ACPI_SET_LED_ON		_IO(CMPC_VKD_ACPI_MAGIC, 1)
#define CMPC_ACPI_SET_LED_OFF		_IO(CMPC_VKD_ACPI_MAGIC, 2)
#define CMPC_ACPI_SET_LED_F		_IO(CMPC_VKD_ACPI_MAGIC, 3)

Display *xdpy = NULL;
static int screen_num;
static int vkd_dev = -1;
static int pm_dev = -1;
static int event4_dev = -1;
unsigned int display_width; 
unsigned int display_height;
static int wifi_fd = 0;
static int kbd_dev_fd = 0;

static char camera[CAMERA_CHARMAX];
static char m_pDriverPath[ACPI_MAX_STRING] = {'\0'};
static const char *VKD_DEV_PATH = "/proc/acpi/cmpc_vkd";
static const char *PM_DEV_PATH = "/dev/input/event4";
static const char *EVENT4_DEV_PATH = "/dev/input/event13";
//static const char *EVENT4_DEV_PATH = "/dev/input/event4";

static const char *XRANDR_VGA =
// "xrandr --output LVDS1 --off --output VGA1 --auto";
"/bin/bash /usr/bin/cmpcfk-control.sh other";
static const char *XRANDR_LVDS =
"/bin/bash /usr/bin/cmpcfk-control.sh lvds";
static const char *XRANDR_DUAL =
"/bin/bash /usr/bin/cmpcfk-control.sh dual";

void SetWlanDownPressed(void);
void SetDisplayDownPressed(void);
void SetBacklightDownPressed(void);
void SetBacklightDownReleased(void);
void SetBacklightUpPressed(void);
void SetBacklightUpReleased(void);
void led_on(void);
void led_off(void);
unsigned long led_status(void);



#define ACPI_TYPE_DEVICE                0x06	/* Name, multiple Node */
#define ACPI_TYPE_PROCESSOR             0x0C	/* Name,byte_const,Dword_const,byte_const,multi nm_o */
#define ACPI_TYPE_THERMAL               0x0D	/* Name, multiple Node */
#define ACPI_TYPE_POWER                 0x0B	/* Name,byte_const,word_const,multi Node */

typedef unsigned int u32;
#define u8 unsigned char
typedef u32 acpi_integer;
#define ACPI_TYPE_INTEGER               0x01	/* Byte/Word/Dword/Zero/One/Ones */
typedef u32 acpi_object_type;
typedef void *acpi_handle;			/* Actually a ptr to a NS Node */
typedef u32 acpi_io_address;

union acpi_object {
	acpi_object_type type;			/* See definition of acpi_ns_type for values */
	struct {
		acpi_object_type type;
		acpi_integer value;		/* The actual number */
	} integer;

	struct {
		acpi_object_type type;
		u32 length;			/* # of bytes in string, excluding trailing null */
		char *pointer;			/* points to the string value */
	} string;

	struct {
		acpi_object_type type;
		u32 length;			/* # of bytes in buffer */
		u8 *pointer;			/* points to the buffer */
	} buffer;

	struct {
		acpi_object_type type;
		u32 fill1;
		acpi_handle handle;		/* object reference */
	} reference;

	struct {
		acpi_object_type type;
		u32 count;			/* # of elements in package */
		union acpi_object *elements;	/* Pointer to an array of ACPI_OBJECTs */
	} package;



	struct {
		acpi_object_type type;
		u32 proc_id;
		acpi_io_address pblk_address;
		u32 pblk_length;
	} processor;

	struct {
		acpi_object_type type;
		u32 system_level;
		u32 resource_order;
	} power_resource;
};

typedef struct key_event {
	long min;
	int sec;
	unsigned short type;
	unsigned short code;
	int value;
} key_event;


/*
 *
 *  * Blatantly copied from acpi source code in the kernel
 *
 */
u32 acpi_ut_dword_byte_swap(u32 value)
{
	union {
		u32	value;
		u8	bytes[4];
	} out;

	union {
		u32	value;
		u8	bytes[4];
	} in;

	in.value = value;
	out.bytes[0] = in.bytes[3];
	out.bytes[1] = in.bytes[2];
	out.bytes[2] = in.bytes[1];
	out.bytes[3] = in.bytes[0];

	return (out.value);
}


static const char acpi_gbl_hex_to_ascii[] = {'0','1','2','3','4','5','6','7',
	'8','9','A','B','C','D','E','F'};

int select_camera()
{
	int fd, ret;
	char cbuf[CAMERA_CHARMAX];
	char WHICHOF[CMAX_BUF];


	fd = open("/etc/cmpcfk/camera.conf", O_RDONLY);
	if (fd < 0)
	{
		printf("can not open camera.conf.\n");
		goto nocamera_2;
	}
        strcpy(WHICHOF, "which ");
	memset(camera, 0, sizeof(camera));
	memset(cbuf, 0, sizeof(cbuf));
	ret = read(fd, cbuf, sizeof(cbuf));
	if (ret <= 0)
	{
		printf("no camera specified in camera.conf.\n");
		strcpy(camera, "nocamera");
		goto nocamera_1;
	}
	else
	{
		strncat(WHICHOF, cbuf, strlen(cbuf) - 1);
		if (CAMERA_STATUS(WHICHOF))
			strcpy(camera, "nocamera");
		else
			strncpy(camera, cbuf, strlen(cbuf) - 1);
		printf("camera=%s\n",camera);
		close(fd);
		return 0;
	}

nocamera_1:
	close(fd);
nocamera_2:
	memset(camera, 0, sizeof(camera));
	strcpy(camera, FNF11_DEFAULT_LAUNCH_PROCESS);
	return -1;
}

char acpi_ut_hex_to_ascii_char(acpi_integer integer, u32 position) 
{
	return (acpi_gbl_hex_to_ascii[(integer >> position) & 0xF]);
}


void acpi_ex_eisa_id_to_string(u32 numeric_id, char *out_string)
{
	u32	eisa_id;

	/* Swap ID to big-endian to get contiguous bits */
	eisa_id = acpi_ut_dword_byte_swap (numeric_id);

	out_string[0] = (char) ('@' + (((unsigned long) eisa_id >> 26) & 0x1f));
	out_string[1] = (char) ('@' + ((eisa_id >> 21) & 0x1f));
	out_string[2] = (char) ('@' + ((eisa_id >> 16) & 0x1f));
	out_string[3] = acpi_ut_hex_to_ascii_char ((acpi_integer) eisa_id, 12);
	out_string[4] = acpi_ut_hex_to_ascii_char ((acpi_integer) eisa_id, 8);
	out_string[5] = acpi_ut_hex_to_ascii_char ((acpi_integer) eisa_id, 4);
	out_string[6] = acpi_ut_hex_to_ascii_char ((acpi_integer) eisa_id, 0);
	out_string[7] = 0;
}


/*
 *
 *  * evaluate _HID object, only know how to handle integer _HIDs right now
 *
 */
void get_hid(int fd, char *path, char *hid)
{
	cmpc_pm_acpi_t		data;
	union acpi_object	eisa_id;

	memset(hid, 0, 8);
	memset(&data, 0, sizeof(data));

	strcpy(data.pathname, path);

	if (ioctl(fd, CMPC_PM_ACPI_EVALUATE_OBJ, &data))
		return;

	if (data.status != sizeof(eisa_id)) 
	{
		sprintf(hid, "BUG:sz");
		return;
	}

	if ((unsigned long long)read(fd, &eisa_id, data.status) != data.status)
		return;

	if (eisa_id.type != ACPI_TYPE_INTEGER) 
	{
		sprintf(hid, "BUG:typ");
		return;
	}

	acpi_ex_eisa_id_to_string(eisa_id.integer.value, hid);

	return;
}


/*
 *  * Do we consider this path a file or directory?
 */
int is_dev(int fd, char *path)
{
	cmpc_pm_acpi_t data;

	memset(&data, 0, sizeof(data));
	strcpy(data.pathname, path);

	if (ioctl(fd, CMPC_PM_ACPI_GET_TYPE, &data))
		return 0;

	switch (data.status) 
	{
		case ACPI_TYPE_DEVICE:
		case ACPI_TYPE_PROCESSOR:
		case ACPI_TYPE_THERMAL:
		case ACPI_TYPE_POWER:
			return 1;
		default:
			return 0;
	}
}


/*
 * Print out a "directory" and recurse through sub-dirs
 */
void get_path(int fd, char *path, int *entries, int level, char *pRetPath)
{

	cmpc_pm_acpi_t	data;
	int		i, cnt;
	char		*buf, *tmp, *tmp2;

	memset(&data, 0, sizeof(data));

	if (path)
	{
		strcpy(data.pathname, path);
	}

	if (ioctl(fd, CMPC_PM_ACPI_GET_NEXT, &data)) 
	{
		return;
	}

	buf = (char*)malloc(data.status);

	if (!buf) 
	{
		return;
	}

	if ((unsigned long long)read(fd, buf, data.status) != data.status) 
	{
		return;
	}

	tmp = buf;
	for (cnt = 0; (tmp = strchr(tmp, '\n')) != NULL ; cnt++)
		tmp++;

	entries[level] = cnt;

	tmp = buf;

	for (i = 0 ; i < cnt ; i++) 
	{
		unsigned long len;
		char *new_path, *cur_obj, hid[8];

		tmp2 = strchr(tmp, '\n');

		len = (unsigned long)tmp2 - (unsigned long)tmp;
		cur_obj = (char*)malloc(len + 1);

		if (!cur_obj) 
		{
			free(buf);
			return;
		}

		memset(cur_obj, 0, len + 1);
		strncpy(cur_obj, tmp, len);

		tmp = tmp2 + 1;

		if (path)
			len += strlen(path) + 1;

		new_path = (char*)malloc(len);

		if (!new_path) 
		{
			free(buf);
			return;
		}

		memset(new_path, 0, len);

		if (path) 
		{
			strcat(new_path, path);
			strcat(new_path, ".");
		}

		strcat(new_path, cur_obj);

		if (is_dev(fd, new_path))
		{
			get_path(fd, new_path, entries, level + 1, pRetPath);
		}
		else
		{
			if (!strcmp(cur_obj, "_HID") && strstr(new_path, "IPML") )
			{
				get_hid(fd, new_path, hid);
				memset(pRetPath, 0, ACPI_MAX_STRING);
				strcpy( pRetPath , new_path);
				break;
			}
		}

		entries[level]--;
		free(new_path);
		free(cur_obj);
	}
}


int set_display(int num)
{
	int ret;

	switch (num)
	{
		case 0:
			// xrandr lvds
			ret = system(XRANDR_LVDS);
			break;
		case 1:
			// xrandr vga
			ret = system(XRANDR_VGA);
			break;
		case 2:
			// xrandr dual
			ret = system(XRANDR_DUAL);
			break;
		default:
			return -1;
	}
}


int get_brightness(int fd, char* path)
{
	cmpc_pm_acpi_t data;

	memset(&data, 0, sizeof(data));
	strcpy(data.pathname, path);

	if (ioctl(fd, CMPC_PM_ACPI_GET_BRIGHTNESS, &data))
		return 0;

	return  data.status;
}


int set_brightness(int fd, char* path, int value)
{
	cmpc_pm_acpi_t data;

	memset(&data, 0, sizeof(data));
	data.inputparam = value;
	strcpy(data.pathname, path);

	if (ioctl(fd, CMPC_PM_ACPI_SET_BRIGHTNESS, &data))
		return 0;

	return  data.status;
}


/* print a message and return to caller. caller specifies "errnoflag". */
static void err_doit(int errnoflag, const char *fmt, va_list ap)
{
	int errno_save;
	char buf[MAX_STRING_LEN];

	errno_save = errno;  /* value caller might want printed */
	vsprintf(buf, fmt, ap);

	if (errnoflag)
		sprintf(buf+strlen(buf),": %s", strerror(errno_save));

	strcat(buf, "\n");
	fflush(stdout); /* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(NULL); /*flushes and stdio  output streams */

	return;
}


/* Fatal error related to a system call . print a message and terminate. */
void err_quit(const char *fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(0, fmt, ap);
	va_end(ap);
	exit(1);
}


void daemonize(const char *cmd)
{
	int                 i, fd0, fd1, fd2;
	pid_t               pid;
	struct rlimit       rl;
	struct sigaction    sa;

	/* Clear file creation mask. */
	umask(0);

	/* Get maximum number of file descriptors. */
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("%s: can't get file limit", cmd);

	/* Become a session leader to lose controlling TTY. */
	if ((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);

	setsid();

	/* Ensure future opens won't allocate controlling TTYs. */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("%s: can't ignore SIGHUP");
	if ((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if (pid != 0) /* parent */

		exit(0);

	/* Change the current working directory to the root so
	 * we won't prevent file systems from being unmounted.
	 */
	if (chdir("/") < 0)
		err_quit("%s: can't change directory to /");

	/* Close all open file descriptors. */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; (unsigned int)i < rl.rlim_max; i++)
		close(i);

	/* Attach file descriptors 0, 1, and 2 to /dev/null. */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/* Initialize the log file. */
	openlog(cmd, LOG_PID, LOG_DAEMON);

	if (fd0 != 0 || fd1 != 1 || fd2 != 2) 
	{
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
				fd0, fd1, fd2);
		exit(1);
	}
}


#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xrender.h>  


static int vga_connect()
{
	char *display_name = NULL;
	int result;
	int screen = -1;
	XRROutputInfo *output_info = NULL;
	Display *dpy;
	Window root;
	XRRScreenResources *res;

	dpy = xdpy;

	if (dpy == NULL) 
	{
		fprintf (stderr, "Could't connect to X server\n");
		return -1;
	}

	screen = DefaultScreen (dpy);

	root = RootWindow (dpy, screen);
	res = XRRGetScreenResources (dpy, root);
	output_info = XRRGetOutputInfo (dpy, res, res->outputs[0]);

	result = output_info->connection;

	return result;
}


static int open_dev()
{
	vkd_dev = open(VKD_DEV_PATH, O_RDONLY | O_NONBLOCK);

	if(vkd_dev == -1)
	{
		printf("fail open %s \n",VKD_DEV_PATH);
		return -1;
	}
#if 0
	pm_dev = open(PM_DEV_PATH, O_RDONLY);

	if (pm_dev == -1)
	{
		printf("fail open %s \n",PM_DEV_PATH);
		return -1;
	}

	if ( (event4_dev = open(EVENT4_DEV_PATH, O_RDONLY)) == -1)
	{
		printf("Faile to open %s\n", EVENT4_DEV_PATH);
	}
#endif

	return 0;
}


/* merged by He,Yingyuan from Button.c start*/
/*
 * function name : get_active_Xid
 * description: Get the active window id on the desktop.
 *
 */
Window get_active_Xid(Display *xdpy)
{
	Window root;
	Window Active_Xid;
	Atom request;
	int status;
	Atom actual_type_return;
	int actual_format_return;
	unsigned long nitems_return;
	unsigned long bytes_after_return;
	unsigned char *prop = NULL;

	request = XInternAtom(xdpy, "_NET_ACTIVE_WINDOW", False);
	if (request == BadAlloc || request == BadValue)
	{
		return 0;
	}	
	root = XDefaultRootWindow(xdpy);
	status = XGetWindowProperty(xdpy, root, request, 0, (~0L),
			False, AnyPropertyType, &actual_type_return,
			&actual_format_return, &nitems_return, &bytes_after_return,
			&prop);

	if (status == BadWindow || status ==BadAtom || status == BadValue) 
	{
		fprintf(stderr, "window id # 0x%lx does not exists!", root);
		return 0;
	} 

	if (status != Success || actual_type_return == None)
	{
		fprintf(stderr, "XGetWindowProperty failed!");
		return 0;
	}

	Active_Xid = *((Window*)prop);

	Xfree(prop);

	return Active_Xid;
}


/*
 * function name : set_window_focus
 * description: set window focus to window id
 *
 */
int set_window_focus(Display *xdpy, Window Xid) 
{
	int ret = 0;

	ret = XSetInputFocus(xdpy, Xid, RevertToParent, CurrentTime);

	XFlush(xdpy);

	return ret;
}


/*
 *function name : get_child_window
 *description: get the child window from screen 
 *
 */
inline void get_child_window(Display *xdpy,Window root,unsigned int *nchild_return,Window **child)
{
	Window dummy;

	if (!XQueryTree(xdpy, root, &dummy, &dummy, child, nchild_return))
		return;
}


/*
 *function name : match_window_name
 *description : match the window name
 *
 */
inline BOOL match_window_name(Display *xdpy, Window window,char* name)
{
	XWindowAttributes attr;
	XClassHint classhint;

	XGetWindowAttributes(xdpy, window, &attr);

	if(attr.width > display_width-66 || attr.height > display_height-25 || attr.width < 390 || attr.height < 242 || attr.y == 0 )
		return False;

	if (XGetClassHint(xdpy, window, &classhint)) 
	{

		if (!strcmp(classhint.res_name,name)) 
		{
			XFree(classhint.res_name);
			XFree(classhint.res_class);
			return True;
		}	

		XFree(classhint.res_name);
		XFree(classhint.res_class);
	}

	return False;
}


/*
 *function name : search_wid_from_name
 *description: get the window id from the application's name 
 *
 */
Window search_wid_from_name(Display *xdpy, char* name)
{
	Window* child,*subchild;
	int screencount = 0;
	int i,j,k;
	unsigned int nchild = 0,nnchild = 0;
	Window ret_xid = 0;

	screencount = ScreenCount(xdpy);
	DEBUG_PRINT("screencount == [%d]\n",screencount);

	for(i = 0; i < screencount; i++)
	{
		get_child_window(xdpy,RootWindow(xdpy,i),&nchild,&child);

		if(!nchild)
			return 0; /*fail for get the child*/

		for (j = 0; j < nchild; j++)
		{
			/*if children still has children,loop*/
			if(match_window_name(xdpy,child[j],name))
			{
				ret_xid = child[j];
				XFree(child);
				return ret_xid;
			}

			get_child_window(xdpy,child[j],&nnchild,&subchild);

			for(k = 0; k < nnchild; k++)
			{
				if(match_window_name(xdpy,subchild[k],name))
				{
					ret_xid = subchild[k];
					XFree(subchild);
					XFree(child);
					return ret_xid;
				}
			}
		}

	}

	/*free the resource alloc by xlib*/
	XFree(child);
	XFree(subchild);

	return 0; /*match failed*/	
}


/*
 *function name : set_window_active
 *description: change the window state to active
 *
 */
int set_window_active(Display *xdpy,Window Xid)
{
	int ret = 0;
	XEvent xevent;
	XWindowAttributes wattr;

	memset(&xevent, 0, sizeof(xevent));
	xevent.type = ClientMessage;
	xevent.xclient.display = xdpy;
	xevent.xclient.window = Xid;
	xevent.xclient.message_type = XInternAtom(xdpy, "_NET_ACTIVE_WINDOW", False);
	xevent.xclient.format = 32;
	xevent.xclient.data.l[0] = 2L; /* 2 == Message from a window pager */
	xevent.xclient.data.l[1] = CurrentTime;

	XGetWindowAttributes(xdpy, Xid, &wattr);
	ret = XSendEvent(xdpy, wattr.screen->root, False,
			SubstructureNotifyMask | SubstructureRedirectMask,
			&xevent);

	/*flush the action*/
	XFlush(xdpy);

	return ret;
}


/*
 * function name : send_fake_key
 * description: Send the fake key to the directive window.
 *
 */
void send_fake_key(Display *xdpy,Window Xid,int keycode,KEY_STATUS_MASK mask,int modstate)
{
	int is_press = 0;

	XKeyEvent xk;
	xk.send_event = True;
	xk.display = xdpy;
	xk.subwindow = None;
	xk.time = CurrentTime;
	xk.same_screen = True;

	/* Should we set these at all? */
	xk.x = xk.y = xk.x_root = xk.y_root = 1;

	if(KeyPressMask==mask)
		is_press = 1;

	xk.window = Xid;
	xk.keycode = keycode;
	xk.state = modstate;
	xk.type = (is_press ? KeyPress : KeyRelease);

	if(!Xid)
		return;

	if(ShiftMask == modstate)
	{
		XTestFakeKeyEvent(xdpy, SHIFT_KEY_CODE, True, CurrentTime);
	}

	//*********************** For the use of hiding desktop
	if(5 == modstate) 
	{
		XTestFakeKeyEvent(xdpy, CTRL_L_KEY_CODE, True, CurrentTime);
		XTestFakeKeyEvent(xdpy, ALT_L_KEY_CODE, True, CurrentTime);
	}
	//*********************** End

	XTestFakeKeyEvent(xdpy, keycode, True, CurrentTime);

	if(ShiftMask == modstate)
		XTestFakeKeyEvent(xdpy, SHIFT_KEY_CODE, False, CurrentTime);		

	XTestFakeKeyEvent(xdpy, keycode, False, CurrentTime);

	//*********************** For the use of hiding desktop
	if(5 == modstate) 
	{
		XTestFakeKeyEvent(xdpy, ALT_L_KEY_CODE, False, CurrentTime);
		XTestFakeKeyEvent(xdpy, CTRL_L_KEY_CODE, False, CurrentTime);
	}

	/*flush the action*/
	XFlush(xdpy);
}


/*
 * function name : pageup
 * description: Send the PageUp to the active window.
 *
 */
void pageup(Display *xdpy,KEY_STATUS_MASK mask)
{
	Window Xid;
	XClassHint hint;
	int modstate = 0;

	Xid = get_active_Xid(xdpy);
	if(!Xid)
		return;

	XGetClassHint(xdpy, Xid, &hint);

	if (!strcmp(hint.res_name, "gnome-terminal"))
	{
		modstate |= ShiftMask;
		send_fake_key(xdpy,Xid,PAGE_UP_KEY_CODE,mask,modstate);
	}
	else
	{
		send_fake_key(xdpy,Xid,PAGE_UP_KEY_CODE,mask,modstate);
	}

	XFree(hint.res_name);
	XFree(hint.res_class);
}


/*
 * function name : pagedown
 * description: Send the PageDown to the active window.
 *
 */
void pagedown(Display *xdpy,KEY_STATUS_MASK mask)
{
	DEBUG_PRINT("pagedown");
	Window Xid;
	XClassHint hint;
	int modstate = 0;

	Xid = get_active_Xid(xdpy);
	if(!Xid)
		return;

	XGetClassHint(xdpy, Xid, &hint);

	if (!strcmp(hint.res_name, "gnome-terminal"))
	{
		modstate |= ShiftMask;
		send_fake_key(xdpy,Xid,PAGE_DOWN_KEY_CODE,mask,modstate);
	}
	else
	{
		send_fake_key(xdpy,Xid,PAGE_DOWN_KEY_CODE,mask,modstate);
	}

	XFree(hint.res_name);
	XFree(hint.res_class);
}


/*
 * function name : launch_camera
 * description: Launch the application "Cheese".
 *
 */
void launch_camera(Display *xdpy)
{
	int ret = 0;
	char syscall[CAMERA_CHARMAX];

	memset(syscall, 0, sizeof(syscall));
	strncpy(syscall, camera, strlen(camera));
	strcat(syscall, " 2>/dev/null &");

	/*launch the cheese*/
	ret = system(syscall);
	sleep(2);
}


/*
 * function name : capture_photo
 * description: Use the Function Button to capture photo.
 *
 */
void capture_photo(Display *xdpy, KEY_STATUS_MASK mask)
{
	Window Xid;
	Xid = get_active_Xid(xdpy);
	Window camera_wid = 0;
	camera_wid = search_wid_from_name(xdpy, camera);
	if(camera_wid)
	{
		set_window_active(xdpy, camera_wid);
		//printf ("[capture_photo]_camera_wid=%ld\n",camera_wid);
	}
	send_fake_key(xdpy, Xid, SPACE_KEY_CODE, mask, 0);


}


void show_desktop(Display *xdpy,KEY_STATUS_MASK mask)
{
	if (!xdpy)
		return;

	XTestFakeKeyEvent(xdpy, KEY_CODE_OF_SUPER_L, True, CurrentTime);
	XTestFakeKeyEvent(xdpy, KEY_CODE_OF_SUPER_L, False, CurrentTime);

	/*flush the action*/
	XFlush(xdpy);

}

int open_kbd_device()
{
	int fd = 0;

	fd = open(KBD_DEV, O_RDONLY);	
	if(-1 == fd)
	{
		perror("kbd_open error:\n");
		return -1;
	}
	return fd;
}
void home_key_event_detect(int fd)
{
	key_event event = {0};
	int read_bytes = 0;
	char s[30] = {0};
	int i = 0;	
	read_bytes = read(fd, &event, 16);
	if (read_bytes > 0)
	{
		printf("kbd event : %d\n", event.code);
#if 0
		if (KBD_HOME_KEY_CODE == event.code)
		{
			show_desktop(xdpy,KEY_PRESSED);
		}
#endif
	}
}

/* merged by He,Yingyuan from Button.c end*/
void ProcFnkeyMsg(int oprt)
{
	DEBUG_PRINT("oprt = %d\n", oprt);
	Window Xid;
	int checkcheese = 0, ret = 0;
	char PIDOF[CMAX_BUF];

	strcpy(PIDOF, "pidof ");
	strncat(PIDOF, camera, strlen(camera));

	switch (oprt)
	{
		case VKBD_KEY_FNF1_PRESS:
			SetWlanDownPressed();
			break;
		case VKBD_KEY_FNF1_RELEASE:
			break;
		case VKBD_KEY_FNF6_PRESS:
			SetDisplayDownPressed();
			break;
		case VKBD_KEY_FNF6_RELEASE:
			break;
		case VKBD_KEY_FNF7_PRESS:
			SetBacklightDownPressed();
			break;
		case VKBD_KEY_FNF7_RELEASE:
			SetBacklightDownReleased();
			break;
		case VKBD_KEY_FNF8_PRESS:
			SetBacklightUpPressed();
			break;
		case VKBD_KEY_FNF8_RELEASE:
			SetBacklightUpReleased();
			break;
		case VKBD_CAMERA_KEY_RELEASED:
			if (strcmp(camera, "nocamera"))
			{
				checkcheese = CAMERA_STATUS(PIDOF);
				if(checkcheese)
				{
					//		Window Xid;
					//		Xid = get_active_Xid(xdpy);
					//			if (!Xid)
					//				show_desktop(xdpy, KEY_PRESSED);
					launch_camera(xdpy);
				}
			}
			else
				printf("you have not specified a camera application.\n");
			break;
		case VKBD_CAMERA_KEY_PRESSED:
			if (strcmp(camera, "nocamera"))
			{
				checkcheese = CAMERA_STATUS(PIDOF);

				if(!checkcheese)
					capture_photo(xdpy,KEY_PRESSED);
			}
			else
				printf("you have not specified a camera application.\n");
			break;
		case VKBD_PAGEDOWN_FN_PRESSED:
			pagedown(xdpy,KEY_PRESSED);
			break;
		case VKBD_PAGEDOWN_FN_RELEASED:
			break;
		case VKBD_PAGEUP_FN_PRESSED:
			pageup(xdpy,KEY_PRESSED);
			break;
		case VKBD_PAGEUP_FN_RELEASED:
			break;
		case VKBD_SHOW_DESKTOP_FN_PRESSED:
			show_desktop(xdpy,KEY_PRESSED);
			break;
		case VKBD_SHOW_DESKTOP_FN_RELEASED:
			break;
		default:
			break;
	}	

	return;
}


int get_wireless_data(int fd, char *path)
{
	cmpc_pm_acpi_t	data;

	memset(&data, 0, sizeof(data));
	strcpy(data.pathname, path);

	if (ioctl(fd, CMPC_PM_ACPI_GET_WIRELESS, &data))
		return 0;

	return data.status;
}

int open_wifi_dev()
{
	int fd = 0;

	fd = open(WIFI_DEV, O_RDONLY);
	if (-1 ==fd) 	
	{
		DEBUG_PRINT("Can't find wifi device\n");	
		return -1;
	}
	return fd;
}


int get_wifi_status()
{
	int ret = 0;
	int ret1 = 0;

	ret = system("/usr/sbin/rfkill list | grep yes");
	ret1 = system("/usr/sbin/rfkill list | grep no");
	if (0 == ret)
	{
		return WIFI_STATUS_CLOSE;	
	}
	else if (0 == ret1)
	{
		return WIFI_STATUS_OPEN;
	}
	else 
	{
		return WIFI_STATUS_CLOSE;
	}
}

void monitor_wifi_status()
{
	int wifi_status = 0;

	wifi_status = get_wifi_status();
	if (WIFI_LED_ON == led_status() && WIFI_STATUS_CLOSE == wifi_status)
	{
		led_off();
	}
	else if (WIFI_LED_ON != led_status() && WIFI_STATUS_OPEN == wifi_status)
	{
		led_on();
	}

}

int set_wireless_data(int fd, char *path, int value)
{
	cmpc_pm_acpi_t	data;

	memset(&data, 0, sizeof(data));
	data.inputparam = value;
	strcpy(data.pathname, path);

	if (ioctl(fd, CMPC_PM_ACPI_SET_WIRELESS, &data))
		return 0;

	return data.status;
}

void SetWlanDownPressed(void)
{
#if 0
/* WIFI LED RDK CONTROL */
	int wifi_status = 0;
	int led_state = 0;

	led_state = led_status();
	printf("led state : %d\n", led_state);

	wifi_status = get_wifi_status();
	if (WIFI_STATUS_OPEN == wifi_status)
	{
		system("/usr/sbin/rfkill block wifi");
		led_off();
		OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_WRL, 0);
	}
	else if (WIFI_STATUS_CLOSE == wifi_status)
	{
		system("/usr/sbin/rfkill unblock wifi");
		led_on();
		OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_WRL, 1);
	}
#else
/* WIFI LED NETWORKMANAGER CONTROL */
	int ret, wifi_status;

	wifi_status = system("dbus-send --system --type=method_call --print-reply --dest=org.freedesktop.NetworkManager /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Get string:org.freedesktop.NetworkManager string:WirelessEnabled |grep true -q");

	if (!WEXITSTATUS(wifi_status))
	{
		// disable wifi
		ret = system("dbus-send --system --type=method_call --print-reply --dest=org.freedesktop.NetworkManager /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Set string:org.freedesktop.NetworkManager string:WirelessEnabled variant:boolean:false &>/dev/null");
		OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_WRL, 0);
	}
	else
	{
		// enable wifi
		ret = system("dbus-send --system --type=method_call --print-reply --dest=org.freedesktop.NetworkManager /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Set string:org.freedesktop.NetworkManager string:WirelessEnabled variant:boolean:true &>/dev/null");
		OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_WRL, 1);
	}
#endif
}


static unsigned int dis_num = 0;

void SetDisplayDownPressed(void)
{
	int ret = 0;

	ret = system("/bin/bash /usr/bin/cmpcfk-control.sh conn");

	if (!WEXITSTATUS(ret))
	{
		dis_num++;
		dis_num%=3;
		set_display(dis_num);
		sleep(1);
		OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_DSP, dis_num+1);
	}
}


void SetBacklightDownPressed(void)
{
	int bright = 0;

	bright = get_brightness(pm_dev, m_pDriverPath);
	bright --;

	if (bright <= 0) bright = 0;
	if (bright >= 7) bright = 7;
	usleep(1000);
	set_brightness(pm_dev, m_pDriverPath, bright);
	OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_BRG, bright);
}


void SetBacklightDownReleased(void)
{
}


void SetBacklightUpPressed(void)
{
	int bright = 0; 

	bright = get_brightness(pm_dev, m_pDriverPath);
	bright ++;

	if (bright <= 0) bright = 0;
	if (bright >= 7) bright = 7;

	usleep(1000);
	set_brightness(pm_dev, m_pDriverPath, bright);
	OnScrDsp::GetIns().ShowWnd(OnScrDsp::WND_BRG, bright);
}


void SetBacklightUpReleased(void)
{
}


void DelPip(int sgn)
{
	int ret = 0;

	/* ignore all other signals */
	if ((SIGHUP == sgn) || (SIGINT == sgn) || (SIGQUIT == sgn) ||
			(SIGILL == sgn) || (SIGABRT == sgn) || (SIGSEGV == sgn) ||
			(SIGTERM == sgn) || (SIGTSTP == sgn) || (SIGPIPE == sgn)) 
	{
		printf("the process is terminated and delete pipe file\n");
		ret = system("rm -f /var/lock/fnkey");

		if (vkd_dev != -1) close(vkd_dev);
		if (pm_dev != -1) close(pm_dev);

		vkd_dev = -1;
		pm_dev = -1;

		exit(1);
	}
}


static int key = 0;

void *VkdEventHandler(void *arg)
{
	ssize_t ret;

	while (1)
	{
		ret = read(vkd_dev, &key, sizeof(int)); 
		ProcFnkeyMsg(key);
	}
}

void *KBDEVENTHandler(void *arg)
{
	while (1)
	{
		home_key_event_detect(kbd_dev_fd);
	}

}

void *WIFILEDHandler(void *arg)
{
	while (1)
	{
		monitor_wifi_status();
		sleep(1);
	}

}

int stop_p_event()
{
    Display *disp = XOpenDisplay(NULL);
    if (disp)
    {
        XTestFakeKeyEvent(disp, XK_KP_Space, True, CurrentTime);
        XCloseDisplay(disp);
        return 0;
    }
    return -1;
}

void *InputEvent4Handler(void *arg)
{
	struct input_event ie = {0};
	__u8 win_down = 0;

	while (1)
	{
		if (read(event4_dev, &ie, sizeof(struct input_event)) < 0) 
		{
	//		printf("Couldn't read the event4 data.\n");
		}
		else
		{
			printf("ie.code = %d, ie.value = %d\n", ie.code, ie.value);
		}
#if 0
		if (ferror(event4_dev))
		{
			perror("fread");
			exit(1);
		}
		printf("ie.code = %d, ie.value = %d\n", ie.code, ie.value);
		if ( !win_down && ie.code == 125 && ie.value > 0)
		{
			win_down = 1;
//			printf("The win key has been down.\n");
			continue;
		}

		if ( win_down && ie.code == 25 && ie.value == 1)
		{
			//printf("The win and p keys have been pressed down.\n");
			SetDisplayDownPressed();
			win_down = 0; // Release the win key up programly, even though pressed down.
		}

		if ( win_down && ie.code == 125 && ie.value == 0)
		{
			win_down = 0;
//			printf("The win key has been up.\n");
		}

		if (ie.code == 0 && ie.value == 0)
		{
			if (stop_p_event() < 0)
			{
				printf("Cannot send the fake key.\n");
			}
		}
#endif
	}	
}

void led_on(void)
{
	ioctl(vkd_dev, CMPC_ACPI_SET_LED_ON);
}

void led_off(void)
{
	ioctl(vkd_dev, CMPC_ACPI_SET_LED_OFF);
}
unsigned long led_status(void)
{
	unsigned long status = 0; 

	ioctl(vkd_dev, CMPC_ACPI_GET_LED_STATUS, &status);
	return status;
}


static pthread_t read_Thread;
static pthread_t kbd_thread;
static pthread_t wifi_led_thread;
static pthread_t win_p_thread;

int main(int argc, char* argv[])
{
	int ret;
	int entries[ACPI_MAX_STRING] = {0};



	// wifi_fd = open_wifi_dev();
	/* install signal handlers to delete named pipe file */
	if ((SIG_ERR == signal(SIGHUP, DelPip)) ||
			(SIG_ERR == signal(SIGINT, DelPip)) ||
			(SIG_ERR == signal(SIGQUIT, DelPip)) ||
			(SIG_ERR == signal(SIGILL, DelPip)) ||
			(SIG_ERR == signal(SIGABRT, DelPip)) ||
			(SIG_ERR == signal(SIGFPE, DelPip)) ||
			(SIG_ERR == signal(SIGSEGV, DelPip)) ||
			(SIG_ERR == signal(SIGPIPE, DelPip)) ||
			(SIG_ERR == signal(SIGALRM, DelPip)) ||
			(SIG_ERR == signal(SIGTERM, DelPip)) ||
			(SIG_ERR == signal(SIGUSR1, DelPip)) ||
			(SIG_ERR == signal(SIGUSR2, DelPip)) ||
			(SIG_ERR == signal(SIGTSTP, DelPip)) ||
			(SIG_ERR == signal(SIGTTIN, DelPip)) ||
			(SIG_ERR == signal(SIGTTOU, DelPip))) {
		printf("Install signal handlers failed\n");
		return 1;
	}

	ret = mkfifo("/var/lock/fnkey", S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
	if (ret)
	{
		printf("there is lock file under /var/lock/, please check if fnkey is already started!\n");
		return -1;
	}

#ifndef DEBUG
	daemonize("FnKey");
#endif

	if(open_dev()) goto err;

//	kbd_dev_fd = open_kbd_device();
//	memset(m_pDriverPath, 0, ACPI_MAX_STRING);
//	get_path(pm_dev, NULL, entries, 1, m_pDriverPath);
//	if (strlen(m_pDriverPath) <= 0) goto err;

	xdpy = XOpenDisplay(NULL);
	if(NULL == xdpy)
		goto err;

	screen_num = DefaultScreen(xdpy);
	display_width = DisplayWidth(xdpy, screen_num);
	display_height = DisplayHeight(xdpy, screen_num);

	select_camera();

	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	OnScrDsp::GetIns();

	if (pthread_create(&read_Thread, NULL, VkdEventHandler, NULL)) goto err;

/*
	if (-1 != kbd_dev_fd) 
	{
		 if (pthread_create(&kbd_thread, NULL, KBDEVENTHandler, NULL)) goto err;
	}
	if (-1 != wifi_fd)
	{
		if (pthread_create(&wifi_led_thread, NULL, WIFILEDHandler, NULL)) goto err;
	}
	if (pthread_create(&win_p_thread, NULL, InputEvent4Handler, NULL)) goto err;
*/

	while (1)
	{
		gtk_main();
		sleep(3);
	}

err:

	if (xdpy != NULL) 
	{
		XCloseDisplay(xdpy);
		xdpy == NULL;
	}

	if (vkd_dev != -1) 
	{
		close(vkd_dev);
		vkd_dev = -1;
	}
/*
	if (wifi_fd != -1) 
	{
		close(wifi_fd);
		wifi_fd = -1;
	}
	if (kbd_dev_fd != -1) 
	{
		close(kbd_dev_fd);
		kbd_dev_fd = -1;
	}
	if (pm_dev != -1) 
	{
		close(pm_dev);
		pm_dev = -1;
	}
*/

	ret = system("rm -f /var/lock/fnkey");

	return 0;	
}
