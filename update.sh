 #!/bin/sh
 
git pull
qmake -makefile -Wall
cd build/
make
