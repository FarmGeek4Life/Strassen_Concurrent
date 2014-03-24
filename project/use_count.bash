#!/bin/bash

#export BG_COMPUTERS="x.x.x.x:y.y.y.y:z.z.z.z"
#export BG_PORT="xxxxx"

if [ $# -lt 2 ]; then
  echo "Usage: $0 port numComputers"
  exit
fi

args=("$@")
port=${args[0]}
if [ ${args[1]} -gt 34 ]; then
   args[1]=34
fi
#this_comp=$(hostname -i | grep -E -o -e "[0-9]{1,3}$")
this_comp=$(hostname -i)
# Full computer listing
for ((i = 1; i <= 35; i+=1))do
   let num=$i+200
   computers[$i]="157.201.194.$num"
   #echo "$num"
done
# Computer preference order....
comp_pref=( 6 7 8 9 10 16 17 18 19 20 26 27 28 29 30 31 32 33 34 35 5 14 15 21 22 23 24 25 1 2 3 4 11 12 13 )
# 1: most used system (remotely)
# 2 3 4 11 12 13: NX systems (more sensitive to cpu usage)
# 21 22 23 24 25: Often used remotely by beginning students; often in local use because of access

count=0
comp_list=""
for ((i = 0; $count < ${args[1]}; i+=1))do
   temp=${computers[${comp_pref[$i]}]}
   if [[ $temp != $this_comp ]]; then
      computers2[$count]=$temp
      comp_list="$comp_list:$temp"
      let count+=1
   fi
done
# Remove first semicolon, while not having to duplicate some of the script above
comp_list=$(echo "$comp_list" | sed 's/^://')

#echo "${computers2[@]}"
#echo "$comp_list"
export BG_COMPUTERS="$comp_list"
#echo "$port"
export BG_PORT="$port"

for i in ${computers2[@]}; do
  echo "System $i:"
  # Getting problem from this line...... - & not working
  #(echo "~/cs499/project/server_leaf $port &"; exit) | ssh -T -p 215 157.201.194.${args[$i]}
  ((echo "nohup ~/cs499/project/server_leaf $port >/dev/null 2>&1 &"; exit) | ssh -T -p 215 $i) &
  #ssh -p 215 157.201.194.${args[$i]} 'nohup "/home/gib09003/cs499/project/server_leaf $port 2>&1 >/dev/null &"; exit'
  #echo "~/cs499/project/server_leaf $port &"
  #echo "ssh -T -p 215 157.201.194.${args[$i]}"
  #sleep 1
  #echo ""
done
sleep 1