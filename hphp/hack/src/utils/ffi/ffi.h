// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<89eebc4b18d334e9c8c8afdd55527156>>


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

/// Like `std::option`.
template<typename T>
struct Maybe {
  enum class Tag {
    Just,
    Nothing,
  };

  struct Just_Body {
    T _0;
  };

  Tag tag;
  union {
    Just_Body just;
  };
};

/// A tuple of two elements.
template<typename U, typename V>
struct Pair {
  U _0;
  V _1;
};

/// A type for an arena backed `&'a mut[T]`. Similar to `Slice<'a, T>`
/// but with mutable contents and an allocator reference (enabling
/// `Clone` support).
template<typename T>
struct BumpSliceMut {
  T *data;
  size_t len;
  size_t alloc;
};


extern "C" {

void no_call_compile_only_USED_TYPES_ffi(Str,
                                         Maybe<int32_t>,
                                         Pair<int32_t, int32_t>,
                                         BumpSliceMut<int32_t>);

} // extern "C"

} // namespace hackc
} // namespace HPHP
