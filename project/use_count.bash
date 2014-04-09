#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 port numComputers"
  echo "Requires SSH_PORT to be set in the environment"
  return
fi

make server_leaf

ssh_options="-o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no"
# ConnectTimeout: fail after 10 seconds without response - seems to be iffy, use 'timeout' command and chain
# BatchMode: Automatic 'yes' to add to known hosts
# StrictHostKeyChecking: Auto add the fingerprint

echo "Killing servers...."
(cleanup_servers_failsafe.bash > /dev/null)

args=("$@")
port=${args[0]}
if [ $port -le 1024 ]; then
   echo "Using 10021 for the port...."
   args[1]=$port
   port=10021
   args[0]=10021
fi
if [ ${args[1]} -gt 34 ]; then
   args[1]=34
fi
ip_start="$(hostname -i | grep -E -o -e "^([0-9]{1,3}\.){3}")"
this_comp=$(hostname -i)
# Full computer listing
for ((i = 1; i <= 35; i+=1))do
   let num=$i+200
   computers[$i]="$ip_start$num"
done
# Computer preference order....
#            1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
comp_pref=(  7  8 18 20 26 27 28  6  9 10 19 29 30 32 34 35  5 14 15 23 24 31 33 16 17 22 25 21  1  2  3  4 11 12 13 )
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

export BG_COMPUTERS="$comp_list"
export BG_PORT="$port"
export BG_COUNT="${args[1]}"

for i in ${computers2[@]}; do
  echo "System $i:"
  ((echo "nohup ~/cs499/project/server_leaf $port >/dev/null 2>&1 &"; exit) | timeout 10s ssh -T -p $SSH_PORT $ssh_options $i) &
done
# Clean up the variables to not leave them as artifacts in the shell environment
unset ssh_options
unset ip_start
unset args
unset port
unset this_comp
unset num
unset computers
unset comp_pref
unset count
unset comp_list
unset temp
unset computers2
unset i

# Allow the servers to all start up
sleep 1
