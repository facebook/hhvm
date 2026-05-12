#!/bin/bash

shift

cat <<EOF > "${INSTALL_DIR}/ptr-def.h"
#pragma once

EOF

if [[ $1 == "--low" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/ptr-def.h"
#ifndef USE_PACKEDPTR
#define USE_PACKEDPTR 1
#endif

EOF
fi

if [[ $1 == "--packed" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/ptr-def.h"
#ifndef USE_PACKEDPTR
#define USE_PACKEDPTR 1
#endif

EOF
fi
