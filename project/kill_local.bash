#!/bin/bash

servCount=$(ps -eLf | grep -v "grep" | grep -c "server_leaf")
pkill server_leaf
echo "servers on $(hostname -i): $servCount $(ps -eLf | grep -v "grep" | grep -c "server_slave")"
