#!/bin/sh

pidof fnkey 

if [ $? = 0 ]; then
        killall fnkey 
fi

rm -f /var/lock/fnkey &> /dev/null

## fnkey login begin ##
/usr/bin/fnkey &>/dev/null
## fnkey login end ##

