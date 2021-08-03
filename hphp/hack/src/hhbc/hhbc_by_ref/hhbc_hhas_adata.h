// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<f1128d49bd4ed5e636c9dee44bd4f5e9>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/utils/ffi/ffi.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

struct HhasAdata {
  Str id;
  TypedValue value;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhas_adata(HhasAdata);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
