#!/bin/sh

rm monitor.txt

COUNT=0
#while true
# Close if no program is running for over 10 seconds
while [ $COUNT -le 20 ]
do
   if [ $(ps aux | grep -E -v -e "grep|bash" | grep -E -c -e "strassen|client_manager|server_leaf") -gt 0 ]; then
      COUNT=0
      echo "$(ps aux | grep -E -v -e "grep|bash" | grep -E -e "strassen|client_manager|server_leaf")" >> monitor.txt
   else
      let COUNT+=1
   fi
   sleep 0.5
done
