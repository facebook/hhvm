#!/usr/bin/env bash

set -euo pipefail

top="$(pwd)"
if [ "$(echo "$top" | grep -c "fbsource/fbcode\$")" -ne 1 ]
  then
    echo "This script should be run from fbsource/fbcode."
    exit 1
fi

if ! command -v "cbindgen" &> /dev/null
then
    echo "It seems cbindgen is not in the PATH."
    echo "Try 'cargo --force install cbindgen' and then run this script again."
    echo "(You may need to 'export https_proxy=http://fwdproxy:8080' to install cbindgen)."
    exit 2
fi

set -x

# ffi.h
(cd hphp/hack/src/utils/ffi&& \
     cbindgen --config ./cbindgen_ffi.toml \
              --crate ffi \
              --output "$top/hphp/hack/src/utils/ffi/ffi.h")

# hhbc_ast.h (includes ffi.h)
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhbc_ast && \
     cbindgen --config ../../cbindgen_hhbc_ast.toml \
              --crate hhbc_by_ref_hhbc_ast \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h")

signscript="$top/../xplat/python/signedsource_lib/signedsource.py"
eval "${signscript}" sign "${top}"/hphp/hack/src/utils/ffi/ffi.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h

# Quick sanity check: Does a program that includes these headers compile?
cat > main.cpp <<EOF
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h"

#include <iostream>

int main() {
  std::cout << "Ok!" << std::endl;
  return 0;
}
EOF
g++ -std=c++14 main.cpp -I . -o run && ./run
rm -f ./main.cpp ./run
