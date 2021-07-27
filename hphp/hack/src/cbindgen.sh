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
    echo "Try 'cargo install --force cbindgen' and then run this script again."
    echo "(You may need to 'export https_proxy=http://fwdproxy:8080' to install cbindgen)."
    exit 2
fi

set -x

# ffi.h
(cd hphp/hack/src/utils/ffi&& \
     cbindgen --config ./cbindgen_ffi.toml \
              --crate ffi \
              --output "$top/hphp/hack/src/utils/ffi/ffi.h")

#  hhbc_id.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhbc_id && \
     cbindgen --config ../../cbindgen_hhbc_id.toml \
              --crate hhbc_by_ref_hhbc_id \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.h")

#  hhbc_label.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_label && \
     cbindgen --config ../../cbindgen_hhbc_label.toml \
              --crate hhbc_by_ref_label \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_label.h")

#  hhbc_local.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_local && \
     cbindgen --config ../../cbindgen_hhbc_local.toml \
              --crate hhbc_by_ref_local \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_local.h")

#  hhbc_local.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_runtime && \
     cbindgen --config ../../cbindgen_hhbc_runtime.toml \
              --crate hhbc_by_ref_runtime \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h")

# hhbc_ast.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhbc_ast && \
     cbindgen --config ../../cbindgen_hhbc_ast.toml \
              --crate hhbc_by_ref_hhbc_ast \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h")

# hhbc_instruction_sequence.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_instruction_sequence && \
     cbindgen --config ../../cbindgen_hhbc_instruction_sequence.toml \
              --crate hhbc_by_ref_instruction_sequence \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_instruction_sequence.h")

signscript="$top/../xplat/python/signedsource_lib/signedsource.py"
eval "${signscript}" sign "${top}"/hphp/hack/src/utils/ffi/ffi.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_label.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_local.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_instruction_sequence.h

# Quick sanity check: Does a program that includes these headers compile?
cat > main.cpp <<EOF
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_instruction_sequence.h"

#include <iostream>

int main() {
  using namespace HPHP::hackc::hhbc::ast;

  InstrSeq _b6;

  std::cout << "Ok!" << std::endl;
  return 0;
}
EOF
g++ -std=c++14 main.cpp -I . -o run && ./run
rm -f ./main.cpp ./run
