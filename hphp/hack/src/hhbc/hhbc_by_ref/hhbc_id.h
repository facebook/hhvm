// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<b52a3f93ed9eeb1675080df4ab360d33>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/utils/ffi/ffi.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

struct ClassType {
  Str _0;
};

struct FunctionType {
  Str _0;
};

struct MethodType {
  Str _0;
};

struct PropType {
  Str _0;
};

struct ConstType {
  Str _0;
};

struct RecordType {
  Str _0;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_id(ClassType,
                                             FunctionType,
                                             MethodType,
                                             PropType,
                                             ConstType,
                                             RecordType);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
