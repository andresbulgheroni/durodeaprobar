#!/bin/bash

clear

echo "Descargando dependencias"
sudo apt-get install libncurses5-dev
git submodule init 
git submodule update

read -p "Presiona enter para compilar"
clear

echo "Compilando..."
