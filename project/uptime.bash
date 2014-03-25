#!/bin/bash

# linux lab computers are from 1 to 35
for ((i = 1; i <= 35; i+=1))do
  #(echo -n "uptime | grep -E -o -e \"[0-9]{1,3} user(s)?,  load average:.*\"") | ssh -T -p 215 aus213l$i
  (echo -ne 'echo -n "$(hostname -i): "\n uptime | grep -E -o -e "[0-9]{1,3} user(s)?,  load average:.*"') | ssh -T -p 215 aus213l$i
done
