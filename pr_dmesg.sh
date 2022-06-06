#!/bin/bash
end=`dmesg |wc -l`
while true
do
    sleep 1
    end2=`dmesg |wc -l`
 
    if [ "$end" != "$end2" ]; then
        dmesg |awk '{print NR, $0}'|tail -$((end2-end))
        end=$end2
    fi
    
    if [ "$end" -ge 1000 ]
    then
        dmesg -c >/dev/null 2>&1
        echo "**********************"
        echo "*dmesg -c now;       *"
        echo "**********************"
        end=`dmesg |wc -l`
    fi
 
done
