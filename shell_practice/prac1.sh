#!/bin/bash
set -x
lo=1
SEQ=$(seq 1 10)

func1() {
#for i in $SEQ;do
#	touch  test_$i.txt
#done

#for i in $SEQ;do
#	if [ $((i%2)) -eq 0 ]; then
#		mv test_$i.txt test_$i.txt.bak
#	fi
#done

#for file in test_*.txt.bak;do
#	num=$(echo "$file" |awk -F'[_.]' '{print $2}')
#	if [ $((num%2)) -eq 0 ] ; then
#		mv $file $file.bak
#	fi
#done

#for file in test_*.txt.bak;do
#	temp=${file#*_}
#	num=${temp%%.*}
#	if [ $num -eq 3 ] ; then
#		mv $file test_$num.txt
#	fi
#done

for file in test_*.txt.bak.bak;do
        temp=${file#*_}
        num=${temp%%.*}
        mv $file test_$num.txt 
done
}
