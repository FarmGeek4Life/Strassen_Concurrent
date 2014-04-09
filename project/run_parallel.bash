#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Usage: $0 port computer(xxx) [computer] [computer] [computer]"
  echo "Requires SSH_PORT to be set in the environment"
  exit
fi

ssh_options="-o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no"
# ConnectTimeout: fail after 10 seconds without response - seems to be iffy, use 'timeout' command and chain
# BatchMode: Automatic 'yes' to add to known hosts
# StrictHostKeyChecking: Auto add the fingerprint

args=("$@")

port=${args[0]}
ip_start="$(hostname -i | grep -E -o -e "^([0-9]{1,3}\.){3}")"
computers="$ip_start${args[1]}"
for ((i = 2; i < $#; i+=1))do
  computers="$computers:$ip_start${args[$i]}"
done

export BG_COMPUTERS="$computers"
export BG_PORT="$port"

for ((i = 1; i < $#; i+=1))do
  echo "System $i..."
  ((echo "nohup ~/cs499/project/server_slave $port >/dev/null 2>&1 &"; exit) | timeout 10s ssh -T -p $SSH_PORT $ssh_options 157.201.194.${args[$i]}) &
done

# Clean up the variables to not leave them as artifacts in the shell environment
unset ssh_options
unset ip_start
unset args
unset port
unset computers
unset i
# Allow the servers to all start up
sleep 2