#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Usage: $0 port computer(xxx) [computer] [computer] [computer]"
  exit
fi

args=("$@")

port=${args[0]}
computers="157.201.194.${args[1]}"
for ((i = 2; i < $#; i+=1))do
  computers="$computers:157.201.194.${args[$i]}"
done

export BG_COMPUTERS="$computers"
export BG_PORT="$port"

for ((i = 1; i < $#; i+=1))do
  echo "System $i..."
  ((echo "nohup ~/cs499/project/server_leaf $port >/dev/null 2>&1 &"; exit) | ssh -T -p 215 157.201.194.${args[$i]}) &
done

# Clean up the variables to not leave them as artifacts in the shell environment
unset args
unset port
unset computers
unset i
# Allow the servers to all start up
sleep 2