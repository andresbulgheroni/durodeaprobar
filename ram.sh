#!/bin/bash

while getopts cvi option
do
case "${option}"
in
c)clear
echo -e "\nCompilando mi_Ram_Hq...\n"

cd mi_Ram_Hq
make clean
make
cd ..
read -p "Presiona enter para finalizar";;

v)clear
echo -e "\nCorriendo mi_Ram_Hq con VALGRIND...\n"
cd mi_Ram_Hq
cd bin 
valgrind --leak-check=full --log-file=vlgnd_ram_$(date '+%d-%m_%H:%M:%S').log ./mi_Ram_Hq
cd ..
cd ..;;

i)clear
echo -e "\nIniciando mi_Ram_Hq...\n"
cd mi_Ram_Hq
cd bin 
./mi_Ram_Hq
cd ..
cd ..;;

esac
done
