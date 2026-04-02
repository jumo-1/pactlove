#!/bin/bash
#source ./prac1.sh
#source ./prac2.sh
#source ./prac3.sh
set -x
if [ $# -ne 1 ]; then
	echo "参数数量不匹配，需要一个参数"
	exit 1
fi
FILE=$1
TOFILE=clean.log
check_file() {
	find $1
	if [ ! -f $1 ]; then
		echo "文件$1不存在"
		exit 1
	fi
}
check_file $FILE
touch $TOFILE

sed  's/[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}/[MASKED_IP]/g' $FILE  > $TOFILE
ERRsum=$(awk '/ERROR/{count++} END{ print count}' $TOFILE)
echo "错误数量为：$ERRsum"
echo "错误前两名为："
awk '/ERROR/' $TOFILE |sort |uniq -c |sort -nr|head -n 2

