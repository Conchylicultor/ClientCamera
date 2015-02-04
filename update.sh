 #!/bin/sh
 
git pull
rm -rf build/
qmake -makefile -o build/makefile -Wall
cd build/
make
