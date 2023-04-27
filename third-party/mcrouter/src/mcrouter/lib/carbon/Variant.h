/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <type_traits>
#include <typeindex>
#include <utility>

#include "mcrouter/lib/fbi/cpp/TypeList.h"

namespace carbon {

template <class... Ts>
class Variant {
 public:
  static_assert(
      facebook::memcache::Distinct<Ts...>::value,
      "Variant may only be used with a list of pairwise distinct types");

  Variant() = default;

  Variant(const Variant& other) {
    *this = other;
  }

  Variant& operator=(const Variant& other) {
    using CopierFun = void (*)(Variant&, const Variant&);
    static constexpr CopierFun dispatcher[sizeof...(Ts)] = {
        &Variant::copier<Ts>...};

    if (cleanupFun_) {
      cleanupFun_(*this);
    }

    const int32_t otherWhich = other.which_;
    if (otherWhich >= 0 && static_cast<size_t>(otherWhich) < sizeof...(Ts)) {
      dispatcher[otherWhich](*this, other);
    }
    return *this;
  }

  Variant(Variant&& other) noexcept {
    *this = std::move(other);
  }

  Variant& operator=(Variant&& other) noexcept {
    using MoverFun = void (*)(Variant&, Variant &&);
    static constexpr MoverFun dispatcher[sizeof...(Ts)] = {
        &Variant::mover<Ts>...};

    if (cleanupFun_) {
      cleanupFun_(*this);
    }

    const int32_t otherWhich = other.which_;
    if (otherWhich >= 0 && static_cast<size_t>(otherWhich) < sizeof...(Ts)) {
      dispatcher[otherWhich](*this, std::move(other));
    }
    return *this;
  }

  template <class T>
  Variant& operator=(T&& t) {
    static_assert(
        std::is_move_constructible<T>::value,
        "Variant::operator=(T&&) requires that T be move-constructible");
    emplace<T>(std::forward<T>(t));
    return *this;
  }

  ~Variant() noexcept {
    // Do proper cleanup of the storage.
    if (cleanupFun_) {
      cleanupFun_(*this);
    }
  }

  /**
   * Destroy any existing stored object and construct the new one with
   * provided arguments.
   *
   * @param args  arguments that will be passed to the constructor of T
   */
  template <class T, class... Args>
  T& emplace(Args&&... args) {
    static_assert(
        facebook::memcache::Has<T, Ts...>::value,
        "Wrong type used with Variant!");
    // Cleanup previous value if we have one.
    if (cleanupFun_) {
      cleanupFun_(*this);
    }

    // Perform proper setup.
    new (&storage_) T(std::forward<Args>(args)...);
    which_ = facebook::memcache::IndexOf<T, Ts...>::value;
    cleanupFun_ = &Variant::cleanup<T>;
    return reinterpret_cast<T&>(storage_);
  }

  /**
   * Returns a reference to an object of the type.
   * It's up to the user to make sure the stored type is correct.
   */
  template <class T>
  T& get() noexcept {
    static_assert(
        facebook::memcache::Has<T, Ts...>::value,
        "Attempt to access incompatible type in Variant!");
    assert(which_ == (facebook::memcache::IndexOf<T, Ts...>::value));
    assert(cleanupFun_ != nullptr);
    return reinterpret_cast<T&>(storage_);
  }

  template <class T>
  const T& get() const noexcept {
    static_assert(
        facebook::memcache::Has<T, Ts...>::value,
        "Attempt to access incompatible type in Variant!");
    assert(which_ == (facebook::memcache::IndexOf<T, Ts...>::value));
    assert(cleanupFun_ != nullptr);
    return reinterpret_cast<const T&>(storage_);
  }

  std::type_index which() const {
    static std::type_index types[sizeof...(Ts)] = {typeid(Ts)...};
    if (which_ >= 0 && static_cast<size_t>(which_) < sizeof...(Ts)) {
      return types[which_];
    }
    return typeid(void);
  }

  template <class T>
  bool is() const {
    return which() == std::type_index(typeid(T));
  }

  // When writing a Carbon structure visitor, it may be more efficient (avoiding
  // RTTI) to use whichId() than which(), while remaining just as safe.
  int32_t whichId() const {
    return which_;
  }

 private:
  static constexpr size_t kStorageSize =
      facebook::memcache::Fold<facebook::memcache::MaxOp, sizeof(Ts)...>::value;

  typename std::aligned_storage<kStorageSize>::type storage_;
  int32_t which_{-1};
  void (*cleanupFun_)(Variant&) noexcept {nullptr};

  template <class T>
  static void cleanup(Variant& me) noexcept {
    reinterpret_cast<T&>(me.storage_).~T();
    me.which_ = -1;
    me.cleanupFun_ = nullptr;
  }

  template <class T>
  static void copier(Variant& me, const Variant& other) {
    new (&me.storage_) T(other.get<T>());
    me.which_ = other.which_;
    me.cleanupFun_ = other.cleanupFun_;
  }

  template <class T>
  static void mover(Variant& me, Variant&& other) noexcept {
    static_assert(
        std::is_nothrow_move_constructible<T>::value,
        "Variant may only be used with types with noexcept move constructors");

    new (&me.storage_) T(std::move(other.get<T>()));
    me.which_ = other.which_;
    me.cleanupFun_ = other.cleanupFun_;
    other.cleanupFun_(other);
  }
};

namespace detail {
template <class... Ts>
struct VariantFromList;

template <class... Ts>
struct VariantFromList<List<Ts...>> {
  using type = Variant<Ts...>;
};
} // namespace detail

template <class TList>
using makeVariantFromList = typename detail::VariantFromList<TList>::type;

} // namespace carbon
