// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<df4d8acf007de6b42f51b66a285df6ef>>


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

struct ConstraintFlags {
  uint8_t bits;

  explicit operator bool() const {
    return !!bits;
  }
  ConstraintFlags operator~() const {
    return {static_cast<decltype(bits)>(~bits)};
  }
  ConstraintFlags operator|(const ConstraintFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits | other.bits)};
  }
  ConstraintFlags& operator|=(const ConstraintFlags& other) {
    *this = (*this | other);
    return *this;
  }
  ConstraintFlags operator&(const ConstraintFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits & other.bits)};
  }
  ConstraintFlags& operator&=(const ConstraintFlags& other) {
    *this = (*this & other);
    return *this;
  }
  ConstraintFlags operator^(const ConstraintFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits ^ other.bits)};
  }
  ConstraintFlags& operator^=(const ConstraintFlags& other) {
    *this = (*this ^ other);
    return *this;
  }
};
static const ConstraintFlags ConstraintFlags_NULLABLE = ConstraintFlags{ /* .bits = */ (uint8_t)1 };
static const ConstraintFlags ConstraintFlags_EXTENDED_HINT = ConstraintFlags{ /* .bits = */ (uint8_t)4 };
static const ConstraintFlags ConstraintFlags_TYPE_VAR = ConstraintFlags{ /* .bits = */ (uint8_t)8 };
static const ConstraintFlags ConstraintFlags_SOFT = ConstraintFlags{ /* .bits = */ (uint8_t)16 };
static const ConstraintFlags ConstraintFlags_TYPE_CONSTANT = ConstraintFlags{ /* .bits = */ (uint8_t)32 };
static const ConstraintFlags ConstraintFlags_DISPLAY_NULLABLE = ConstraintFlags{ /* .bits = */ (uint8_t)64 };
static const ConstraintFlags ConstraintFlags_UPPERBOUND = ConstraintFlags{ /* .bits = */ (uint8_t)128 };

struct Constraint {
  Maybe<Str> name;
  ConstraintFlags flags;
};

/// Type info has additional optional user type
struct Info {
  Maybe<Str> user_type;
  Constraint type_constraint;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhas_type(Info);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
