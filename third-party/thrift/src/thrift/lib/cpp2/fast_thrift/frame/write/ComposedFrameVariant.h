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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>

#include <folly/io/IOBuf.h>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace apache::thrift::fast_thrift::frame {

namespace composed_frame_variant_detail {

template <typename T, typename... Us>
concept OneOf = (std::same_as<T, Us> || ...);

template <typename... Ts>
inline constexpr size_t maxSizeof = std::max({sizeof(Ts)...});

template <typename... Ts>
inline constexpr size_t maxAlignof = std::max({alignof(Ts)...});

} // namespace composed_frame_variant_detail

/**
 * Concept satisfied by every per-frame-type composed-frame struct in
 * ComposedFrame.h. Each composed frame provides:
 *
 *   - `T::kFrameType`     compile-time FrameType tag.
 *   - `t.streamId()`      stream this frame belongs to.
 *   - `std::move(t).serialize()`  produce wire bytes (consuming).
 *
 * `ComposedFrameVariant` accepts only types satisfying this concept.
 */
template <typename T>
concept ComposedFrameConcept = requires(T t, T&& moved) {
  { t.streamId() } noexcept -> std::same_as<uint32_t>;
  {
    std::move(moved).serialize()
  } -> std::same_as<std::unique_ptr<folly::IOBuf>>;
} && std::same_as<std::remove_cv_t<decltype(T::kFrameType)>, FrameType>;

/**
 * ComposedFrameVariant - A discriminated union of ComposedFrame types
 * that exposes the 3 common APIs (streamId / frameType / serialize) as
 * direct methods, with no `std::visit` at any call site.
 *
 * Why specialized vs a general variant:
 *   The 3 APIs are statically known. Fold-expression dispatch over the
 *   type pack lets the compiler inline the per-type branch at every call
 *   site — no function pointers, no vtables, no captured visitor lambdas.
 *   Per-instance size matches the underlying CompactVariant pattern
 *   (storage + 1-byte tag); the FrameType enum doubles as the
 *   discriminator, so frameType() is a direct field load with zero
 *   dispatch.
 *
 * Move-only. Default-constructed instances are valueless (tag is RESERVED).
 *
 * Usage:
 *   ComposedFrameVariant<ComposedRequestResponseFrame, ComposedSetupFrame> v
 *       = ComposedRequestResponseFrame{...};
 *   auto streamId = v.streamId();      // 1 inlined branch
 *   auto type     = v.frameType();     // direct field load
 *   auto bytes    = std::move(v).serialize();  // 1 inlined branch
 */
#pragma pack(push, 1)
template <ComposedFrameConcept... Ts>
class ComposedFrameVariant {
  static_assert(sizeof...(Ts) > 0);

  alignas(composed_frame_variant_detail::maxAlignof<Ts...>)
      std::byte storage_[composed_frame_variant_detail::maxSizeof<Ts...>];
  FrameType tag_{FrameType::RESERVED};

  template <size_t I>
  using AltAt = std::tuple_element_t<I, std::tuple<Ts...>>;

  template <size_t... Is>
  void destroyImpl(std::index_sequence<Is...>) noexcept {
    (void)((tag_ == AltAt<Is>::kFrameType &&
            (std::destroy_at(
                 std::launder(reinterpret_cast<AltAt<Is>*>(storage_))),
             true)) ||
           ...);
  }

  template <size_t... Is>
  void moveImpl(
      ComposedFrameVariant& src, std::index_sequence<Is...>) noexcept {
    (void)((tag_ == AltAt<Is>::kFrameType &&
            (std::construct_at(
                 reinterpret_cast<AltAt<Is>*>(storage_),
                 std::move(*std::launder(
                     reinterpret_cast<AltAt<Is>*>(src.storage_)))),
             true)) ||
           ...);
  }

