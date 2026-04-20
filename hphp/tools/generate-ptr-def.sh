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

cat <<EOF >> "${INSTALL_DIR}/ptr-def.h"
#ifdef __has_feature
 #if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
  #ifdef USE_PACKEDPTR
   #error "USE_PACKEDPTR is incompatible with sanitizer builds"
  #endif
 #endif
#endif

#if __SANITIZE_ADDRESS__ || __SANITIZE_THREAD__
 #ifdef USE_PACKEDPTR
  #error "USE_PACKEDPTR is incompatible with sanitizer builds"
 #endif
#endif
EOF
