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

#include <concepts>
#include <cstdint>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace apache::thrift::fast_thrift {

namespace compact_variant_detail {

template <typename T, typename... Us>
concept OneOf = (std::same_as<T, Us> || ...);

// Index of T in the type pack (1-based; 0 = valueless).
// Compile error if T is not in the pack.
template <typename T, typename... Pack>
struct TypeIndex;

template <typename T, typename... Rest>
struct TypeIndex<T, T, Rest...> : std::integral_constant<uint8_t, 1> {};

template <typename T, typename First, typename... Rest>
struct TypeIndex<T, First, Rest...>
    : std::integral_constant<uint8_t, 1 + TypeIndex<T, Rest...>::value> {};

template <typename... Ts>
inline constexpr size_t maxSizeof = std::max({sizeof(Ts)...});

template <typename... Ts>
inline constexpr size_t maxAlignof = std::max({alignof(Ts)...});

} // namespace compact_variant_detail

/**
 * CompactVariant - A space-efficient variant with a 1-byte discriminator.
 *
 * Similar API to std::variant but uses a uint8_t index instead of size_t,
 * saving 7 bytes. Moves and destruction use compile-time dispatch — the
 * compiler generates an if-else chain that it can fully inline and optimize.
 *
 * Index 0 means valueless (empty). Types are 1-indexed in pack order.
 */
#pragma pack(push, 1)
template <typename... Ts>
class CompactVariant {
  static_assert(sizeof...(Ts) > 0);
  static_assert(sizeof...(Ts) <= 255);

  alignas(compact_variant_detail::maxAlignof<Ts...>) unsigned char storage_
      [compact_variant_detail::maxSizeof<Ts...>];
  uint8_t index_{0};

  template <size_t... Is>
  void destroyImpl(std::index_sequence<Is...>) noexcept {
    (void)((index_ == (Is + 1) &&
            (std::destroy_at(
                 std::launder(
                     reinterpret_cast<
                         std::tuple_element_t<Is, std::tuple<Ts...>>*>(
                         storage_))),
             true)) ||
           ...);
  }

  template <size_t... Is>
  void moveImpl(CompactVariant& src, std::index_sequence<Is...>) noexcept {
    (void)((index_ == (Is + 1) &&
            (std::construct_at(
                 reinterpret_cast<std::tuple_element_t<Is, std::tuple<Ts...>>*>(
                     storage_),
                 std::move(*std::launder(
                     reinterpret_cast<
                         std::tuple_element_t<Is, std::tuple<Ts...>>*>(
                         src.storage_)))),
             true)) ||
           ...);
  }

  void destroy() noexcept {
    if (index_ != 0) {
      destroyImpl(std::index_sequence_for<Ts...>{});
      index_ = 0;
    }
  }

 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  CompactVariant() noexcept = default;

  template <typename T>
    requires compact_variant_detail::OneOf<std::decay_t<T>, Ts...>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  /* implicit */ CompactVariant(T&& value) noexcept {
    using D = std::decay_t<T>;
    std::construct_at(reinterpret_cast<D*>(storage_), std::forward<T>(value));
    index_ = compact_variant_detail::TypeIndex<D, Ts...>::value;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  CompactVariant(CompactVariant&& o) noexcept : index_(o.index_) {
    if (index_ != 0) {
      moveImpl(o, std::index_sequence_for<Ts...>{});
      o.index_ = 0;
    }
  }

  CompactVariant& operator=(CompactVariant&& o) noexcept {
    if (this != &o) {
      destroy();
      index_ = o.index_;
      if (index_ != 0) {
        moveImpl(o, std::index_sequence_for<Ts...>{});
        o.index_ = 0;
      }
    }
    return *this;
  }

  ~CompactVariant() { destroy(); }

  CompactVariant(const CompactVariant&) = delete;
  CompactVariant& operator=(const CompactVariant&) = delete;

  template <typename T, typename... Args>
  T& emplace(Args&&... args) {
    destroy();
    auto* p = std::construct_at(
        reinterpret_cast<T*>(storage_), std::forward<Args>(args)...);
    index_ = compact_variant_detail::TypeIndex<T, Ts...>::value;
    return *p;
  }

  template <typename T>
    requires compact_variant_detail::OneOf<std::decay_t<T>, Ts...>
  CompactVariant& operator=(T&& value) noexcept {
    emplace<std::decay_t<T>>(std::forward<T>(value));
    return *this;
  }

  uint8_t index() const noexcept { return index_; }
  bool valueless() const noexcept { return index_ == 0; }

  template <typename T>
  bool is() const noexcept {
    return index_ == compact_variant_detail::TypeIndex<T, Ts...>::value;
  }

  template <typename T>
  T& get() noexcept {
    return *std::launder(reinterpret_cast<T*>(storage_));
  }

  template <typename T>
  const T& get() const noexcept {
    return *std::launder(reinterpret_cast<const T*>(storage_));
  }
};
#pragma pack(pop)

template <typename T, typename... Ts>
bool holds_alternative(const CompactVariant<Ts...>& v) noexcept {
  return v.template is<T>();
}

template <typename T, typename... Ts>
T& get(CompactVariant<Ts...>& v) noexcept {
  return v.template get<T>();
}

template <typename T, typename... Ts>
const T& get(const CompactVariant<Ts...>& v) noexcept {
  return v.template get<T>();
}

} // namespace apache::thrift::fast_thrift
