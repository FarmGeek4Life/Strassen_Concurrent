#!/bin/bash

for ((i = 1; i <= 35; i+=1))do
  #echo "---------------------------- aus213l$i ----------------------------------"
  #((echo -e 'for PID in $(ps -eo pid,user,cmd | grep "server_leaf" | grep -v "grep" | grep -E -o -e "^[0-9]{1,7}")\n do\n echo "found server_leaf on $(hostname -i)"\n done\n' ; \
  #exit) | ssh -T -p 215 aus213l$i) &
  ((echo 'echo "servers on $(hostname -i): $(ps -eLf | grep -v "grep" | grep -c "server_leaf")"' ; exit) | ssh -T -p 215 aus213l$i) &
  #echo ""
done
wait