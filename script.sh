#!/bin/bash

while getopts cmid option
do
case "${option}"
in
c) echo -e "\nComenzando deploy...\n"
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

read -p "Presiona enter para compilar"
echo -e "\nCompilando...\n"

cd i_Mongo_Store
make clean
make
cd ..
read -p "Presiona enter para continuar"

cd mi_Ram_Hq
make clean
make
cd ..
read -p "Presiona enter para continuar"

cd discordiador
make clean
make
cd ..
read -p "Presiona enter para continuar";;

m) echo "Iniciando mi_Ram_Hq..."
cd mi_Ram_Hq
cd bin 
./mi_Ram_Hq;;

i) echo "Iniciando i_Mongo_Store..."
cd i_Mongo_Store
cd bin 
./i_Mongo_Store;;

d) echo "Iniciando discordiador..."
cd discordiador
cd bin 
./discordiador;;

esac
done
