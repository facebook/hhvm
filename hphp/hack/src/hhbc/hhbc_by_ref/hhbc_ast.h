// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<3a72b17327134c00cf1fc7aba506badf>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/utils/ffi/ffi.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_label.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_local.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

enum class CheckStarted {
  IgnoreStarted,
  CheckStarted,
};

enum class FreeIterator {
  IgnoreIter,
  FreeIter,
};

struct FcallFlags {
  uint8_t bits;

  explicit operator bool() const {
    return !!bits;
  }
  FcallFlags operator~() const {
    return {static_cast<decltype(bits)>(~bits)};
  }
  FcallFlags operator|(const FcallFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits | other.bits)};
  }
  FcallFlags& operator|=(const FcallFlags& other) {
    *this = (*this | other);
    return *this;
  }
  FcallFlags operator&(const FcallFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits & other.bits)};
  }
  FcallFlags& operator&=(const FcallFlags& other) {
    *this = (*this & other);
    return *this;
  }
  FcallFlags operator^(const FcallFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits ^ other.bits)};
  }
  FcallFlags& operator^=(const FcallFlags& other) {
    *this = (*this ^ other);
    return *this;
  }
};
static const FcallFlags FcallFlags_HAS_UNPACK = FcallFlags{ /* .bits = */ (uint8_t)1 };
static const FcallFlags FcallFlags_HAS_GENERICS = FcallFlags{ /* .bits = */ (uint8_t)2 };
static const FcallFlags FcallFlags_LOCK_WHILE_UNWINDING = FcallFlags{ /* .bits = */ (uint8_t)4 };

using ParamNum = ptrdiff_t;

using StackIndex = ptrdiff_t;

using RecordNum = ptrdiff_t;

using TypedefNum = ptrdiff_t;

using ClassNum = ptrdiff_t;

using ConstNum = ptrdiff_t;

using ClassId = Type;

using FunctionId = Type;

using MethodId = Type;

using ConstId = Type;

using PropId = Type;

using NumParams = size_t;

using ByRefs = Slice<bool>;

struct FcallArgs {
  FcallFlags _0;
  NumParams _1;
  NumParams _2;
  ByRefs _3;
  Maybe<Label> _4;
  Maybe<Str> _5;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_ast(CheckStarted,
                                              FreeIterator,
                                              FcallFlags,
                                              ParamNum,
                                              StackIndex,
                                              RecordNum,
                                              TypedefNum,
                                              ClassNum,
                                              ConstNum,
                                              ClassId,
                                              FunctionId,
                                              MethodId,
                                              ConstId,
                                              PropId,
                                              FcallArgs);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
