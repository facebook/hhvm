// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#pragma once

namespace HPHP::hackc {
// these are newtype structs in Rust
using BytesId = uint32_t;
using StringId = BytesId;

// OffsetArc<T> is defined to have the layout of const T*
// https://docs.rs/triomphe/latest/triomphe/struct.OffsetArc.html
template <typename T>
using OffsetArc = const T*;
} // namespace HPHP::hackc
