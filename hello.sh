#!/bin/bash
ws="working space"
ws__=$(pwd)
echo "time:"
date
echo "用户名:$(whoami)"
for filename in *.c ;do
	echo "the:$filename"
done

echo "$ws:$ws_"
file="hello.c"
if [ -f $file ];then
	echo "$file,yes"
else
	echo "$file,no"
fi
for i in {1..5};do
	echo "let us:$i"
done
for i in {0..20..2};do
	if [ $i -eq 10 ];then
		echo "the:$i"
		continue
	fi
	if [ $i -eq 14 ];then
		echo "bye"
		break
	fi
	
	echo "$i"
	
done
while read line ;do
	echo "$line"
done < $filename

