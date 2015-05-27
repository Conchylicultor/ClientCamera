#!/bin/sh
 
git pull
rm -rf build/
qmake -makefile -Wall
cd build/
make
