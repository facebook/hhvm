cd test/server
g++ -g -I. -I$ICE_HOME/include -c *.cpp
g++ -g -o server class_demo.o Server.o class_demoI.o  -L$ICE_HOME/lib64 -lIce -lIceUtil -liconv

run server ./server

php client/client.php
hhvm client/client.php