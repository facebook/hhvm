#!/bin/sh

cd ${HPHP_HOME}
COMPILER_ID=`git describe --all --long --abbrev=40 --always`
echo "#define COMPILER_ID \"${COMPILER_ID}\"" > src/runtime/base/compiler_id.h
