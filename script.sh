#!/bin/bash

while getopts cmid option
do
case "${option}"
in
c) echo "Comenzando deploy..."
echo "Descargando dependencias"
sudo apt-get install libncurses5-dev
git submodule init 
git submodule update

read -p "Presiona enter para compilar"
echo "Compilando...";;

m) echo "Iniciando mi_Ram_Hq...";;

i) echo "Iniciando i_Mongo_Store...";;

d) echo "Iniciando discordiador...";;

esac
done
