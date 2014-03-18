#!/bin/bash

for ((i = 1; i <= 35; i+=1))do
  #echo "---------------------------- aus213l$i ----------------------------------"
  ((echo -e 'servCount=$(ps -eLf | grep -v "grep" | grep -c "server_leaf")\n pkill server_leaf\n echo "servers on $(hostname -i): $servCount $(ps -eLf | grep -v "grep" | grep -c "server_leaf")"' ; exit) | ssh -T -p 215 aus213l$i) &
  #echo ""
done
wait
