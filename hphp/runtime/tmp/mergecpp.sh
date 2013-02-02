#/bin/sh
# called by src/

cd runtime/tmp/$1
cat *.cpp > temp.txt
rm *.cpp
mv temp.txt all.cpp
