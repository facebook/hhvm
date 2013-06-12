#!/bin/sh

# $1 class_map cpp file to build
# $2 constants.h file to build
# $3 globals/constdef.json
# $4- Additional IDL files to parse for class_map

CLASSMAP=$1
CONSTANTS=$2
CONSTDEF=$3
shift; shift; shift;

$HPHP_HOME/hphp/tools/gen-ext-hhvm/gen-class-map \
    constants $CONSTANTS $CONSTDEF

$HPHP_HOME/hphp/tools/gen-ext-hhvm/gen-class-map \
    classmap $CLASSMAP $CONSTDEF $@

