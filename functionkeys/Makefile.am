prefix=/usr
etcdir=/etc
bin_PROGRAMS=fnkey

fnkey_CPPFLAGS=${CFLAGS}
fnkey_LDADD=${LDADD}
#wlan key is 139 with the scope of the marco CDV
#DEFS=-DCDV -DDEBUG
DEFS=-DCDV
LDADD+=-lX11

include_HEADERS=button.h cmpc_pm.h OnScrDsp.h
fnkey_SOURCES=fnkey.cpp OnScrDsp.cpp 

fnkey_DATA=data/Images/*.png
fnkeydir=${datadir}/fnkey/images

acpi_DATA=data/*.sh
acpidir=${etcdir}/acpi

acpi_conf_DATA=data/*.conf
acpi_confdir=${etcdir}/apci/events

camera_conf_DATA=data/camera.conf
camera_confdir=${etcdir}/cmpcfk

display_DATA=data/cmpcfk-control.sh
displaydir=${bindir}

deb_DATA=data/deb/*
debdir=/DEBIAN

launch_DATA=data/fnkey-start.sh
launchdir=${etcdir}/profile.d/

debug_DATA=data/debug/*
debugdir=${bindir}/fnkey_debug/

EXTRA_DIST=data/* autogen.sh
