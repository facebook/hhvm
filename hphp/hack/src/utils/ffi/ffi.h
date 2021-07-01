// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<15f6f64f07cf422b1077036557b15735>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>


namespace HPHP {
namespace hackc {

/// A type to substitute for `&'a[T]`.
template<typename T>
struct Slice {
  const T *data;
  size_t len;
};

/// An alias for a type that substitutes for `&'str`.
using Str = Slice<uint8_t>;


extern "C" {

void ffi_07(Str);

} // extern "C"

} // namespace hackc
} // namespace HPHP
