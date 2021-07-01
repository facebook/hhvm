// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<b30e387bd819b05d741c9e6905f3b41d>>


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

struct Dummy {
  Slice<int32_t> _0;
};


extern "C" {

void hhbc_ast_07(CheckStarted, FreeIterator, FcallFlags, Dummy);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
