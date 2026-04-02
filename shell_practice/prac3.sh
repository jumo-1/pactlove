#!/bin/bash
set -x
func3() {
FILE="file.txt"

echo "优秀学院名单(>=80)"
#awk '$NF>80 { print $0 }' $FILE 
awk '{if( $2>=80) print $0}' $FILE | sort -t' ' -k2nr -k1
AVE=$(awk '{ sum+=$2 } END {print sum/FNR}' $FILE) 
echo "平均成绩:$AVE"
}
