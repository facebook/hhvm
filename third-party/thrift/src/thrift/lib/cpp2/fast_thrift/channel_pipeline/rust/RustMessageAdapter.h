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

#include <memory>
#include <optional>
#include <type_traits>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace channel_pipeline_rust {

/**
 * RustMessageAdapter concept — defines how a C++ pipeline message type
 * moves across the FFI boundary to Rust and back.
 *
 * The message type itself is the identity — there is NO numeric type id and
 * NO central enum. Each Rust-supported message type must specialize this
 * template with:
 * - CppType: the C++ pipeline message type
 * - tryTake(TypeErasedBox&&) -> optional<CppType>: fallible ownership transfer
 * - tryRestore(TypeErasedBox&, CppType&&) -> bool: restore the original box
 *   after Rust returns the original message representation
 * - tryBox(CppType&&) -> optional<TypeErasedBox>: box a replacement or
 *   fan-out message
 * - box(CppType&&) -> TypeErasedBox: infallible boxing (DCHECK non-null)
 *
 * Design invariants:
 * - tryTake fallible via optional catching TypeErasedBox mismatch (empty box
 *   or wrong type) — mirrors debug-mode TypeMismatch exception, opt builds
 *   rely on single-message-type-per-layer invariant.
 * - tryBox null rejection: null UniquePtr -> nullopt, prevents null crossing.
 * - RustMessageAdapterConcept enforces a CppType satisfying TypeErasedBox's
 *   inline contract and exact tryTake/tryRestore/tryBox signatures.
 * - No runtime registry: compile-time template specialization per type, zero
 *   cost for native C++ pipelines (pipeline_impl has no Rust/CXX edge).
 * - Single-message-type-per-layer preserved: adapters convert at handler
 *   boundary, TypeErasedBox itself never crosses FFI.
 * - Inline C++ message types must satisfy TypeErasedBox's 120-byte,
 *   pointer-alignment, and nothrow-move constraints; otherwise use opaque
 *   UniquePtr or explicitly serialized representation.
 * - Never mirror ABI-unstable C++ or Thrift generated layout in Rust.
 */
template <typename T>
struct RustMessageAdapter;

// BytesPtr adapter — zero-copy move of unique_ptr<folly::IOBuf>
template <>
struct RustMessageAdapter<
    apache::thrift::fast_thrift::channel_pipeline::BytesPtr> {
  using CppType = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  // Rust side uses cxx::UniquePtr<folly::IOBuf> via folly/rust/iobuf crate
  // The actual Rust type is opaque; C++ side just passes UniquePtr through.

  // In optimized builds, callers must uphold TypeErasedBox's message-type
  // invariant before calling tryTake; get/take intentionally carry no RTTI.
  static std::optional<CppType> tryTake(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          box) noexcept {
    try {
      auto value = box.take<CppType>();
      if (!value) {
        return std::nullopt;
      }
      return value;
    } catch (...) {
      return std::nullopt;
    }
  }

  static bool tryRestore(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox& box,
      CppType&& value) noexcept {
    if (!value || !box.empty()) {
      return false;
    }
    box = apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
        std::move(value));
    return true;
  }

  static apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox box(
      CppType&& value) noexcept {
    DCHECK(value);
    return apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
        std::move(value));
  }

  static std::optional<
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
  tryBox(CppType&& value) noexcept {
    if (!value) {
      return std::nullopt;
    }
    return box(std::move(value));
  }
};

/**
 * Concept check for conforming adapters.
 */
template <typename Adapter>
concept RustMessageAdapterConcept =
    requires {
      typename Adapter::CppType;
      typename Adapter::CppType;
    } &&
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox::fits_inline<
        typename Adapter::CppType>() &&
    requires(
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox& box,
        typename Adapter::CppType&& cxx_value) {
      {
        Adapter::tryTake(std::move(box))
      } -> std::same_as<std::optional<typename Adapter::CppType>>;
      { Adapter::tryRestore(box, std::move(cxx_value)) } -> std::same_as<bool>;
      {
        Adapter::tryBox(std::move(cxx_value))
      } -> std::same_as<std::optional<
          apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>>;
    };

static_assert(RustMessageAdapterConcept<RustMessageAdapter<
                  apache::thrift::fast_thrift::channel_pipeline::BytesPtr>>);

/**
 * Extension patterns:
 *
 * - C-compatible POD: declare an intentionally stable shared CXX type and keep
 *   the C++ pipeline value within TypeErasedBox's 120-byte, alignment, and
 *   nothrow-move constraints.
 * - Opaque C++ object: box a unique_ptr and expose behavior through CXX
 * methods; never share the C++ layout with Rust.
 * - Serialized Thrift value: serialize to IOBuf with cxx-thrift-utils in take()
 *   and deserialize in box(). This is intentionally not implemented until a
 *   pipeline layer actually uses a Thrift struct message; benchmark it
 *   separately from the zero-copy BytesPtr adapter when introduced.
 */

} // namespace channel_pipeline_rust
