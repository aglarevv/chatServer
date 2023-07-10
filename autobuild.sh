 #!/bin/bash
     
 set -x
 mkdir build && 
 rm -rf `pwd`/build/*
 cd `pwd`/build &&
   cmake .. &&
   make

