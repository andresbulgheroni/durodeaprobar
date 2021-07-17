#!/bin/bash

cd mi_Ram_Hq
cd Debug
rm valgrind.log
valgrind --leak-check=full --log-file=valgrind.log ./mi_Ram_Hq
