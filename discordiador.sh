#!/bin/bash

cd discordiador
cd Debug
rm valgrind.log
valgrind --leak-check=full --log-file=valgrind.log ./discordiador
