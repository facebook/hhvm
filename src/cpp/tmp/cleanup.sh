#/bin/sh
# called by src/
cd cpp/tmp
rm -fR -- *.php *.cpp *.d *.o *.h test php sys cls

# remove all the sub directories as well
for i in `ls`; do
  if [ -d $i ]; then
    rm -fR $i
  fi
done
