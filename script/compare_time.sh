#!/bin/bash

# ./compare_time.sh 10.10.10.160
# ./compare_time.sh 192.168.22.241

(date +%s;echo -e 'time\r\n'|nc -w1 $1 5002 )|awk '{if(NR==1){a=$1}} END{printf"%d %d %d\n", a,$1,a-$1;}'
