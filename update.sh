 #!/bin/sh
 
git pull
qmake -makefile -o build/makefile -Wall
cd build/
make
