// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#pragma once

// Forward declare bumpalo::Bump
struct Bump;
using BytesId = uint32_t;
using StringId = BytesId;

namespace HPHP::hackc {
  // OffsetArc<T> is defined to have the layout of const T*
  // https://docs.rs/triomphe/latest/triomphe/struct.OffsetArc.html
  template<typename T> using OffsetArc = const T*;
}
