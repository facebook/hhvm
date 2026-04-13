#!/bin/bash

shift

cat <<EOF > "${INSTALL_DIR}/jemalloc-def.h"
#pragma once

EOF

if [[ $1 == "--jemalloc" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/jemalloc-def.h"
#ifndef USE_JEMALLOC
#define USE_JEMALLOC 1
#endif

EOF
fi
