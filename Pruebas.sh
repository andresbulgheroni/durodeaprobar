#!/bin/bash

rm -fv discordiador/*.log
rm -fv i_Mongo_Store/*.log
rm -fv mi_Ram_Hq/*.log
rm -rf /home/utnso/polus
rm -fv mi_Ram_Hq/swapFile.bin

cd i_Mongo_Store
make clean
make
cd bin 
lxterminal -e bash -c './i_Mongo_Store; bash'
cd ..
cd ..

cd mi_Ram_Hq
make clean
make
cd bin
lxterminal -e bash -c './mi_Ram_Hq; bash'
cd ..
cd ..

cd discordiador
make clean
make
cd bin
lxterminal -e bash -c './discordiador; bash'

esac
done