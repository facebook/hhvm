#!/bin/sh

OBJDUMP=`which objdump`
if [ ! -x "${OBJDUMP}" ]; then
  OBJDUMP=`which gobjdump`
fi

if [ ! -x "${OBJDUMP}" ]; then
  echo "objdump utility not found" 2>&1
  exit 1
fi

if [ -z "$1" ]; then
  echo "Usage: $0 <hhvm binary> [section name]" 2>&1
  exit 1
fi

if [ -z "$2" ]; then
  SECTNAME=systemlib
else
  DIRNAME=`dirname $0`
  SECTNAME=`${DIRNAME}/section_name.sh $2`
fi

${OBJDUMP} -s -j ${SECTNAME} $1
