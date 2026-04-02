#!/bin/bash
set -x
func2() {
name="Avail"
DATE=$(date)

NUM=$(ls -l | grep "^-rwx" | wc -l)
for i in $(seq 1 7);do
	na=$(df -h / |awk -v col=$i 'NR==1 {print $col}')
       if [ $na == $name ]; then
	       av=$(df -h / | awk -v col=$i 'NR==2 {print $col}')
       fi
done
file="report.txt"
touch $file
echo "====================" > $file
echo "time:$DATE" >> $file
echo "====================" >> $file
echo "sum:$NUM" >> $file
echo "====================" >> $file
echo "avail:$av" >> $file
echo "====================" >> $file
}
