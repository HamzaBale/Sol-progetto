#!/bin/bash

while [ -e /proc/$1 ]
do 
    sleep 1
done

if [ -f "statsfile.log" ]; then
    echo "$0:esketit"
    while read line; do echo $line; done < statsfile.log
else
    echo "$0:ErrorEEEEEEE" 1>&2
fi
echo "$0:gang"
