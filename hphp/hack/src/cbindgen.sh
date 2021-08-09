#!/usr/bin/env bash

set -euo pipefail

top="$(pwd)"
if [ "$(echo "$top" | grep -c "fbcode\$")" -ne 1 ]
  then
    echo "This script should be run from fbsource/fbcode."
    exit 1
fi

set -x

buck run @mode/opt-clang //hphp/hack/src/utils:ffi_cbindgen -- \
     --header hhbc-ast.h \
     --namespaces HPHP,hackc,hhbc \
     --srcs \
hphp/hack/src/utils/ffi/ffi.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/label.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/local.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/typed_value.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/instruction_sequence.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/symbol_refs_state.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_symbol_refs.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_constant.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_type.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_attribute.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_adata.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_body.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_param.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_pos.rs,\
hphp/hack/src/hhbc/hhbc_by_ref/hhas_record_def.rs,

signscript="$top/../xplat/python/signedsource_lib/signedsource.py"
eval "${signscript}" sign "${top}"/hhbc-ast.h

# Quick sanity check: Does a program that includes these headers compile?
cat > main.cpp <<EOF
#include "hhbc-ast.h"

#include <iostream>

int main() {
  using namespace HPHP::hackc::hhbc;

  InstrSeq _b6;
  HhasSymbolRefs _b7;
  HhasConstant _b8;
  HhasAttribute _b9;
  Info _b10;
  HhasAdata _b11;
  HhasBody _b13;
  HhasParam _b14;
  Field _b15;
  Span _b16;

  std::cout << "Ok!" << std::endl;
  return 0;
}
EOF
g++ -std=c++14 main.cpp -I . -o run && ./run
rm -f ./main.cpp ./run
