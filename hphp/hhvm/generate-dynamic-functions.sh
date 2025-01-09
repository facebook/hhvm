#!/bin/bash

FILE="${OUT}"

if [ "$#" -ne 1 ]; then
    echo "Usage $0 on|off"
    exit 1
fi

cat > "$FILE" <<- EOM
{
EOM
if [ "$1" = "on" ]; then
cat >> "$FILE" <<- EOM
  extern "C++" {
    HPHP::type_scan::detail::Indexer*;
    *::_type_scan_custom_*;
  };
EOM
fi
cat >> "$FILE" <<- EOM
  extern "C" {
    hphp_type_scan_module_init;
    g_member_reflection_vtable_impl;
    /* functions defined by jemalloc */
    dallocx;
    mallctl;
    mallctlbymib;
    mallctlnametomib;
    malloc_stats_print;
    mallocx;
    nallocx;
    nallocx;
    rallocx;
    sallocx;
    sdallocx;
    xallocx;
  };
};
EOM