  template <size_t... Is>
  uint32_t streamIdImpl(std::index_sequence<Is...>) const noexcept {
    uint32_t result = 0;
    (void)((tag_ == AltAt<Is>::kFrameType &&
            (result = std::launder(reinterpret_cast<const AltAt<Is>*>(storage_))
                          ->streamId(),
             true)) ||
           ...);
    return result;
  }

  template <size_t... Is>
  std::unique_ptr<folly::IOBuf> serializeImpl(std::index_sequence<Is...>) && {
    std::unique_ptr<folly::IOBuf> result;
    (void)((tag_ == AltAt<Is>::kFrameType &&
            (result = std::move(
                          *std::launder(reinterpret_cast<AltAt<Is>*>(storage_)))
                          .serialize(),
             true)) ||
           ...);
    return result;
  }

  void destroy() noexcept {
    if (tag_ != FrameType::RESERVED) {
      destroyImpl(std::index_sequence_for<Ts...>{});
      tag_ = FrameType::RESERVED;
    }
  }

 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  ComposedFrameVariant() noexcept = default;

  template <typename T>
    requires composed_frame_variant_detail::OneOf<std::decay_t<T>, Ts...>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  /* implicit */ ComposedFrameVariant(T&& value) noexcept {
    using D = std::decay_t<T>;
    std::construct_at(reinterpret_cast<D*>(storage_), std::forward<T>(value));
    tag_ = D::kFrameType;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  ComposedFrameVariant(ComposedFrameVariant&& o) noexcept : tag_(o.tag_) {
    if (tag_ != FrameType::RESERVED) {
      moveImpl(o, std::index_sequence_for<Ts...>{});
      o.destroy();
    }
  }

  ComposedFrameVariant& operator=(ComposedFrameVariant&& o) noexcept {
    if (this != &o) {
      destroy();
      tag_ = o.tag_;
      if (tag_ != FrameType::RESERVED) {
        moveImpl(o, std::index_sequence_for<Ts...>{});
        o.destroy();
      }
    }
    return *this;
  }

  ~ComposedFrameVariant() { destroy(); }

  ComposedFrameVariant(const ComposedFrameVariant&) = delete;
  ComposedFrameVariant& operator=(const ComposedFrameVariant&) = delete;

  template <typename T, typename... Args>
    requires composed_frame_variant_detail::OneOf<T, Ts...>
  T& emplace(Args&&... args) {
    destroy();
    auto* p = std::construct_at(
        reinterpret_cast<T*>(storage_), std::forward<Args>(args)...);
    tag_ = T::kFrameType;
    return *p;
  }

  template <typename T>
    requires composed_frame_variant_detail::OneOf<std::decay_t<T>, Ts...>
  ComposedFrameVariant& operator=(T&& value) noexcept {
    emplace<std::decay_t<T>>(std::forward<T>(value));
    return *this;
  }

  // === Discriminator queries ===

  bool valueless() const noexcept { return tag_ == FrameType::RESERVED; }

  template <typename T>
    requires composed_frame_variant_detail::OneOf<T, Ts...>
  bool is() const noexcept {
    return tag_ == T::kFrameType;
  }

  template <typename T>
    requires composed_frame_variant_detail::OneOf<T, Ts...>
  T& get() noexcept {
    return *std::launder(reinterpret_cast<T*>(storage_));
  }

  template <typename T>
    requires composed_frame_variant_detail::OneOf<T, Ts...>
  const T& get() const noexcept {
    return *std::launder(reinterpret_cast<const T*>(storage_));
  }

  // === The 3 typed APIs ===
  //
  // Each dispatches via inlined fold-expression branch on tag_; the
  // compiler can fully inline the per-type branch at the call site.
  // frameType() needs no dispatch — tag_ IS the FrameType.

  FrameType frameType() const noexcept { return tag_; }

  uint32_t streamId() const noexcept {
    return streamIdImpl(std::index_sequence_for<Ts...>{});
  }

  std::unique_ptr<folly::IOBuf> serialize() && {
    return std::move(*this).serializeImpl(std::index_sequence_for<Ts...>{});
  }
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::frame
