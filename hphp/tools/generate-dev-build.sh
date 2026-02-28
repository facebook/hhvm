#!/bin/bash

shift

cat <<GUARD > "${INSTALL_DIR}/dev-build-def.h"
#ifndef incl_HPHP_DEV_BUILD_DEF_H_
#define incl_HPHP_DEV_BUILD_DEF_H_

GUARD

if [[ $1 == "--dev-build" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/dev-build-def.h"
#ifndef USE_DEVBUILD
#define USE_DEVBUILD 1
#endif

EOF

fi

cat <<TAIL >> "${INSTALL_DIR}/dev-build-def.h"
#endif

TAIL
