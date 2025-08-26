#!/bin/bash

shift

cat <<EOF > "${INSTALL_DIR}/low-ptr-def.h"
#pragma once

EOF

if [[ $1 == "--low" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/low-ptr-def.h"
#ifndef USE_LOWPTR
#define USE_LOWPTR 1
#endif

EOF
fi

cat <<EOF >> "${INSTALL_DIR}/low-ptr-def.h"
#ifdef __has_feature
 #if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
  #undef USE_LOWPTR
 #endif
#endif

#if __SANITIZE_ADDRESS__ || __SANITIZE_THREAD__
 #undef USE_LOWPTR
#endif

#ifndef USE_LOWPTR
 #undef USE_PACKEDPTR
#endif

EOF

if [[ $2 == "--packed" ]] ; then
    cat <<EOF >> "${INSTALL_DIR}/low-ptr-def.h"
#ifdef USE_LOWPTR
#define USE_PACKEDPTR 1
#endif

EOF
fi
