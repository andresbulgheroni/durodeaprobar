#!/bin/bash

while getopts cvi option
do
case "${option}"
in
c)clear
echo -e "\nCompilando mongo...\n"

cd i_Mongo_Store
make clean
make
cd ..
read -p "Presiona enter para finalizar";;

v)clear
echo -e "\nCorriendo i_Mongo_Store con VALGRIND...\n"
cd i_Mongo_Store
cd bin 
valgrind --leak-check=full --log-file=vlgnd_imongo_$(date '+%d-%m_%H:%M:%S').log ./i_Mongo_Store
cd ..
cd ..;;

i)clear
echo -e "\nIniciando i_Mongo_Store...\n"
cd i_Mongo_Store
cd bin 
./i_Mongo_Store
cd ..
cd ..;;

esac
done
