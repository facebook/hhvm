g++ -g -I. -I$ICE_HOME/include -c *.cpp
g++ -g -o server2 class_demo.o Server1.o class_demoI_info.o  -L$ICE_HOME/lib64 -lIce -lIceUtil -liconv