#!/bin/bash

ssh_options="-o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no"
# ConnectTimeout: fail after 10 seconds without response - seems to be iffy, use 'timeout' command and chain
# BatchMode: Automatic 'yes' to add to known hosts
# StrictHostKeyChecking: Auto add the fingerprint

# Requires SSH_PORT to be set in the environment
for ((i = 1; i <= 35; i+=1))do
  ((echo -e 'servCount=$(ps -eLf | grep -v "grep" | grep -c "server_leaf")\n pkill server_leaf\n echo "servers on $(hostname -i): $servCount $(ps -eLf | grep -v "grep" | grep -c "server_leaf")"' ; exit) | timeout 10s ssh -T -p $SSH_PORT $ssh_options aus213l$i) &
done
wait
