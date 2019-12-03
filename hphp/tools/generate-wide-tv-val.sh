#!/bin/bash

shift

cat <<GUARD > "${INSTALL_DIR}/wide-tv-val-def.h"
#ifndef incl_HPHP_WIDE_TV_VAL_DEF_H_
#define incl_HPHP_WIDE_TV_VAL_DEF_H_

GUARD

if [[ $1 == "--wide" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/wide-tv-val-def.h"
#ifndef HHVM_WIDE_TV_VAL
#define HHVM_WIDE_TV_VAL 1
#endif

EOF

fi

cat <<TAIL >> "${INSTALL_DIR}/wide-tv-val-def.h"

#endif
TAIL
