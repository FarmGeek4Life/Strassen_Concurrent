#!/bin/bash

#export BG_COMPUTERS="x.x.x.x:y.y.y.y:z.z.z.z"
#export BG_PORT="xxxxx"

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

echo "$computers"
export BG_COMPUTERS="$computers"
echo "$port"
export BG_PORT="$port"

for ((i = 1; i < $#; i+=1))do
  echo "---------------------------- System $i ----------------------------------"
  # Getting problem from this line...... - & not working
  #(echo "~/cs499/project/server_leaf $port &"; exit) | ssh -T -p 215 157.201.194.${args[$i]}
  ((echo "nohup ~/cs499/project/server_leaf $port >/dev/null 2>&1 &"; exit) | ssh -T -p 215 157.201.194.${args[$i]}) &
  #ssh -p 215 157.201.194.${args[$i]} 'nohup "/home/gib09003/cs499/project/server_leaf $port 2>&1 >/dev/null &"; exit'
  #echo "~/cs499/project/server_leaf $port &"
  #echo "ssh -T -p 215 157.201.194.${args[$i]}"
  #sleep 1
  #echo ""
done
sleep 2