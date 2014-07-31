#!/bin/sh
if [ $(/sbin/ifconfig|grep wlan|awk '{print $1}') ];then
/sbin/rmmod r8192se_pci
else 
sleep 1 
/sbin/modprobe r8192se_pci
sleep 1 
/sbin/rmmod r8192se_pci
sleep 1 
/sbin/modprobe r8192se_pci
fi
