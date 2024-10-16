#!/bin/bash

/bin/echo -e "\n------------------------------------------------------------------------------------"
echo "------ PRA1. Processos, pipes i senyals: Primers -------"
/bin/echo -e  "\n--- Processos en execucio ---"
echo  "-----------------------------"
ps -eaf | grep "PPID" | grep -v "grep"
ps -eaf | grep "controlador \|calculador \|generador" | grep -v "grep"

/bin/echo -e "\n-----------------------------------------------------------------------------------"

echo "--- Llistat de PIPEs ---"
echo "------------------------"
lsof +c 15 -c controlador -c calculador -c generador 2>/dev/null | grep -i "FIFO\|COMMAND";
/bin/echo -e "\n-----------------------------------------------------------------------------------"
