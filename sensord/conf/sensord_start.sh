#!/bin/sh

############ sensord daemon #############
killall sensord &>/dev/null
rm -f /var/lock/sensord &>/dev/null
/usr/bin/sensord &>/dev/null
sleep 1

