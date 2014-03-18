#!/bin/bash

for ((i = 1; i <= 35; i+=1));do
  #echo "---------------------------- aus213l$i ----------------------------------"
  #((echo -e 'for PID in $(ps -eo pid,user,cmd | grep "server_leaf" | grep -v "grep" | grep -E -o -e "^[0-9]{1,7}")\n do\n echo "found server_leaf on $(hostname -i)"\n done\n' ; \
  #exit) | ssh -T -p 215 aus213l$i) &
  #(status[$i]=$((echo 'echo "$(ps -eLf | grep -v "grep" | grep -c "server_leaf")"' ; exit) | ssh -T -p 215 aus213l$i)) &
  ((echo 'COUNT=$(ps -eLf | grep -v "grep" | grep -c "server_leaf"); exit $COUNT'; exit) | ssh -T -p 215 aus213l$i) 2>/dev/null &
  PIDS[$i]=$!
  #echo ""
done
#wait

#i=1;
#for pid in ${PIDS[@]}; do
for ((i = 1; i <= 35; i+=1));do
   #wait $pid
   wait ${PIDS[$i]}
   status[$i]=$?
   #echo "$i"
   #i=$i+1
done

echo "Server map:"
#echo " ${status[1]} ${status[2]} ${status[3]} ${status[4]} ${status[5]}   ${status[6]} ${status[7]} ${status[8]} ${status[9]} ${status[10]}"
#echo " ${status[11]} ${status[12]} ${status[13]} ${status[14]} ${status[15]}   ${status[16]} ${status[17]} ${status[18]} ${status[19]} ${status[20]}"
#echo " ${status[21]} ${status[22]} ${status[23]} ${status[24]} ${status[25]}   ${status[26]} ${status[27]} ${status[28]} ${status[29]} ${status[30]}"
#echo "             ${status[31]} ${status[32]} ${status[33]} ${status[34]} ${status[35]}"
#echo
#
#printf -- '%3s%3s%3s%3s%3s   %3s%3s%3s%3s%3s\n' ${status[1]} ${status[2]} ${status[3]} ${status[4]} ${status[5]} ${status[6]} ${status[7]} ${status[8]} ${status[9]} ${status[10]}
#printf -- '%3s%3s%3s%3s%3s   %3s%3s%3s%3s%3s\n' ${status[11]} ${status[12]} ${status[13]} ${status[14]} ${status[15]}   ${status[16]} ${status[17]} ${status[18]} ${status[19]} ${status[20]}
#printf -- '%3s%3s%3s%3s%3s   %3s%3s%3s%3s%3s\n' ${status[21]} ${status[22]} ${status[23]} ${status[24]} ${status[25]}   ${status[26]} ${status[27]} ${status[28]} ${status[29]} ${status[30]}
#printf -- '%21s%3s%3s%3s%3s\n' ${status[31]} ${status[32]} ${status[33]} ${status[34]} ${status[35]}
#echo
#printf -- '%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%20s%3s%3s%3s%3s\n' ${status[1]} ${status[2]} ${status[3]} ${status[4]} ${status[5]} ${status[6]} ${status[7]} ${status[8]} ${status[9]} ${status[10]} ${status[11]} ${status[12]} ${status[13]} ${status[14]} ${status[15]}   ${status[16]} ${status[17]} ${status[18]} ${status[19]} ${status[20]} ${status[21]} ${status[22]} ${status[23]} ${status[24]} ${status[25]}   ${status[26]} ${status[27]} ${status[28]} ${status[29]} ${status[30]} ${status[31]} ${status[32]} ${status[33]} ${status[34]} ${status[35]}
#echo
# This is much cleaner...
OLD_IFS=$IFS
IFS=$'\n'
printf -- '%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%20s%3s%3s%3s%3s\n' 1 2 3 4 5 6 7 8 9 10 ${status[@]}
IFS=$OLD_IFS