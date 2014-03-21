#!/bin/bash
# Using grep -v 'text' will ignore all lines that have 'text' on them

# write a script that will ssh or telnet into every computer in the lab and run 'who' in the terminal, to see who is actively logged in and using the computer.
# also could write a script that will run any following command on the computer to do quick lab analysis.
# refer to nss.sh and asdf.sh for some tips.

# linux lab computers are from 1 to 35
for ((i = 1; i <= 35; i+=1))do
  #(echo -n "uptime | grep -E -o -e \"[0-9]{1,3} user(s)?,  load average:.*\"") | ssh -T -p 215 aus213l$i
  (echo -ne 'echo -n "$(hostname -i): "\n uptime | grep -E -o -e "[0-9]{1,3} user(s)?,  load average:.*"') | ssh -T -p 215 aus213l$i
done
