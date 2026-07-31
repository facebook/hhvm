/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

// Minimal CXX surface for the Rust `RustTypeErasedBox`. The box lives on the
// stack and the Rust handler BORROWS it (Pin<&mut TypeErasedBox>); the typed
// take happens entirely on the Rust side (a zero-copy `unsafe` relocate out of
// the inline storage), mirroring C++ `TypeErasedBox::take<T>()`. C++ therefore
// exposes no take thunk here -- only `reset` (used after the Rust relocate to
// flip the box to empty) and `is_empty`. There is no
// per-type code: the take is fully generic on the Rust side.

#include <typeinfo>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace channel_pipeline_rust {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

// Dev-mode type check for BytesPtr, mirroring C++ `take<T>()`'s `checkAccess`.
// In dev builds it compares the box's stored `type_info` to `typeid(BytesPtr)`;
// in release there is no type tag, so it returns true and the caller's relocate
// reinterprets (crash/UB on the wrong type) -- the accepted price of this
// pipeline. This is the one small per-type piece: one such thunk per message
// type, exactly like the per-type C++ `RustMessageAdapter<T>` specialization.
inline bool rust_teb_holds_bytes(const TypeErasedBox& box) noexcept {
#ifndef NDEBUG
  const std::type_info* stored = box.storedType();
  return stored != nullptr && *stored == typeid(BytesPtr);
#else
  (void)box;
  return true;
#endif
}

// True when the box holds no value.
inline bool rust_teb_is_empty(const TypeErasedBox& box) noexcept {
  return box.empty();
}

// Flip the box to the empty state. Called by `RustTypeErasedBox::take` AFTER
// the value has been relocated out and its source bytes zeroed on the Rust
// side, so the destroy hook runs on a null/empty value (a no-op) -- this just
// marks the box empty, exactly like the `reset()` inside C++ `take<T>()`.
inline void rust_teb_reset(TypeErasedBox& box) noexcept {
  box.reset();
}

} // namespace channel_pipeline_rust
