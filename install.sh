#!/bin/bash

while getopts fdc option
do
case "${option}"
in
f) clear
echo -e "\nComenzando full deploy...\n"
echo -e "\nDescargando dependencias...\n"
sudo apt-get install libncurses5-dev

rm -rf so-commons-library
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
sudo make uninstall
sudo make install
cd ..

rm -rf so-nivel-gui-library
git clone https://github.com/sisoputnfrba/so-nivel-gui-library/
cd so-nivel-gui-library
make uninstall
make install
cd ..

read -p "Presiona enter para compilar mongo"
clear

cd i_Mongo_Store
make clean
make
cd ..
read -p "Presiona enter para compilar ram"
clear

cd mi_Ram_Hq
make clean
make
cd ..
read -p "Presiona enter para compilar discordiador"
clear

cd discordiador
make clean
make
cd ..
read -p "Presiona enter para finalizar";;

d)clear
echo -e "\nInstalando dependencias...\n"
sudo apt-get install libncurses5-dev

rm -rf so-commons-library
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
sudo make uninstall
sudo make install
cd ..

rm -rf so-nivel-gui-library
git clone https://github.com/sisoputnfrba/so-nivel-gui-library/
cd so-nivel-gui-library
make uninstall
make install
cd ..

read -p "Presiona enter para finalizar";;

c) clear
echo -e "\nComenzando compilacion...\n"

read -p "Presiona enter para compilar mongo"
clear

cd i_Mongo_Store
make clean
make
cd ..
read -p "Presiona enter para compilar ram"
clear

cd mi_Ram_Hq
make clean
make
cd ..
read -p "Presiona enter para compilar discordiador"
clear

cd discordiador
make clean
make
cd ..
read -p "Presiona enter para finalizar";;


esac
done