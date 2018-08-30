#!/bin/bash

shift

cat <<GUARD > "${INSTALL_DIR}/low-ptr-def.h"
#ifndef incl_HPHP_LOW_PTR_DEF_H_
#define incl_HPHP_LOW_PTR_DEF_H_

GUARD

if [[ $1 == "--low" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/low-ptr-def.h"
#ifndef USE_LOWPTR
#define USE_LOWPTR 1
#endif

EOF

fi

cat <<TAIL >> "${INSTALL_DIR}/low-ptr-def.h"
#ifdef __has_feature
 #if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
  #undef USE_LOWPTR
 #endif
#endif

#if __SANITIZE_ADDRESS__ || __SANITIZE_THREAD__
 #undef USE_LOWPTR
#endif

#endif
TAIL
