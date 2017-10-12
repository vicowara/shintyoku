#!/bin/bash

MAJOR=`awk '/shintyoku/ {print $1}' /proc/devices`
mknod /dev/shintyoku c $MAJOR 0
