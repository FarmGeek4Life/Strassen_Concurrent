#!/bin/bash

export BG_PORT="10021"
BG_COMPUTERS=""

for ((i = 1; i <= 35; i+=1))do
   BG_COMPUTERS+="$(printf -- '2%02i ' $i)"
done
echo "$BG_COMPUTERS"
. ./run_parallel.bash 10021 $BG_COMPUTERS
