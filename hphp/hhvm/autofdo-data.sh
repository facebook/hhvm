#!/bin/sh

if echo "$1" | grep -q -e "install_dir=" -e "fbcode_dir=" ; then
  # Skip --install_dir and --fbcode_dir.
  shift 1
fi
if echo "$1" | grep -q -e "install_dir=" -e "fbcode_dir=" ; then
  # Skip --install_dir and --fbcode_dir.
  shift 1
fi

cp "$1" "${INSTALL_DIR}/data.afdo.bz2"
bunzip2 "${INSTALL_DIR}/data.afdo.bz2"
