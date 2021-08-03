#!/usr/bin/env bash

set -euo pipefail

top="$(pwd)"
if [ "$(echo "$top" | grep -c "fbcode\$")" -ne 1 ]
  then
    echo "This script should be run from fbsource/fbcode."
    exit 1
fi

if ! command -v "cargo" &> /dev/null
then
    echo "It seems cargo is not in the PATH."
    echo " Try: 'export PATH=/data/users/${USER}/fbsource/fbcode/third-party-buck/platform009/build/rust/bin:\$PATH'"
    read -r -p " Perform these steps? [y/n] " input
    case $input in
        [yY][eE][sS]|[yY])
        echo " Perfoming steps"
        export PATH="/data/users/${USER}/fbsource/fbcode/third-party-buck/platform009/build/rust/bin:$PATH"
        ;;
    *)
        echo " No steps performed. Exiting."
        exit 2
        ;;
    esac
fi

if [ "$(sudo feature installed | grep -c ttls_fwdproxy)" -ne 1 ]
then
    echo "ttls_fwdproxy is not installed."
    echo "Try: 'sudo feature install ttls_fwdproxy'"
    read -r -p " Perform these steps? [y/n] " input
    case $input in
        [yY][eE][sS]|[yY])
        echo " Perfoming steps"
        sudo feature install ttls_fwdproxy
        ;;
    *)
        echo " No steps performed. Exiting."
        exit 2
        ;;
    esac
fi

DIR=hphp/hack/.cargo_vendor
if [ ! -d "$DIR" ];
then
    echo "Need to generate .cargo_vendor"
    echo " Try 'hphp/hack/scripts/facebook/cargo_fetch.sh'"
    read -r -p " Perform these steps? [y/n] " input
    case $input in
        [yY][eE][sS]|[yY])
        echo " Perfoming steps"
        hphp/hack/scripts/facebook/cargo_fetch.sh
        ;;
    *)
        echo " No steps performed. Exiting."
        exit 2
        ;;
    esac
fi

# These are sometimes needed to properly install cbindgen
if [ "$(git config --list | grep -c http.proxy)" -ne 1 ]
then  git config --global  http.proxy http://fwdproxy:8080
fi
if [ "$(git config --list | grep -c https.proxy)" -ne 1 ]
then  git config --global  https.proxy https://fwdproxy:8080
fi

if ! command -v "cbindgen" &> /dev/null
then
    echo "It seems cbindgen is not in the PATH."
    echo " Try: 'cargo install --force cbindgen'"
    echo "  (You may need to 'export https_proxy=http://fwdproxy:8080' to install cbindgen)."
    echo "  (You may need to 'export PATH=${HOME}/.cargo/bin:\$PATH' to be able to run the installed binaries)."
    read -r -p " Perform these steps? [y/n] " input
    case $input in
        [yY][eE][sS]|[yY])
        echo " Perfoming steps"
        export https_proxy=http://fwdproxy:8080
        cargo install --force cbindgen
        export PATH="${HOME}/.cargo/bin:$PATH"
        ;;
    *)
        echo " No steps performed. Exiting."
        exit 2
        ;;
    esac
fi

set -x

# 'cargo update' ensures the existing Cargo.lock files contain the latest versions

# ffi.h
(cd hphp/hack/src/utils/ffi && \
     cargo update && \
     cbindgen --config ./cbindgen_ffi.toml \
              --crate ffi \
              --output "$top/hphp/hack/src/utils/ffi/ffi.h")

#  hhbc_id.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhbc_id && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_id.toml \
              --crate hhbc_by_ref_hhbc_id \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.h")

#  hhbc_label.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_label && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_label.toml \
              --crate hhbc_by_ref_label \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_label.h")

#  hhbc_local.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_local && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_local.toml \
              --crate hhbc_by_ref_local \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_local.h")

#  hhbc_runtime.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_runtime && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_runtime.toml \
              --crate hhbc_by_ref_runtime \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h")

# hhbc_ast.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhbc_ast && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_ast.toml \
              --crate hhbc_by_ref_hhbc_ast \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h")

# hhbc_instruction_sequence.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_instruction_sequence && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_instruction_sequence.toml \
              --crate hhbc_by_ref_instruction_sequence \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_instruction_sequence.h")

# hhbc_symbol_refs_state.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_symbol_refs_state && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_symbol_refs_state.toml \
              --crate hhbc_by_ref_symbol_refs_state \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_symbol_refs_state.h")

# hhbc_hhas_symbol_refs.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhas_symbol_refs && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_hhas_symbol_refs.toml \
              --crate hhbc_by_ref_hhas_symbol_refs \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_symbol_refs.h")

# hhbc_hhas_constant.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhas_constant && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_hhas_constant.toml \
              --crate hhbc_by_ref_hhas_constant \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_constant.h")

# hhbc_hhas_type.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhas_type && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_hhas_type.toml \
              --crate hhbc_by_ref_hhas_type \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_type.h")
# hhbc_hhas_attribute.h
(cd hphp/hack/src/hhbc/hhbc_by_ref/cargo/hhbc_by_ref_hhas_attribute && \
     cargo update && \
     cbindgen --config ../../cbindgen_hhbc_hhas_attribute.toml \
              --crate hhbc_by_ref_hhas_attribute \
              --output "$top/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_attribute.h")

signscript="$top/../xplat/python/signedsource_lib/signedsource.py"
eval "${signscript}" sign "${top}"/hphp/hack/src/utils/ffi/ffi.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_label.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_local.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_instruction_sequence.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_symbol_refs_state.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_symbol_refs.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_constant.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_type.h
eval "${signscript}" sign "${top}"/hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_attribute.h

# Quick sanity check: Does a program that includes these headers compile?
cat > main.cpp <<EOF
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_instruction_sequence.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_symbol_refs.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_constant.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_type.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_hhas_attribute.h"

#include <iostream>

int main() {
  using namespace HPHP::hackc::hhbc::ast;

  InstrSeq _b6;
  HhasSymbolRefs _b7;
  HhasConstant _b8;
  HhasAttribute _b9;
  Info _c1;

  std::cout << "Ok!" << std::endl;
  return 0;
}
EOF
g++ -std=c++14 main.cpp -I . -o run && ./run
rm -f ./main.cpp ./run
