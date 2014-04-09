#!/bin/bash

ssh_options="-o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no"
# ConnectTimeout: fail after 10 seconds without response - seems to be iffy, use 'timeout' command and chain
# BatchMode: Automatic 'yes' to add to known hosts
# StrictHostKeyChecking: Auto add the fingerprint

# Run a command on each computer, and set the result as the exit status...
for ((i = 1; i <= 35; i+=1));do
  ((echo 'COUNT=$(ps -eLf | grep -v "grep" | grep -c "server_slave"); exit $COUNT'; exit) | timeout 10s ssh -T -p $SSH_PORT $ssh_options aus213l$i) 2>/dev/null &
  PIDS[$i]=$!
done

# Join back up with the background commands, and read the exit statuses
count=0
for ((i = 1; i <= 35; i+=1));do
   wait ${PIDS[$i]}
   status[$i]=$?
   let count+=${status[$i]}
done

# Output the counts of servers running on each system
echo "Server map:"
OLD_IFS=$IFS
IFS=$'\n'
printf -- '%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%3s%3s%3s%3s%3s  %3s%3s%3s%3s%3s\n%20s%3s%3s%3s%3s\n' 1 2 3 4 5 6 7 8 9 10 ${status[@]}
IFS=$OLD_IFS
echo
echo "Total servers running: $count"
