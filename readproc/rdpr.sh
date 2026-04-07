#!/bin/bash
set -x -u
FILE1=/proc/loadavg
FILE2=/proc/meminfo
n1=$(awk '{print $1}' $FILE1)
echo "$n1"
n2=$(awk '/MemTotal/ {print $2}' $FILE2)
n3=$(awk '/MemAvailable/ {print $2}' $FILE2)
echo "$(((n3*100)/n2))%"
