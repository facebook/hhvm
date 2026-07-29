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

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace channel_pipeline_rust {

/**
 * Stable numeric type IDs for Rust-supported message types.
 *
 * These IDs cross the FFI boundary and must remain stable across versions.
 * New IDs must be added at the end (append-only); never reuse or reorder
 * existing IDs. This preserves wire compatibility for Rust/C++ interop and
 * mirrors the native EventEnum append-only contract (Count sentinel gives
 * storage size).
 *
 * Phase 6 audit (/tmp/phase6-audit.md): native pipelines DO use a second
 * message type ParsedFrame/ComposedFrame via FrameCodecHandler, but no
 * concrete synchronous Rust handler in the repo consumes it. Rust bridge
 * tests, benches, and production handlers only exercise BytesPtr. Therefore
 * only BytesPtr=1 is registered; second adapter intentionally not added now.
 * BytesPtr remains the zero-copy sole production adapter.
 */
enum class RustMessageTypeId : uint32_t {
  kBytesPtr = 1,
};

/**
 * Whether a numeric type ID is registered for Rust interop.
 *
 * Only BytesPtr=1 is registered today. Unregistered IDs are rejected
 * (return false) — fallible conversion path, no runtime registry, O(1)
 * compile-time check. Mirrors Rust side RustMessageTypeId::BytesPtr=1.
 */
constexpr bool isRegisteredRustMessageTypeId(uint32_t typeId) noexcept {
  return typeId == static_cast<uint32_t>(RustMessageTypeId::kBytesPtr);
}

/**
 * RustMessageAdapter concept — defines how a C++ pipeline message type
 * moves across the FFI boundary to Rust and back.
 *
 * Each Rust-supported message type must specialize this template with:
 * - kTypeId: stable numeric ID exposed to Rust (append-only, never reuse)
 * - CppType: the C++ pipeline message type
 * - tryTake(TypeErasedBox&&) -> optional<CppType>: fallible via
 *   std::optional, catches TypeErasedBox mismatch (empty/wrong-type) via
 *   try/catch and returns nullopt, plus null check for BytesPtr
 * - tryBox(CppType&&) -> optional<TypeErasedBox>: fallible, null rejection
 *   returns nullopt; non-null path boxes via erase_and_box (zero-copy move)
 * - box(CppType&&) -> TypeErasedBox: infallible helper DCHECKs non-null
 *
 * Design invariants (Phase 6, Step3):
 * - Stable IDs append-only: new IDs added at end, never reordered/reused.
 * - Only BytesPtr=1 registered today; isRegisteredRustMessageTypeId rejects
 *   unregistered IDs — no runtime registry on native paths.
 * - tryTake fallible via optional catching TypeErasedBox mismatch (empty box
 *   or wrong type) — mirrors debug-mode TypeMismatch exception, opt builds
 *   rely on single-message-type-per-layer invariant.
 * - tryBox null rejection: null UniquePtr -> nullopt, prevents null crossing.
 * - Concept RustMessageAdapterConcept enforces kTypeId convertible to
 *   RustMessageTypeId, tryTake/tryBox signatures.
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
  static constexpr RustMessageTypeId kTypeId = RustMessageTypeId::kBytesPtr;
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
concept RustMessageAdapterConcept = requires(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&& box,
    typename Adapter::CppType&& cpp_value) {
  { Adapter::kTypeId } -> std::convertible_to<RustMessageTypeId>;
  {
    Adapter::tryTake(std::move(box))
  } -> std::same_as<std::optional<typename Adapter::CppType>>;
  {
    Adapter::tryBox(std::move(cpp_value))
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
