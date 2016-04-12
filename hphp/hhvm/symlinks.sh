#/bin/sh

if echo $1 | grep -q -e 'install_dir=' -e 'fbcode_dir=' ; then
  # Skip --install_dir and --fbcode_dir.
  shift 2
fi

HHVM_BIN=$1

ln -sf "${HHVM_BIN}" "${INSTALL_DIR}/hphp"
ln -sf "${HHVM_BIN}" "${INSTALL_DIR}/php"
ln -sf "${HHVM_BIN}" "${INSTALL_DIR}/hhbbc"
