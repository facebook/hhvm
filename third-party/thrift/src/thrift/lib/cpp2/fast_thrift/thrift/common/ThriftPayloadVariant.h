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

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrameVariant.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftControlPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadConcept.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftRequestPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftResponsePayloads.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace apache::thrift::fast_thrift::thrift {

namespace thrift_payload_variant_detail {

template <typename T, typename... Us>
concept OneOf = (std::same_as<T, Us> || ...);

template <typename... Ts>
inline constexpr size_t maxSizeof = std::max({sizeof(Ts)...});

template <typename... Ts>
inline constexpr size_t maxAlignof = std::max({alignof(Ts)...});

} // namespace thrift_payload_variant_detail

/**
 * ThriftPayloadVariant - A discriminated union of `ThriftPayloadConcept`
 * types that exposes a single `toRocketFrame()` accessor returning the
 * matching `ComposedFrameVariant<typename Ts::RocketFrame...>`.
 *
 * Why specialized vs a general variant:
 *   `toRocketFrame()` is the one operation every Thrift→Rocket dispatch
 *   does. Fold-expression dispatch over the type pack lets the compiler
 *   inline the per-type branch at every call site — no function pointers,
 *   no vtables, no captured visitor lambdas. The transport adapter
 *   simplifies from a runtime switch to a single `.toRocketFrame()` call.
 *
 * Per-instance size: storage + 1-byte index + 1-byte valueless flag.
 *
 * Move-only. Default-constructed instances are valueless.
 *
 * Usage:
 *   ThriftPayloadVariant<ThriftRequestResponsePayload, ...> v
 *       = ThriftRequestResponsePayload{...};
 *   auto rocketFrame = std::move(v).toRocketFrame();
 *     // → ComposedFrameVariant<ComposedRequestResponseFrame, ...>
 */
#pragma pack(push, 1)
template <ThriftPayloadConcept... Ts>
class ThriftPayloadVariant {
  static_assert(sizeof...(Ts) > 0);
  static_assert(sizeof...(Ts) <= 255);

  alignas(thrift_payload_variant_detail::maxAlignof<Ts...>)
      std::byte storage_[thrift_payload_variant_detail::maxSizeof<Ts...>];
  apache::thrift::RpcKind rpcKind_{
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  uint8_t index_{0};
  bool valueless_{true};

  template <size_t I>
  using AltAt = std::tuple_element_t<I, std::tuple<Ts...>>;

  template <size_t... Is>
  void destroyImpl(std::index_sequence<Is...>) noexcept {
    (void)((index_ == Is &&
            (std::destroy_at(
                 std::launder(reinterpret_cast<AltAt<Is>*>(storage_))),
             true)) ||
           ...);
  }

  template <size_t... Is>
  void moveImpl(
      ThriftPayloadVariant& src, std::index_sequence<Is...>) noexcept {
    (void)((index_ == Is &&
            (std::construct_at(
                 reinterpret_cast<AltAt<Is>*>(storage_),
                 std::move(*std::launder(
                     reinterpret_cast<AltAt<Is>*>(src.storage_)))),
             true)) ||
           ...);
  }

  using RocketFrameVariant =
      apache::thrift::fast_thrift::frame::ComposedFrameVariant<
          typename Ts::RocketFrame...>;

  template <size_t... Is>
  RocketFrameVariant toRocketFrameImpl(
      rocket::server::MetadataProtocol metadataProtocol,
      std::index_sequence<Is...>) && {
    RocketFrameVariant result;
    (void)((index_ == Is &&
            (result = std::move(
                          *std::launder(reinterpret_cast<AltAt<Is>*>(storage_)))
                          .toRocketFrame(metadataProtocol),
             true)) ||
           ...);
    return result;
  }

  template <size_t... Is>
  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadataImpl(
      std::index_sequence<Is...>) const noexcept {
    const apache::thrift::RequestRpcMetadata* result = nullptr;
    (void)((index_ == Is &&
            (result = std::launder(reinterpret_cast<const AltAt<Is>*>(storage_))
                          ->getRequestRpcMetadata(),
             true)) ||
           ...);
    return result;
  }

  template <size_t... Is>
  const apache::thrift::ResponseRpcMetadata* getResponseRpcMetadataImpl(
      std::index_sequence<Is...>) const noexcept {
    const apache::thrift::ResponseRpcMetadata* result = nullptr;
    (void)((index_ == Is &&
            (result = std::launder(reinterpret_cast<const AltAt<Is>*>(storage_))
                          ->getResponseRpcMetadata(),
             true)) ||
           ...);
    return result;
  }

  void destroy() noexcept {
    if (!valueless_) {
      destroyImpl(std::index_sequence_for<Ts...>{});
      valueless_ = true;
    }
  }

  template <typename T>
  static constexpr uint8_t indexOf() noexcept {
    uint8_t i = 0;
    auto check = [&]<size_t... Is>(std::index_sequence<Is...>) {
      (void)((std::same_as<T, AltAt<Is>> && (i = Is, true)) || ...);
    };
    check(std::index_sequence_for<Ts...>{});
    return i;
  }

 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  ThriftPayloadVariant() noexcept = default;

  // Construct from an alternative. If the alternative type carries a
  // `kRpcKind` constexpr (request-side payloads), `rpcKind` is auto-set
  // from it. Response-side payloads (no `kRpcKind`) leave `rpcKind` at the
  // default — use the (T&&, RpcKind) overload below to set it explicitly.
  template <typename T>
    requires thrift_payload_variant_detail::OneOf<std::decay_t<T>, Ts...>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  /* implicit */ ThriftPayloadVariant(T&& value) noexcept {
    using D = std::decay_t<T>;
    std::construct_at(reinterpret_cast<D*>(storage_), std::forward<T>(value));
    index_ = indexOf<D>();
    valueless_ = false;
    if constexpr (requires { D::kRpcKind; }) {
      rpcKind_ = D::kRpcKind;
    }
  }

  // Construct with an explicit `rpcKind` — required for response-side
  // alternatives (which don't carry `kRpcKind`); also valid for
  // request-side alternatives, where it overrides the auto-deduced kind.
  template <typename T>
    requires thrift_payload_variant_detail::OneOf<std::decay_t<T>, Ts...>
  ThriftPayloadVariant(T&& value, apache::thrift::RpcKind kind) noexcept
      : ThriftPayloadVariant(std::forward<T>(value)) {
    rpcKind_ = kind;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  ThriftPayloadVariant(ThriftPayloadVariant&& o) noexcept
      : rpcKind_(o.rpcKind_), index_(o.index_), valueless_(o.valueless_) {
    if (!valueless_) {
      moveImpl(o, std::index_sequence_for<Ts...>{});
      o.destroy();
    }
  }

  ThriftPayloadVariant& operator=(ThriftPayloadVariant&& o) noexcept {
    if (this != &o) {
      destroy();
      rpcKind_ = o.rpcKind_;
      index_ = o.index_;
      valueless_ = o.valueless_;
      if (!valueless_) {
        moveImpl(o, std::index_sequence_for<Ts...>{});
        o.destroy();
      }
    }
    return *this;
  }

  ~ThriftPayloadVariant() { destroy(); }

  ThriftPayloadVariant(const ThriftPayloadVariant&) = delete;
  ThriftPayloadVariant& operator=(const ThriftPayloadVariant&) = delete;

  template <typename T, typename... Args>
    requires thrift_payload_variant_detail::OneOf<T, Ts...>
  T& emplace(Args&&... args) {
    destroy();
    auto* p = std::construct_at(
        reinterpret_cast<T*>(storage_), std::forward<Args>(args)...);
    index_ = indexOf<T>();
    valueless_ = false;
    if constexpr (requires { T::kRpcKind; }) {
      rpcKind_ = T::kRpcKind;
    }
    return *p;
  }

  template <typename T>
    requires thrift_payload_variant_detail::OneOf<std::decay_t<T>, Ts...>
  ThriftPayloadVariant& operator=(T&& value) noexcept {
    emplace<std::decay_t<T>>(std::forward<T>(value));
    return *this;
  }

  // === Discriminator queries ===

  bool valueless() const noexcept { return valueless_; }

  template <typename T>
    requires thrift_payload_variant_detail::OneOf<T, Ts...>
  bool is() const noexcept {
    return !valueless_ && index_ == indexOf<T>();
  }

  template <typename T>
    requires thrift_payload_variant_detail::OneOf<T, Ts...>
  T& get() noexcept {
    return *std::launder(reinterpret_cast<T*>(storage_));
  }

  template <typename T>
    requires thrift_payload_variant_detail::OneOf<T, Ts...>
  const T& get() const noexcept {
    return *std::launder(reinterpret_cast<const T*>(storage_));
  }

  // === RpcKind ===
  //
  // Carried alongside the held alternative. For request-side payloads
  // (which have a `kRpcKind` constexpr) this is auto-set on construction.
  // For response-side payloads it must be set explicitly via the
  // `(T&&, RpcKind)` constructor or `setRpcKind()`.

  apache::thrift::RpcKind rpcKind() const noexcept { return rpcKind_; }

  void setRpcKind(apache::thrift::RpcKind kind) noexcept { rpcKind_ = kind; }

  // === Rocket frame conversion ===
  //
  // Single fold-expression dispatch over alternatives. Each alternative's
  // `toRocketFrame()` returns its specific `Composed*Frame`, which is
  // assigned into the uniform `ComposedFrameVariant` return type.
  // Eliminates the runtime switch in the transport adapter.

  RocketFrameVariant toRocketFrame(
      rocket::server::MetadataProtocol metadataProtocol) && {
    return std::move(*this).toRocketFrameImpl(
        metadataProtocol, std::index_sequence_for<Ts...>{});
  }

  // === Metadata accessors ===
  //
  // Constrained on the refined concepts: only well-formed when every
  // alternative satisfies the corresponding role-payload concept. This
  // turns "is this a role-X variant?" into a compile-time question — the
  // method exists iff the variant's alternatives form a role.
  //
  // Returns nullptr if the variant is valueless or the held alternative
  // hasn't yet been reshaped to expose typed metadata.

  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept
    requires(ThriftRequestPayloadConcept<Ts> && ...)
  {
    if (valueless_) {
      return nullptr;
    }
    return getRequestRpcMetadataImpl(std::index_sequence_for<Ts...>{});
  }

  const apache::thrift::ResponseRpcMetadata* getResponseRpcMetadata()
      const noexcept
    requires(ThriftInitialResponsePayloadConcept<Ts> && ...)
  {
    if (valueless_) {
      return nullptr;
    }
    return getResponseRpcMetadataImpl(std::index_sequence_for<Ts...>{});
  }
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
