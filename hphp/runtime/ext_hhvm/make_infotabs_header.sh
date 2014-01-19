#!/bin/sh

# Ignore the --foo arguments passed by fbconfig
shift; shift;

out_file=${INSTALL_DIR}/$1
shift
if [ ! -d `dirname $out_file` ] ; then
    mkdir -p `dirname $out_file`
fi
cp /dev/null $out_file

while [ $# -gt 0 ] ; do
    header=`echo $1 | sed -e 's/.cpp\$/.h/'`
    echo "#include \"$header\"" >> $out_file
    shift
done
