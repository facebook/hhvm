#!/bin/sh

cd ${HPHP_HOME}
COMPILER_ID=`git describe --all --long --abbrev=40 --always`
if [ -z "${COMPILER_ID}" ]; then
  # HipHop has been divorced from git,
  # define REPO_SCHEMA using current time
  # to play it safe
  COMPILER_ID="nongit/source-0-`date +%N%s`"
fi

echo "#define COMPILER_ID \"${COMPILER_ID}\"" > src/runtime/base/compiler_id.h
