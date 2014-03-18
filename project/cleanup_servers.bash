#!/bin/bash

for ((i = 1; i <= 35; i+=1))do
  #echo "---------------------------- aus213l$i ----------------------------------"
  ((echo -e 'servCount=$(ps -eLf | grep -v "grep" | grep -c "server_leaf")\n for PID in $(ps -eo pid,user,cmd | grep -v "grep" | grep "server_leaf" | grep -E -o -e "^[0-9]{1,7}")\n do\n kill -9 $PID\n echo "killed server_leaf on $(hostname -i)"\n done\n echo "servers on $(hostname -i): $servCount $(ps -eLf | grep -v "grep" | grep -c "server_leaf")"' ; \
  exit) | ssh -T -p 215 aus213l$i) &
  #echo ""
done