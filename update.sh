 #!/bin/sh
 
git pull
qmake -makefile
qmake -o build/makefile -Wall
make
