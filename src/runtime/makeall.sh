#/bin/sh
# called by src/

if [ $1 == "SHARED" ]; then
  SHARED=1 make -kj -C runtime/tmp all
else
  COMPILE=1 make -kj -C runtime/tmp all
  LINK=1 make -kj10 -C runtime/tmp all
fi
