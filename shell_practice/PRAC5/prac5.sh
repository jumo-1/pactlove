#!/bin/bash
set -x
log_action() {
	if [ $1 -ne 0 ];then
		echo  " 该文件处理错误：$2"
		return 1
	fi
	if [ $1 -eq 0 ];then
		echo  " 该文件处理成功：$2"
		return 0
	fi
}
if [ $# -ne 3 ]; then
	echo "参数个数不匹配，需要三合参数，当前只有$#个"
	exit 1
fi	
BOOL=0
for FILE in $@; do
	sed -i 's/[0-9]\{1,4\}/8081/' $FILE
	curr=$?
       	log_action $curr #FILE
	if [ $curr -ne 0 ]; then
		BOOL=1
	fi
done
if [ $BOOL -ne 0 ];then
	exit 1
fi
exit 0



