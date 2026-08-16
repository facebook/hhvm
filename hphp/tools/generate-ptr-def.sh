#!/bin/bash

mode="${2:-}"

cat <<EOF > "${INSTALL_DIR}/ptr-def.h"
#pragma once

EOF

if [[ "$mode" == "--low" || "$mode" == "--packed" ]]; then
  cat <<EOF >> "${INSTALL_DIR}/ptr-def.h"
#ifndef USE_PACKEDPTR
#define USE_PACKEDPTR 1
#endif

EOF
fi
