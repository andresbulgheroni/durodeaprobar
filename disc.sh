#!/bin/bash

while getopts cvi option
do
case "${option}"
in
c)clear
echo -e "\nCompilando discordiador...\n"

cd discordiador
make clean
make
cd ..
read -p "Presiona enter para finalizar";;

v)clear
echo -e "\nCorriendo discordiador con VALGRIND...\n"
cd discordiador
cd bin 
valgrind --leak-check=full --log-file=vlgnd_discordiador_$(date '+%d-%m_%H:%M:%S').log ./discordiador
cd ..
cd ..;;

i)clear
echo -e "\nIniciando discordiador...\n"
cd discordiador
cd bin 
./discordiador
cd ..
cd ..;;

esac
done
