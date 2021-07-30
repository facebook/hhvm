// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<6b0a06c31b26f332ba4508626afa4225>>


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

struct HhasConstant {
  ConstType name;
  Maybe<TypedValue> value;
  Maybe<InstrSeq> initializer_instrs;
  bool is_abstract;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhas_constant(HhasConstant);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
