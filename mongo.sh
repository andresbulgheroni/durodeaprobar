#!/bin/bash

cd i_Mongo_Store
cd Debug
rm valgrind.log
valgrind --leak-check=full --log-file=valgrind.log ./i_Mongo_Store
