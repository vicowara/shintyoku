#!/bin/bash

if [ -e /dev/shintyoku ]; then
	rm /dev/shintyoku
fi

MAJOR=`awk '/shintyoku/ {print $1}' /proc/devices`
mknod /dev/shintyoku c $MAJOR 0
