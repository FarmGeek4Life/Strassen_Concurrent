#!/bin/bash

ssh_options="-o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no"
# ConnectTimeout: fail after 10 seconds without response - seems to be iffy, use 'timeout' command and chain
# BatchMode: Automatic 'yes' to add to known hosts
# StrictHostKeyChecking: Auto add the fingerprint

# linux lab computers are from 1 to 35
for ((i = 1; i <= 35; i+=1))do
  #(echo -n "uptime | grep -E -o -e \"[0-9]{1,3} user(s)?,  load average:.*\"") | ssh -T -p 215 $ssh_options aus213l$i
  (echo -ne 'echo -n "$(hostname -i): "\n uptime | grep -E -o -e "[0-9]{1,3} user(s)?,  load average:.*"') | timeout 10s ssh -T -p 215 $ssh_options aus213l$i
done
