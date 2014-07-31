#!/bin/sh


DIS=`xrandr|grep -s "\<connected"|awk '{ print $1 }'`
echo $DIS
MODE=`xrandr|awk '{ print $1 }'|grep ^[0-9]|sort -tx -nr -k1|awk 'BEGIN{tmp=""} { if($1==tmp) { print $1; } tmp=$1; }'|sort -tx -nr -k1|head -1`
echo $MODE

case "$1" in
"dual" )
	xrandr --output `echo "$DIS" |grep -s "eDP1"` --mode $MODE --output `echo "$DIS"|grep -sv "eDP1"` --mode $MODE
	;;
"lvds" )
	xrandr --output `echo "$DIS" |grep -s "eDP1"` --auto --output `echo "$DIS"|grep -sv "eDP1"` --off
	;;
"other" )
	xrandr --output `echo "$DIS" |grep -s "eDP1"` --off --output `echo "$DIS"|grep -sv "eDP1"` --auto
	;;
"conn" )
	xrandr | grep "\<connected" | grep -vq "eDP1"
	exit $?
	;;
esac

exit 0
