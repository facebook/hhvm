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

#include <assert.h>
#include <limits.h>
#include <initializer_list>
#include <memory>
#include <optional>
#include <type_traits>
#include <folly/CPortability.h>
#include <folly/CppAttributes.h>
#include <folly/Function.h>
#include <folly/Portability.h>
#include <folly/Traits.h>
#include <folly/synchronization/AtomicUtil.h>
#include <thrift/lib/cpp2/BoxedValuePtr.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache {
namespace thrift {
namespace detail {

template <typename T>
using is_set_t =
    std::conditional_t<std::is_const<T>::value, const uint8_t, uint8_t>;

[[noreturn]] void throw_on_bad_optional_field_access();
[[noreturn]] void throw_on_bad_union_field_access();
[[noreturn]] void throw_on_nullptr_dereferencing();

struct ensure_isset_unsafe_fn;
struct unset_unsafe_fn;
struct alias_isset_fn;
struct move_to_unique_ptr_fn;
struct assign_from_unique_ptr_fn;
struct union_value_unsafe_fn;

// IntWrapper is a wrapper of integer that's always copy/move assignable
// even if integer is atomic
template <typename T>
struct IntWrapper {
  IntWrapper() = default;
  explicit IntWrapper(T t) : value(t) {}

  T value{0};
};

// If the integer is atomic, operations use only relaxed memory-order since the
// requirement is only to protect the integer against torn reads and writes and
// not to protect any other objects.
template <typename U>
struct IntWrapper<std::atomic<U>> {
  IntWrapper() = default;

  IntWrapper(const IntWrapper& other) noexcept
      : value(other.value.load(std::memory_order_relaxed)) {}
  IntWrapper(IntWrapper&& other) noexcept
      : value(other.value.load(std::memory_order_relaxed)) {}

  IntWrapper& operator=(const IntWrapper& other) noexcept {
    value.store(
        other.value.load(std::memory_order_relaxed), std::memory_order_relaxed);
    return *this;
  }

  IntWrapper& operator=(IntWrapper&& other) noexcept {
    value.store(
        other.value.load(std::memory_order_relaxed), std::memory_order_relaxed);
    return *this;
  }

  std::atomic<U> value{0};
};

template <typename T>
class BitSet {
 public:
  BitSet() = default;

  explicit BitSet(T value) : int_(value) {}

  BitSet(const BitSet&) = default;
  BitSet& operator=(const BitSet& other) = default;

  class reference {
   public:
    reference(BitSet& bitSet, const uint8_t bit) : bitSet_(bitSet), bit_(bit) {}

    reference& operator=(bool flag) {
      if (flag) {
        bitSet_.set(bit_);
      } else {
        bitSet_.reset(bit_);
      }
      return *this;
    }

    operator bool() const { return bitSet_.get(bit_); }

    reference& operator=(reference& other) { return *this = bool(other); }

   private:
    BitSet& bitSet_;
    const uint8_t bit_;
  };

  bool operator[](const uint8_t bit) const {
    assert(bit < NUM_BITS);
    return get(bit);
  }

  reference operator[](const uint8_t bit) {
    assert(bit < NUM_BITS);
    return reference(*this, bit);
  }

  T& value() { return int_.value; }

  const T& value() const { return int_.value; }

 private:
  template <class U>
  static bool get(U u, std::size_t bit) {
    return u & (U(1) << bit);
  }

  template <class U>
  static void set(U& u, std::size_t bit) {
    u |= (U(1) << bit);
  }

  template <class U>
  static void reset(U& u, std::size_t bit) {
    u &= ~(U(1) << bit);
  }

  // If the integer is atomic, operations use only relaxed memory-order since
  // the requirement is only to protect the integer against torn reads and
  // writes and not to protect any other objects.
  template <class U>
  static bool get(const std::atomic<U>& u, std::size_t bit) {
    return u.load(std::memory_order_relaxed) & (U(1) << bit);
  }

  template <class U>
  static void set(std::atomic<U>& u, std::size_t bit) {
    folly::atomic_fetch_set(u, bit, std::memory_order_relaxed);
  }

  template <class U>
  static void reset(std::atomic<U>& u, std::size_t bit) {
    folly::atomic_fetch_reset(u, bit, std::memory_order_relaxed);
  }

  bool get(std::size_t bit) const { return get(int_.value, bit); }
  void set(std::size_t bit) { set(int_.value, bit); }
  void reset(std::size_t bit) { reset(int_.value, bit); }

  IntWrapper<T> int_;

  static constexpr int NUM_BITS = sizeof(T) * CHAR_BIT;
};

template <bool kIsConst>
class BitRef {
  template <bool B>
  friend class BitRef;

 public:
  using Isset = std::conditional_t<kIsConst, const uint8_t, uint8_t>;
  using AtomicIsset = std::
      conditional_t<kIsConst, const std::atomic<uint8_t>, std::atomic<uint8_t>>;

  FOLLY_ERASE BitRef(Isset& isset, uint8_t bit_index)
      : value_(isset), bit_index_(bit_index) {}

  FOLLY_ERASE BitRef(AtomicIsset& isset, uint8_t bit_index)
      : value_(isset), bit_index_(bit_index), is_atomic_(true) {}

  template <bool B>
  explicit BitRef(const BitRef<B>& other)
      : value_(
            other.is_atomic_ ? IssetBitSet(other.value_.atomic.value())
                             : IssetBitSet(other.value_.non_atomic.value())),
        bit_index_(other.bit_index_),
        is_atomic_(other.is_atomic_) {}

#if FOLLY_MOBILE
  // We have this attribute to prevent binary size regression
  // TODO: Remove special attribute for MOBILE
  FOLLY_ERASE
#endif
  void operator=(bool flag) {
    if (is_atomic_) {
      value_.atomic[bit_index_] = flag;
    } else {
      value_.non_atomic[bit_index_] = flag;
    }
  }

  explicit operator bool() const {
    if (is_atomic_) {
      return value_.atomic[bit_index_];
    } else {
      return value_.non_atomic[bit_index_];
    }
  }

 private:
  union IssetBitSet {
    explicit IssetBitSet(Isset& isset) : non_atomic(isset) {}
    explicit IssetBitSet(AtomicIsset& isset) : atomic(isset) {}
    apache::thrift::detail::BitSet<Isset&> non_atomic;
    apache::thrift::detail::BitSet<AtomicIsset&> atomic;
  } value_;

  const uint8_t bit_index_;
  const bool is_atomic_ = false;
};

template <typename value_type, typename return_type = value_type>
using EnableIfConst =
    std::enable_if_t<std::is_const<value_type>::value, return_type>;

template <typename value_type, typename return_type = value_type>
using EnableIfNonConst =
    std::enable_if_t<!std::is_const<value_type>::value, return_type>;

template <typename T, typename U>
using EnableIfImplicit = std::enable_if_t<
    std::is_same<
        std::add_const_t<std::remove_reference_t<U>>,
        std::remove_reference_t<T>>{} &&
    !(std::is_rvalue_reference<T>{} && std::is_lvalue_reference<U>{})>;

} // namespace detail

/// A reference to an unqualified field of the possibly const-qualified type
/// std::remove_reference_t<T> in a Thrift-generated struct.
template <typename T>
class field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");

  template <typename U>
  friend class field_ref;
  friend struct apache::thrift::detail::unset_unsafe_fn;

 public:
  using value_type = std::remove_reference_t<T>;
  using reference_type = T;

 private:
  using BitRef =
      apache::thrift::detail::BitRef<std::is_const<value_type>::value>;

 public:
  /// Internal constructor
  FOLLY_ERASE field_ref(
      reference_type value,
      typename BitRef::Isset& is_set,
      const uint8_t bit_index = 0) noexcept
      : value_(value), bitref_(is_set, bit_index) {}

  /// Internal constructor
  FOLLY_ERASE field_ref(
      reference_type value,
      typename BitRef::AtomicIsset& is_set,
      const uint8_t bit_index = 0) noexcept
      : value_(value), bitref_(is_set, bit_index) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */ field_ref(const field_ref<U>& other) noexcept
      : value_(other.value_), bitref_(other.bitref_) {}

  template <typename U = value_type>
  FOLLY_ERASE
      std::enable_if_t<std::is_assignable<value_type&, U&&>::value, field_ref&>
      operator=(U&& value) noexcept(
          std::is_nothrow_assignable<value_type&, U&&>::value) {
    value_ = static_cast<U&&>(value);
    bitref_ = true;
    return *this;
  }

  // Workaround for https://bugs.llvm.org/show_bug.cgi?id=49442
  FOLLY_ERASE field_ref& operator=(value_type&& value) noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_ = static_cast<value_type&&>(value);
    bitref_ = true;
    return *this;
    value.~value_type(); // Force emit destructor...
  }

  /// Assignment from field_ref is intentionally not provided to prevent
  /// potential confusion between two possible behaviors, copying and reference
  /// rebinding. The copy_from method is provided instead.
  template <typename U>
  FOLLY_ERASE void copy_from(field_ref<U> other) noexcept(
      std::is_nothrow_assignable<value_type&, U>::value) {
    value_ = other.value();
    bitref_ = other.is_set();
  }

  [[deprecated("Use is_set() method instead")]] FOLLY_ERASE bool has_value()
      const noexcept {
    return bool(bitref_);
  }

  /// Returns true iff the field is set. field_ref doesn't provide conversion to
  /// bool to avoid confusion between checking if the field is set and getting
  /// the field's value, particularly for bool fields.
  FOLLY_ERASE bool is_set() const noexcept { return bool(bitref_); }

  /// Returns a reference to the value.
  FOLLY_ERASE reference_type value() const noexcept {
    return static_cast<reference_type>(value_);
  }

  /// Returns a reference to the value.
  FOLLY_ERASE reference_type operator*() const noexcept {
    return static_cast<reference_type>(value_);
  }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() const noexcept {
    return &value_;
  }

  /// Returns a pointer to the value.
  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const noexcept {
    return &value_;
  }

  /// Returns a pointer to the value.
  FOLLY_ERASE value_type* operator->() noexcept { return &value_; }

  FOLLY_ERASE reference_type ensure() noexcept {
    bitref_ = true;
    return static_cast<reference_type>(value_);
  }

  /// Forward operator[] to the value.
  template <typename Index>
  FOLLY_ERASE auto operator[](const Index& index) const -> decltype(auto) {
    return value_[index];
  }

  /// Constructs the value in-place.
  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    bitref_ = false; // C++ Standard requires *this to be empty if
                     // `std::optional::emplace(...)` throws
    value_ = value_type(static_cast<Args&&>(args)...);
    bitref_ = true;
    return value_;
  }

  /// Constructs the value in-place.
  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    bitref_ = false;
    value_ = value_type(ilist, static_cast<Args&&>(args)...);
    bitref_ = true;
    return value_;
  }

 private:
  value_type& value_;
  BitRef bitref_;
};

template <typename T, typename U>
bool operator==(field_ref<T> lhs, field_ref<U> rhs) {
  return *lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(field_ref<T> lhs, field_ref<U> rhs) {
  return *lhs != *rhs;
}

template <typename T, typename U>
bool operator<(field_ref<T> lhs, field_ref<U> rhs) {
  return *lhs < *rhs;
}

template <typename T, typename U>
bool operator>(field_ref<T> lhs, field_ref<U> rhs) {
  return *lhs > *rhs;
}

template <typename T, typename U>
bool operator<=(field_ref<T> lhs, field_ref<U> rhs) {
  return *lhs <= *rhs;
}

template <typename T, typename U>
bool operator>=(field_ref<T> lhs, field_ref<U> rhs) {
  return *lhs >= *rhs;
}

template <typename T, typename U>
bool operator==(field_ref<T> lhs, const U& rhs) {
  return *lhs == rhs;
}

template <typename T, typename U>
bool operator!=(field_ref<T> lhs, const U& rhs) {
  return *lhs != rhs;
}

template <typename T, typename U>
bool operator<(field_ref<T> lhs, const U& rhs) {
  return *lhs < rhs;
}

template <typename T, typename U>
bool operator>(field_ref<T> lhs, const U& rhs) {
  return *lhs > rhs;
}

template <typename T, typename U>
bool operator<=(field_ref<T> lhs, const U& rhs) {
  return *lhs <= rhs;
}

template <typename T, typename U>
bool operator>=(field_ref<T> lhs, const U& rhs) {
  return *lhs >= rhs;
}

template <typename T, typename U>
bool operator==(const T& lhs, field_ref<U> rhs) {
  return lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(const T& lhs, field_ref<U> rhs) {
  return lhs != *rhs;
}

template <typename T, typename U>
bool operator<(const T& lhs, field_ref<U> rhs) {
  return lhs < *rhs;
}

template <typename T, typename U>
bool operator>(const T& lhs, field_ref<U> rhs) {
  return lhs > *rhs;
}

template <typename T, typename U>
bool operator<=(const T& lhs, field_ref<U> rhs) {
  return lhs <= *rhs;
}

template <typename T, typename U>
bool operator>=(const T& lhs, field_ref<U> rhs) {
  return lhs >= *rhs;
}

// A reference to an optional field of the possibly const-qualified type
// std::remove_reference_t<T> in a Thrift-generated struct.
template <typename T>
class optional_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");

  template <typename U>
  friend class optional_field_ref;
  friend struct apache::thrift::detail::ensure_isset_unsafe_fn;
  friend struct apache::thrift::detail::unset_unsafe_fn;
  friend struct apache::thrift::detail::alias_isset_fn;

 public:
  using value_type = std::remove_reference_t<T>;
  using reference_type = T;

 private:
  using BitRef =
      apache::thrift::detail::BitRef<std::is_const<value_type>::value>;

  // for alias_isset_fn
  FOLLY_ERASE optional_field_ref(reference_type value, BitRef bitref)
      : value_(value), bitref_(bitref) {}

 public:
  FOLLY_ERASE optional_field_ref(
      reference_type value,
      typename BitRef::Isset& is_set,
      const uint8_t bit_index = 0) noexcept
      : value_(value), bitref_(is_set, bit_index) {}

  FOLLY_ERASE optional_field_ref(
      reference_type value,
      typename BitRef::AtomicIsset& is_set,
      const uint8_t bit_index = 0) noexcept
      : value_(value), bitref_(is_set, bit_index) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */ optional_field_ref(
      const optional_field_ref<U>& other) noexcept
      : value_(other.value_), bitref_(other.bitref_) {}

  template <
      typename U,
      std::enable_if_t<
          std::is_same<T, U&&>{} || std::is_same<T, const U&&>{},
          int> = 0>
  FOLLY_ERASE explicit optional_field_ref(
      const optional_field_ref<U&>& other) noexcept
      : value_(other.value_), bitref_(other.bitref_) {}

  template <typename U = value_type>
  FOLLY_ERASE std::enable_if_t<
      std::is_assignable<value_type&, U&&>::value,
      optional_field_ref&>
  operator=(U&& value) noexcept(
      std::is_nothrow_assignable<value_type&, U&&>::value) {
    value_ = static_cast<U&&>(value);
    bitref_ = true;
    return *this;
  }

  // Workaround for https://bugs.llvm.org/show_bug.cgi?id=49442
  FOLLY_ERASE optional_field_ref& operator=(value_type&& value) noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_ = static_cast<value_type&&>(value);
    bitref_ = true;
    return *this;
    value.~value_type(); // Force emit destructor...
  }

  // Copies the data (the set flag and the value if available) from another
  // optional_field_ref object.
  //
  // Assignment from optional_field_ref is intentionally not provided to prevent
  // potential confusion between two possible behaviors, copying and reference
  // rebinding. This copy_from method is provided instead.
  template <typename U>
  FOLLY_ERASE void copy_from(const optional_field_ref<U>& other) noexcept(
      std::is_nothrow_assignable<value_type&, U>::value) {
    value_ = other.value_unchecked();
    bitref_ = other.has_value();
  }

  template <typename U>
  FOLLY_ERASE void move_from(optional_field_ref<U> other) noexcept(
      std::is_nothrow_assignable<value_type&, std::remove_reference_t<U>&&>::
          value) {
    value_ = static_cast<std::remove_reference_t<U>&&>(other.value_);
    bitref_ = other.has_value();
  }

  template <typename U>
  FOLLY_ERASE void from_optional(const std::optional<U>& other) noexcept(
      std::is_nothrow_assignable<value_type&, const U&>::value) {
    // Use if instead of a shorter ternary expression to prevent a potential
    // copy if T and U mismatch.
    if (other) {
      value_ = *other;
    } else {
      value_ = {};
    }
    bitref_ = other.has_value();
  }

  // Moves the value from std::optional. As std::optional's move constructor,
  // move_from doesn't make other empty.
  template <typename U>
  FOLLY_ERASE void from_optional(std::optional<U>&& other) noexcept(
      std::is_nothrow_assignable<value_type&, U&&>::value) {
    // Use if instead of a shorter ternary expression to prevent a potential
    // copy if T and U mismatch.
    if (other) {
      value_ = static_cast<U&&>(*other);
    } else {
      value_ = {};
    }
    bitref_ = other.has_value();
  }

  FOLLY_ERASE std::optional<std::remove_const_t<value_type>> to_optional()
      const {
    using type = std::optional<std::remove_const_t<value_type>>;
    return bitref_ ? type(value_) : type();
  }

  FOLLY_ERASE bool has_value() const noexcept { return bool(bitref_); }

  FOLLY_ERASE explicit operator bool() const noexcept { return bool(bitref_); }

  FOLLY_ERASE void reset() noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_ = value_type();
    bitref_ = false;
  }

  // Returns a reference to the value if this optional_field_ref has one; throws
  // bad_field_access otherwise.
  FOLLY_ERASE reference_type value() const {
    throw_if_unset();
    return static_cast<reference_type>(value_);
  }

  template <typename U = std::remove_const_t<value_type>>
  FOLLY_ERASE std::remove_const_t<value_type> value_or(
      U&& default_value) const {
    using type = std::remove_const_t<value_type>;
    return bitref_ ? type(static_cast<reference_type>(value_))
                   : type(static_cast<U&&>(default_value));
  }

  // Returns a reference to the value without checking whether it is available.
  FOLLY_ERASE reference_type value_unchecked() const {
    return static_cast<reference_type>(value_);
  }

  FOLLY_ERASE reference_type operator*() const { return value(); }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() const {
    throw_if_unset();
    return &value_;
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const {
    throw_if_unset();
    return &value_;
  }

  FOLLY_ERASE value_type* operator->() {
    throw_if_unset();
    return &value_;
  }

  FOLLY_ERASE reference_type
  ensure() noexcept(std::is_nothrow_move_assignable<value_type>::value) {
    if (!bitref_) {
      value_ = value_type();
      bitref_ = true;
    }
    return static_cast<reference_type>(value_);
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    reset(); // C++ Standard requires *this to be empty if
             // `std::optional::emplace(...)` throws
    value_ = value_type(static_cast<Args&&>(args)...);
    bitref_ = true;
    return value_;
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>&, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    reset();
    value_ = value_type(ilist, static_cast<Args&&>(args)...);
    bitref_ = true;
    return value_;
  }

 private:
  FOLLY_ERASE void throw_if_unset() const {
    if (!bitref_) {
      apache::thrift::detail::throw_on_bad_optional_field_access();
    }
  }

  value_type& value_;
  BitRef bitref_;
};

template <typename T1, typename T2>
bool operator==(optional_field_ref<T1> a, optional_field_ref<T2> b) {
  return a && b ? *a == *b : a.has_value() == b.has_value();
}

template <typename T1, typename T2>
bool operator!=(optional_field_ref<T1> a, optional_field_ref<T2> b) {
  return !(a == b);
}

template <typename T1, typename T2>
bool operator<(optional_field_ref<T1> a, optional_field_ref<T2> b) {
  if (a.has_value() != b.has_value()) {
    return a.has_value() < b.has_value();
  }
  return a ? *a < *b : false;
}

template <typename T1, typename T2>
bool operator>(optional_field_ref<T1> a, optional_field_ref<T2> b) {
  return b < a;
}

template <typename T1, typename T2>
bool operator<=(optional_field_ref<T1> a, optional_field_ref<T2> b) {
  return !(a > b);
}

template <typename T1, typename T2>
bool operator>=(optional_field_ref<T1> a, optional_field_ref<T2> b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator==(optional_field_ref<T> a, const U& b) {
  return a ? *a == b : false;
}

template <typename T, typename U>
bool operator!=(optional_field_ref<T> a, const U& b) {
  return !(a == b);
}

template <typename T, typename U>
bool operator==(const U& a, optional_field_ref<T> b) {
  return b == a;
}

template <typename T, typename U>
bool operator!=(const U& a, optional_field_ref<T> b) {
  return b != a;
}

template <typename T, typename U>
bool operator<(optional_field_ref<T> a, const U& b) {
  return a ? *a < b : true;
}

template <typename T, typename U>
bool operator>(optional_field_ref<T> a, const U& b) {
  return a ? *a > b : false;
}

template <typename T, typename U>
bool operator<=(optional_field_ref<T> a, const U& b) {
  return !(a > b);
}

template <typename T, typename U>
bool operator>=(optional_field_ref<T> a, const U& b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator<(const U& a, optional_field_ref<T> b) {
  return b > a;
}

template <typename T, typename U>
bool operator<=(const U& a, optional_field_ref<T> b) {
  return b >= a;
}

template <typename T, typename U>
bool operator>(const U& a, optional_field_ref<T> b) {
  return b < a;
}

template <typename T, typename U>
bool operator>=(const U& a, optional_field_ref<T> b) {
  return b <= a;
}

template <class T>
bool operator==(const optional_field_ref<T>& a, std::nullopt_t) {
  return !a.has_value();
}
template <class T>
bool operator==(std::nullopt_t, const optional_field_ref<T>& a) {
  return !a.has_value();
}
template <class T>
bool operator!=(const optional_field_ref<T>& a, std::nullopt_t) {
  return a.has_value();
}
template <class T>
bool operator!=(std::nullopt_t, const optional_field_ref<T>& a) {
  return a.has_value();
}

namespace detail {

template <typename T>
FOLLY_INLINE_VARIABLE constexpr bool is_boxed_value_ptr_v = false;

template <typename T>
FOLLY_INLINE_VARIABLE constexpr bool is_boxed_value_ptr_v<boxed_value_ptr<T>> =
    true;

template <typename T>
FOLLY_INLINE_VARIABLE constexpr bool is_boxed_value_v = false;

template <typename T>
FOLLY_INLINE_VARIABLE constexpr bool is_boxed_value_v<boxed_value<T>> = true;

template <typename From, typename To>
using copy_reference_t = std::conditional_t<
    std::is_lvalue_reference<From>{},
    std::add_lvalue_reference_t<To>,
    std::add_rvalue_reference_t<To>>;

template <typename From, typename To>
using copy_const_t = std::conditional_t<
    std::is_const<std::remove_reference_t<From>>{},
    std::add_const_t<To>,
    To>;

} // namespace detail

template <typename T>
class optional_boxed_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");
  static_assert(
      detail::is_boxed_value_ptr_v<folly::remove_cvref_t<T>>,
      "not a boxed_value_ptr");

  using element_type = typename folly::remove_cvref_t<T>::element_type;

  template <typename U>
  friend class optional_boxed_field_ref;
  friend struct apache::thrift::detail::move_to_unique_ptr_fn;
  friend struct apache::thrift::detail::assign_from_unique_ptr_fn;

 public:
  using value_type = detail::copy_const_t<T, element_type>;
  using reference_type = detail::copy_reference_t<T, value_type>;

  FOLLY_ERASE explicit optional_boxed_field_ref(T value) noexcept
      : value_(value) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */
  optional_boxed_field_ref(const optional_boxed_field_ref<U>& other) noexcept
      : value_(other.value_) {}

  template <
      typename U,
      std::enable_if_t<
          std::is_same<T, U&&>{} || std::is_same<T, const U&&>{},
          int> = 0>
  FOLLY_ERASE explicit optional_boxed_field_ref(
      const optional_boxed_field_ref<U&>& other) noexcept
      : value_(other.value_) {}

  template <typename U = value_type>
  FOLLY_ERASE std::enable_if_t<
      std::is_assignable<value_type&, U&&>::value,
      optional_boxed_field_ref&>
  operator=(U&& value) {
    value_ = static_cast<U&&>(value);
    return *this;
  }

  // Copies the data (the set flag and the value if available) from another
  // optional_boxed_field_ref object.
  //
  // Assignment from optional_boxed_field_ref is intentionally not provided to
  // prevent potential confusion between two possible behaviors, copying and
  // reference rebinding. This copy_from method is provided instead.
  template <typename U>
  FOLLY_ERASE void copy_from(const optional_boxed_field_ref<U>& other) {
    value_ = T(other.value_);
  }

  template <typename U>
  FOLLY_ERASE void move_from(optional_boxed_field_ref<U> other) noexcept {
    value_ = static_cast<std::remove_reference_t<U>&&>(other.value_);
  }

  template <typename U>
  FOLLY_ERASE void from_optional(const std::optional<U>& other) {
    // Use if instead of a shorter ternary expression to prevent a potential
    // copy if T and U mismatch.
    if (other) {
      value_ = *other;
    } else {
      value_ = {};
    }
  }

  // Moves the value from std::optional. As std::optional's move constructor,
  // move_from doesn't make other empty.
  template <typename U>
  FOLLY_ERASE void from_optional(std::optional<U>&& other) {
    // Use if instead of a shorter ternary expression to prevent a potential
    // copy if T and U mismatch.
    if (other) {
      value_ = static_cast<U&&>(*other);
    } else {
      value_ = {};
    }
  }

  FOLLY_ERASE std::optional<std::remove_const_t<value_type>> to_optional()
      const {
    using type = std::optional<std::remove_const_t<value_type>>;
    return has_value() ? type(*value_) : type();
  }

  FOLLY_ERASE bool has_value() const noexcept {
    return static_cast<bool>(value_);
  }

  FOLLY_ERASE explicit operator bool() const noexcept { return has_value(); }

  FOLLY_ERASE void reset() noexcept { value_.reset(); }

  // Returns a reference to the value if this optional_boxed_field_ref has one;
  // throws bad_field_access otherwise.
  FOLLY_ERASE reference_type value() const {
    throw_if_unset();
    return static_cast<reference_type>(*value_);
  }

  template <typename U = std::remove_const_t<value_type>>
  FOLLY_ERASE std::remove_const_t<value_type> value_or(
      U&& default_value) const {
    using type = std::remove_const_t<value_type>;
    return has_value() ? type(static_cast<reference_type>(*value_))
                       : type(static_cast<U&&>(default_value));
  }

  FOLLY_ERASE reference_type operator*() const { return value(); }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() const {
    throw_if_unset();
    return &*value_;
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const {
    throw_if_unset();
    return &*value_;
  }

  FOLLY_ERASE value_type* operator->() {
    throw_if_unset();
    return &*value_;
  }

  FOLLY_ERASE reference_type ensure() {
    if (!has_value()) {
      emplace();
    }
    return static_cast<reference_type>(*value_);
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    reset(); // C++ Standard requires *this to be empty if
             // `std::optional::emplace(...)` throws
    value_ = value_type(static_cast<Args&&>(args)...);
    return *value_;
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>&, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    reset();
    value_ = value_type(ilist, static_cast<Args&&>(args)...);
    return *value_;
  }

 private:
  FOLLY_ERASE void throw_if_unset() const {
    if (!has_value()) {
      apache::thrift::detail::throw_on_bad_optional_field_access();
    }
  }

  FOLLY_ERASE std::unique_ptr<element_type> release() noexcept {
    return value_.release();
  }

  FOLLY_ERASE void reset(std::unique_ptr<element_type> ptr) noexcept {
    value_.reset(ptr.release());
  }

  std::remove_reference_t<T>& value_;
};

template <typename T1, typename T2>
bool operator==(
    optional_boxed_field_ref<T1> a, optional_boxed_field_ref<T2> b) {
  return a && b ? *a == *b : a.has_value() == b.has_value();
}

template <typename T1, typename T2>
bool operator!=(
    optional_boxed_field_ref<T1> a, optional_boxed_field_ref<T2> b) {
  return !(a == b);
}

template <typename T1, typename T2>
bool operator<(optional_boxed_field_ref<T1> a, optional_boxed_field_ref<T2> b) {
  if (a.has_value() != b.has_value()) {
    return a.has_value() < b.has_value();
  }
  return a ? *a < *b : false;
}

template <typename T1, typename T2>
bool operator>(optional_boxed_field_ref<T1> a, optional_boxed_field_ref<T2> b) {
  return b < a;
}

template <typename T1, typename T2>
bool operator<=(
    optional_boxed_field_ref<T1> a, optional_boxed_field_ref<T2> b) {
  return !(a > b);
}

template <typename T1, typename T2>
bool operator>=(
    optional_boxed_field_ref<T1> a, optional_boxed_field_ref<T2> b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator==(optional_boxed_field_ref<T> a, const U& b) {
  return a ? *a == b : false;
}

template <typename T, typename U>
bool operator!=(optional_boxed_field_ref<T> a, const U& b) {
  return !(a == b);
}

template <typename T, typename U>
bool operator==(const U& a, optional_boxed_field_ref<T> b) {
  return b == a;
}

template <typename T, typename U>
bool operator!=(const U& a, optional_boxed_field_ref<T> b) {
  return b != a;
}

template <typename T, typename U>
bool operator<(optional_boxed_field_ref<T> a, const U& b) {
  return a ? *a < b : true;
}

template <typename T, typename U>
bool operator>(optional_boxed_field_ref<T> a, const U& b) {
  return a ? *a > b : false;
}

template <typename T, typename U>
bool operator<=(optional_boxed_field_ref<T> a, const U& b) {
  return !(a > b);
}

template <typename T, typename U>
bool operator>=(optional_boxed_field_ref<T> a, const U& b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator<(const U& a, optional_boxed_field_ref<T> b) {
  return b > a;
}

template <typename T, typename U>
bool operator<=(const U& a, optional_boxed_field_ref<T> b) {
  return b >= a;
}

template <typename T, typename U>
bool operator>(const U& a, optional_boxed_field_ref<T> b) {
  return b < a;
}

template <typename T, typename U>
bool operator>=(const U& a, optional_boxed_field_ref<T> b) {
  return b <= a;
}

template <class T>
bool operator==(const optional_boxed_field_ref<T>& a, std::nullopt_t) {
  return !a.has_value();
}
template <class T>
bool operator==(std::nullopt_t, const optional_boxed_field_ref<T>& a) {
  return !a.has_value();
}
template <class T>
bool operator!=(const optional_boxed_field_ref<T>& a, std::nullopt_t) {
  return a.has_value();
}
template <class T>
bool operator!=(std::nullopt_t, const optional_boxed_field_ref<T>& a) {
  return a.has_value();
}

// A reference to a 'Fill' intern boxed field.
//
// It currently only supports Thrift structs.
template <typename T>
class intern_boxed_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");
  static_assert(
      detail::is_boxed_value_v<folly::remove_cvref_t<T>>, "not a boxed_value");

  using element_type = typename folly::remove_cvref_t<T>::element_type;
  using boxed_value_type = std::remove_reference_t<T>;

  template <typename U>
  friend class intern_boxed_field_ref;

  // TODO(dokwon): Consider removing `get_default_t` after resolving
  // dependency issue.
  using get_default_t = folly::FunctionRef<const element_type&()>;

 public:
  using value_type = detail::copy_const_t<T, element_type>;
  using reference_type = detail::copy_reference_t<T, value_type>;

 private:
  using BitRef =
      apache::thrift::detail::BitRef<std::is_const<value_type>::value>;

 public:
  FOLLY_ERASE intern_boxed_field_ref(
      T value,
      get_default_t get_default,
      typename BitRef::Isset& is_set,
      const uint8_t bit_index = 0) noexcept
      : value_(value), get_default_(get_default), bitref_(is_set, bit_index) {}

  FOLLY_ERASE intern_boxed_field_ref(
      T value,
      get_default_t get_default,
      typename BitRef::AtomicIsset& is_set,
      const uint8_t bit_index = 0) noexcept
      : value_(value), get_default_(get_default), bitref_(is_set, bit_index) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */ intern_boxed_field_ref(
      const intern_boxed_field_ref<U>& other) noexcept
      : value_(other.value_), bitref_(other.bitref_) {}

  template <typename U = value_type>
  FOLLY_ERASE std::enable_if_t<
      std::is_assignable<value_type&, U&&>::value,
      intern_boxed_field_ref&>
  operator=(U&& value) {
    value_.mut() = static_cast<U&&>(value);
    bitref_ = true;
    return *this;
  }

  // Workaround for https://bugs.llvm.org/show_bug.cgi?id=49442
  FOLLY_ERASE intern_boxed_field_ref& operator=(value_type&& value) noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_.mut() = static_cast<value_type&&>(value);
    bitref_ = true;
    return *this;
    value.~value_type(); // Force emit destructor...
  }

  // If the other field owns the value, it will perform deep copy. If the other
  // field does not own the value, it will perform a shallow copy.
  template <typename U>
  FOLLY_ERASE void copy_from(const intern_boxed_field_ref<U>& other) {
    value_ = other.value_;
    bitref_ = other.is_set();
  }

  [[deprecated("Use is_set() method instead")]] FOLLY_ERASE bool has_value()
      const noexcept {
    return bool(bitref_);
  }

  // Returns true iff the field is set. 'intern_boxed_field_ref' doesn't provide
  // conversion to bool to avoid confusion between checking if the field is set
  // and getting the field's value, particularly for bool fields.
  FOLLY_ERASE bool is_set() const noexcept { return bool(bitref_); }

  FOLLY_ERASE void reset() noexcept {
    // reset to the intern default.
    value_ = boxed_value_type::fromStaticConstant(&get_default_());
    bitref_ = false;
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfNonConst<U, reference_type> value() {
    return static_cast<reference_type>(value_.mut());
  }
  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U, reference_type> value() const {
    return static_cast<reference_type>(value_.value());
  }

  FOLLY_ERASE reference_type ensure() noexcept {
    bitref_ = true;
    return static_cast<reference_type>(value_.mut());
  }

  FOLLY_ERASE reference_type operator*() { return value(); }
  FOLLY_ERASE reference_type operator*() const { return value(); }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() {
    return &value_.mut();
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const {
    return &value_.value();
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    bitref_ = false; // C++ Standard requires *this to be empty if
                     // `std::optional::emplace(...)` throws
    value_.reset(std::make_unique<value_type>(static_cast<Args&&>(args)...));
    bitref_ = true;
    return value_.mut();
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>&, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    bitref_ = false;
    value_.reset(
        std::make_unique<value_type>(ilist, static_cast<Args&&>(args)...));
    bitref_ = true;
    return value_.value();
  }

 private:
  boxed_value_type& value_;
  get_default_t get_default_;
  BitRef bitref_;
};

template <typename T1, typename T2>
bool operator==(intern_boxed_field_ref<T1> a, intern_boxed_field_ref<T2> b) {
  return *a == *b;
}

template <typename T1, typename T2>
bool operator!=(intern_boxed_field_ref<T1> a, intern_boxed_field_ref<T2> b) {
  return !(a == b);
}

template <typename T1, typename T2>
bool operator<(intern_boxed_field_ref<T1> a, intern_boxed_field_ref<T2> b) {
  return *a < *b;
}

template <typename T1, typename T2>
bool operator>(intern_boxed_field_ref<T1> a, intern_boxed_field_ref<T2> b) {
  return b < a;
}

template <typename T1, typename T2>
bool operator<=(intern_boxed_field_ref<T1> a, intern_boxed_field_ref<T2> b) {
  return !(a > b);
}

template <typename T1, typename T2>
bool operator>=(intern_boxed_field_ref<T1> a, intern_boxed_field_ref<T2> b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator==(intern_boxed_field_ref<T> a, const U& b) {
  return *a == b;
}

template <typename T, typename U>
bool operator!=(intern_boxed_field_ref<T> a, const U& b) {
  return !(a == b);
}

template <typename T, typename U>
bool operator==(const U& a, intern_boxed_field_ref<T> b) {
  return b == a;
}

template <typename T, typename U>
bool operator!=(const U& a, intern_boxed_field_ref<T> b) {
  return b != a;
}

template <typename T, typename U>
bool operator<(intern_boxed_field_ref<T> a, const U& b) {
  return *a < b;
}

template <typename T, typename U>
bool operator>(intern_boxed_field_ref<T> a, const U& b) {
  return *a > b;
}

template <typename T, typename U>
bool operator<=(intern_boxed_field_ref<T> a, const U& b) {
  return !(a > b);
}

template <typename T, typename U>
bool operator>=(intern_boxed_field_ref<T> a, const U& b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator<(const U& a, intern_boxed_field_ref<T> b) {
  return b > a;
}

template <typename T, typename U>
bool operator<=(const U& a, intern_boxed_field_ref<T> b) {
  return b >= a;
}

template <typename T, typename U>
bool operator>(const U& a, intern_boxed_field_ref<T> b) {
  return b < a;
}

template <typename T, typename U>
bool operator>=(const U& a, intern_boxed_field_ref<T> b) {
  return b <= a;
}

// A reference to a 'terse' intern boxed field.
//
// It currently only supports Thrift structs.
template <typename T>
class terse_intern_boxed_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");
  static_assert(
      detail::is_boxed_value_v<folly::remove_cvref_t<T>>, "not a boxed_value");

  using element_type = typename folly::remove_cvref_t<T>::element_type;
  using boxed_value_type = std::remove_reference_t<T>;

  template <typename U>
  friend class terse_intern_boxed_field_ref;

  // TODO(dokwon): Consider removing `get_default_t` after resolving
  // dependency issue.
  using get_default_t = folly::FunctionRef<const element_type&()>;

 public:
  using value_type = detail::copy_const_t<T, element_type>;
  using reference_type = detail::copy_reference_t<T, value_type>;

  FOLLY_ERASE terse_intern_boxed_field_ref(
      T value, get_default_t get_default) noexcept
      : value_(value), get_default_(get_default) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */ terse_intern_boxed_field_ref(
      const terse_intern_boxed_field_ref<U>& other) noexcept
      : value_(other.value_) {}

  template <typename U = value_type>
  FOLLY_ERASE std::enable_if_t<
      std::is_assignable<value_type&, U&&>::value,
      terse_intern_boxed_field_ref&>
  operator=(U&& value) {
    value_.mut() = static_cast<U&&>(value);
    return *this;
  }

  // Workaround for https://bugs.llvm.org/show_bug.cgi?id=49442
  FOLLY_ERASE terse_intern_boxed_field_ref&
  operator=(value_type&& value) noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_.mut() = static_cast<value_type&&>(value);
    return *this;
    value.~value_type(); // Force emit destructor...
  }

  template <typename U>
  FOLLY_ERASE void copy_from(const terse_intern_boxed_field_ref<U>& other) {
    value_ = other.value_;
  }

  template <typename U>
  FOLLY_ERASE void move_from(terse_intern_boxed_field_ref<U> other) noexcept(
      std::is_nothrow_assignable<value_type&, std::remove_reference_t<U>&&>::
          value) {
    value_ = static_cast<std::remove_reference_t<U>&&>(other.value_);
  }

  FOLLY_ERASE void reset() noexcept {
    // reset to the intern intrinsic default.
    value_ = boxed_value_type::fromStaticConstant(&get_default_());
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfNonConst<U, reference_type> value() {
    return static_cast<reference_type>(value_.mut());
  }
  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U, reference_type> value() const {
    return static_cast<reference_type>(value_.value());
  }

  FOLLY_ERASE reference_type ensure() noexcept {
    return static_cast<reference_type>(value_.mut());
  }

  FOLLY_ERASE reference_type operator*() { return value(); }
  FOLLY_ERASE reference_type operator*() const { return value(); }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() {
    return &value_.mut();
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const {
    return &value_.value();
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    value_.reset(std::make_unique<value_type>(static_cast<Args&&>(args)...));
    return value_.mut();
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>&, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    value_.reset(
        std::make_unique<value_type>(ilist, static_cast<Args&&>(args)...));
    return value_.value();
  }

 private:
  boxed_value_type& value_;
  get_default_t get_default_;
};

template <typename T>
terse_intern_boxed_field_ref<const T&> as_const_intern_box(
    terse_intern_boxed_field_ref<T&> val) {
  return val;
}

template <typename T1, typename T2>
bool operator==(
    terse_intern_boxed_field_ref<T1> a, terse_intern_boxed_field_ref<T2> b) {
  return *a == *b;
}

template <typename T1, typename T2>
bool operator!=(
    terse_intern_boxed_field_ref<T1> a, terse_intern_boxed_field_ref<T2> b) {
  return !(a == b);
}

template <typename T1, typename T2>
bool operator<(
    terse_intern_boxed_field_ref<T1> a, terse_intern_boxed_field_ref<T2> b) {
  return *a < *b;
}

template <typename T1, typename T2>
bool operator>(
    terse_intern_boxed_field_ref<T1> a, terse_intern_boxed_field_ref<T2> b) {
  return b < a;
}

template <typename T1, typename T2>
bool operator<=(
    terse_intern_boxed_field_ref<T1> a, terse_intern_boxed_field_ref<T2> b) {
  return !(a > b);
}

template <typename T1, typename T2>
bool operator>=(
    terse_intern_boxed_field_ref<T1> a, terse_intern_boxed_field_ref<T2> b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator==(terse_intern_boxed_field_ref<T> a, const U& b) {
  return *a == b;
}

template <typename T, typename U>
bool operator!=(terse_intern_boxed_field_ref<T> a, const U& b) {
  return !(a == b);
}

template <typename T, typename U>
bool operator==(const U& a, terse_intern_boxed_field_ref<T> b) {
  return b == a;
}

template <typename T, typename U>
bool operator!=(const U& a, terse_intern_boxed_field_ref<T> b) {
  return b != a;
}

template <typename T, typename U>
bool operator<(terse_intern_boxed_field_ref<T> a, const U& b) {
  return *a < b;
}

template <typename T, typename U>
bool operator>(terse_intern_boxed_field_ref<T> a, const U& b) {
  return *a > b;
}

template <typename T, typename U>
bool operator<=(terse_intern_boxed_field_ref<T> a, const U& b) {
  return !(a > b);
}

template <typename T, typename U>
bool operator>=(terse_intern_boxed_field_ref<T> a, const U& b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator<(const U& a, terse_intern_boxed_field_ref<T> b) {
  return b > a;
}

template <typename T, typename U>
bool operator<=(const U& a, terse_intern_boxed_field_ref<T> b) {
  return b >= a;
}

template <typename T, typename U>
bool operator>(const U& a, terse_intern_boxed_field_ref<T> b) {
  return b < a;
}

template <typename T, typename U>
bool operator>=(const U& a, terse_intern_boxed_field_ref<T> b) {
  return b <= a;
}

namespace detail {

struct get_pointer_fn {
  template <class T>
  T* operator()(optional_field_ref<T&> field) const {
    return field ? &*field : nullptr;
  }

  template <class T>
  auto* operator()(optional_boxed_field_ref<T&> field) const {
    return field ? &*field : nullptr;
  }
};

struct can_throw_fn {
  template <typename T>
  FOLLY_ERASE T&& operator()(T&& value) const {
    return static_cast<T&&>(value);
  }
};

struct ensure_isset_unsafe_fn {
  template <typename T>
  void operator()(optional_field_ref<T> ref) const noexcept {
    ref.bitref_ = true;
  }
};

struct unset_unsafe_fn {
  template <typename T>
  void operator()(field_ref<T> ref) const noexcept {
    ref.bitref_ = false;
  }

  template <typename T>
  void operator()(optional_field_ref<T> ref) const noexcept {
    ref.bitref_ = false;
  }
};

struct alias_isset_fn {
  template <typename T, typename F>
  auto operator()(optional_field_ref<T> ref, F functor) const
      noexcept(noexcept(functor(ref.value_))) {
    auto&& result = functor(ref.value_);
    return optional_field_ref<decltype(result)>(
        static_cast<decltype(result)>(result), ref.bitref_);
  }
};

template <typename T>
FOLLY_ERASE apache::thrift::optional_field_ref<T&&> make_optional_field_ref(
    T&& ref,
    apache::thrift::detail::is_set_t<std::remove_reference_t<T>>& is_set) {
  return {std::forward<T>(ref), is_set};
}

template <typename T>
FOLLY_ERASE apache::thrift::field_ref<T&&> make_field_ref(
    T&& ref,
    apache::thrift::detail::is_set_t<std::remove_reference_t<T>>& is_set) {
  return {std::forward<T>(ref), is_set};
}

struct move_to_unique_ptr_fn {
  template <typename T>
  FOLLY_ERASE auto operator()(optional_boxed_field_ref<T> ref) const noexcept {
    return ref.release();
  }
};

struct assign_from_unique_ptr_fn {
  template <typename T>
  FOLLY_ERASE void operator()(
      optional_boxed_field_ref<T> ref,
      std::unique_ptr<typename folly::remove_cvref_t<T>::element_type> ptr)
      const noexcept {
    ref.reset(std::move(ptr));
  }
};

} // namespace detail

//  get_pointer
//
//  Access optional fields without throwing. If the field is set, it returns the
//  pointer to the field. If the field is not set, it returns nullptr.
//
//  Example:
//
//    auto* ptr = apache::thrift::get_pointer(obj.optional_field());
constexpr apache::thrift::detail::get_pointer_fn get_pointer;

//  can_throw
//
//  Used to annotate optional field accesses that can throw,
//  suppressing any linter warning about unchecked access.
//
//  Example:
//
//    auto value = apache::thrift::can_throw(*obj.field_ref());
constexpr apache::thrift::detail::can_throw_fn can_throw;

//  move_to_unique_ptr
//
//  Transfer ownership of underlying boxed field to std::unique_ptr.
//
//  Example:
//
//    auto ptr = apache::thrift::move_to_unique_ptr(obj.field_ref());
constexpr apache::thrift::detail::move_to_unique_ptr_fn move_to_unique_ptr;

//  assign_from_unique_ptr
//
//  Transfer ownership of std::unique_ptr to underlying boxed field.
//
//  Example:
//
//    apache::thrift::assign_from_unique_ptr(obj.field_ref(),
//                                           std::make_unique<int>(42));
constexpr apache::thrift::detail::assign_from_unique_ptr_fn
    assign_from_unique_ptr;

[[deprecated("Use `emplace` or `operator=` to set Thrift fields.")]] //
constexpr apache::thrift::detail::ensure_isset_unsafe_fn ensure_isset_unsafe;

constexpr apache::thrift::detail::ensure_isset_unsafe_fn
    ensure_isset_unsafe_deprecated;

[[deprecated("Use `reset` to clear Thrift fields.")]] //
constexpr apache::thrift::detail::unset_unsafe_fn unset_unsafe;

constexpr apache::thrift::detail::unset_unsafe_fn unset_unsafe_deprecated;

[[deprecated]] //
constexpr apache::thrift::detail::alias_isset_fn alias_isset;

// A reference to an required field of the possibly const-qualified type
// std::remove_reference_t<T> in a Thrift-generated struct.
template <typename T>
class required_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");

  template <typename U>
  friend class required_field_ref;

 public:
  using value_type = std::remove_reference_t<T>;
  using reference_type = T;

  FOLLY_ERASE explicit required_field_ref(reference_type value) noexcept
      : value_(value) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */ required_field_ref(
      const required_field_ref<U>& other) noexcept
      : value_(other.value_) {}

  template <typename U = value_type>
  FOLLY_ERASE std::enable_if_t<
      std::is_assignable<value_type&, U&&>::value,
      required_field_ref&>
  operator=(U&& value) noexcept(
      std::is_nothrow_assignable<value_type&, U&&>::value) {
    value_ = static_cast<U&&>(value);
    return *this;
  }

  // Workaround for https://bugs.llvm.org/show_bug.cgi?id=49442
  FOLLY_ERASE required_field_ref& operator=(value_type&& value) noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_ = static_cast<value_type&&>(value);
    return *this;
    value.~value_type(); // Force emit destructor...
  }

  // Assignment from required_field_ref is intentionally not provided to prevent
  // potential confusion between two possible behaviors, copying and reference
  // rebinding. The copy_from method is provided instead.
  template <typename U>
  FOLLY_ERASE void copy_from(required_field_ref<U> other) noexcept(
      std::is_nothrow_assignable<value_type&, U>::value) {
    value_ = other.value();
  }

  // Returns true iff the field is set. required_field_ref doesn't provide
  // conversion to bool to avoid confusion between checking if the field is set
  // and getting the field's value, particularly for bool fields.
  FOLLY_ERASE bool has_value() const noexcept { return true; }

  // Returns a reference to the value.
  FOLLY_ERASE reference_type value() const noexcept {
    return static_cast<reference_type>(value_);
  }

  FOLLY_ERASE reference_type operator*() const noexcept {
    return static_cast<reference_type>(value_);
  }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() const noexcept {
    return &value_;
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const noexcept {
    return &value_;
  }

  FOLLY_ERASE value_type* operator->() noexcept { return &value_; }

  FOLLY_ERASE reference_type ensure() noexcept {
    return static_cast<reference_type>(value_);
  }

  template <typename Index>
  FOLLY_ERASE auto operator[](const Index& index) const -> decltype(auto) {
    return value_[index];
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    return value_ = value_type(static_cast<Args&&>(args)...);
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    return value_ = value_type(ilist, static_cast<Args&&>(args)...);
  }

 private:
  value_type& value_;
};

template <typename T, typename U>
bool operator==(required_field_ref<T> lhs, required_field_ref<U> rhs) {
  return *lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(required_field_ref<T> lhs, required_field_ref<U> rhs) {
  return *lhs != *rhs;
}

template <typename T, typename U>
bool operator<(required_field_ref<T> lhs, required_field_ref<U> rhs) {
  return *lhs < *rhs;
}

template <typename T, typename U>
bool operator>(required_field_ref<T> lhs, required_field_ref<U> rhs) {
  return *lhs > *rhs;
}

template <typename T, typename U>
bool operator<=(required_field_ref<T> lhs, required_field_ref<U> rhs) {
  return *lhs <= *rhs;
}

template <typename T, typename U>
bool operator>=(required_field_ref<T> lhs, required_field_ref<U> rhs) {
  return *lhs >= *rhs;
}

template <typename T, typename U>
bool operator==(required_field_ref<T> lhs, const U& rhs) {
  return *lhs == rhs;
}

template <typename T, typename U>
bool operator!=(required_field_ref<T> lhs, const U& rhs) {
  return *lhs != rhs;
}

template <typename T, typename U>
bool operator<(required_field_ref<T> lhs, const U& rhs) {
  return *lhs < rhs;
}

template <typename T, typename U>
bool operator>(required_field_ref<T> lhs, const U& rhs) {
  return *lhs > rhs;
}

template <typename T, typename U>
bool operator<=(required_field_ref<T> lhs, const U& rhs) {
  return *lhs <= rhs;
}

template <typename T, typename U>
bool operator>=(required_field_ref<T> lhs, const U& rhs) {
  return *lhs >= rhs;
}

template <typename T, typename U>
bool operator==(const T& lhs, required_field_ref<U> rhs) {
  return lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(const T& lhs, required_field_ref<U> rhs) {
  return lhs != *rhs;
}

template <typename T, typename U>
bool operator<(const T& lhs, required_field_ref<U> rhs) {
  return lhs < *rhs;
}

template <typename T, typename U>
bool operator>(const T& lhs, required_field_ref<U> rhs) {
  return lhs > *rhs;
}

template <typename T, typename U>
bool operator<=(const T& lhs, required_field_ref<U> rhs) {
  return lhs <= *rhs;
}

template <typename T, typename U>
bool operator>=(const T& lhs, required_field_ref<U> rhs) {
  return lhs >= *rhs;
}

namespace detail {

struct union_field_ref_owner_vtable {
  using reset_t = void(void*);

  reset_t* reset;
};

struct union_field_ref_owner_vtable_impl {
  template <typename T>
  static void reset(void* obj) {
    apache::thrift::clear(*static_cast<T*>(obj));
  }
};

template <typename T>
FOLLY_INLINE_VARIABLE constexpr union_field_ref_owner_vtable //
    union_field_ref_owner_vtable_for{nullptr};
template <typename T>
FOLLY_INLINE_VARIABLE constexpr union_field_ref_owner_vtable //
    union_field_ref_owner_vtable_for<T&>{
        &union_field_ref_owner_vtable_impl::reset<T>};
template <typename T>
FOLLY_INLINE_VARIABLE constexpr union_field_ref_owner_vtable //
    union_field_ref_owner_vtable_for<const T&>{nullptr};

} // namespace detail

// A reference to an union field of the possibly const-qualified type
template <typename T>
class union_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");

  template <typename>
  friend class union_field_ref;
  friend struct detail::union_value_unsafe_fn;

  using is_cpp_ref_or_boxed = folly::bool_constant<
      detail::is_boxed_value_ptr_v<folly::remove_cvref_t<T>> ||
      detail::is_shared_or_unique_ptr_v<folly::remove_cvref_t<T>>>;

  struct element_type_adapter {
    using element_type = folly::remove_cvref_t<T>;
  };

  using element_type = typename std::conditional_t<
      is_cpp_ref_or_boxed::value,
      folly::remove_cvref_t<T>,
      element_type_adapter>::element_type;

  using storage_reference_type = T;
  using storage_value_type = std::remove_reference_t<T>;

 public:
  using value_type = detail::copy_const_t<T, element_type>;
  using reference_type = detail::copy_reference_t<T, value_type>;

 private:
  using int_t = detail::copy_const_t<T, int>;
  using owner = detail::copy_const_t<T, void>*;
  using vtable = apache::thrift::detail::union_field_ref_owner_vtable;

 public:
  FOLLY_ERASE union_field_ref(
      storage_reference_type storage_value,
      int_t& type,
      int field_type,
      owner ow,
      const vtable& vt) noexcept
      : storage_value_(storage_value),
        type_(type),
        field_type_(field_type),
        owner_(ow),
        vtable_(vt) {}

  template <
      typename U = value_type,
      std::enable_if_t<
          std::is_assignable<reference_type, U&&>::value &&
              std::is_constructible<value_type, U&&>::value,
          int> = 0>
  FOLLY_ERASE union_field_ref& operator=(U&& other) noexcept(
      std::is_nothrow_constructible<value_type, U>::value&&
          std::is_nothrow_assignable<value_type, U>::value) {
    if (has_value() &&
        !detail::is_shared_or_unique_ptr_v<folly::remove_cvref_t<T>>) {
      get_value() = static_cast<U&&>(other);
    } else {
      emplace(static_cast<U&&>(other));
    }
    return *this;
  }

  FOLLY_ERASE bool has_value() const { return type_ == field_type_; }

  FOLLY_ERASE explicit operator bool() const { return has_value(); }

  // Returns a reference to the value if this is union's active field,
  // bad_field_access otherwise.
  FOLLY_ERASE reference_type value() const {
    throw_if_unset();
    return static_cast<reference_type>(get_value());
  }

  template <typename U = std::remove_const_t<value_type>>
  FOLLY_ERASE std::remove_const_t<value_type> value_or(
      U&& default_value) const {
    using type = std::remove_const_t<value_type>;
    return has_value() ? type(static_cast<reference_type>(get_value()))
                       : type(static_cast<U&&>(default_value));
  }

  FOLLY_ERASE reference_type operator*() const { return value(); }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() const {
    throw_if_unset();
    return &get_value();
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const {
    throw_if_unset();
    return &get_value();
  }

  FOLLY_ERASE value_type* operator->() {
    throw_if_unset();
    return &get_value();
  }

  FOLLY_ERASE reference_type ensure() {
    if (!has_value()) {
      emplace();
    }
    return static_cast<reference_type>(get_value());
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    vtable_.reset(owner_);
    emplace_impl(is_cpp_ref_or_boxed{}, static_cast<Args&&>(args)...);
    type_ = field_type_;
    return get_value();
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    vtable_.reset(owner_);
    emplace_impl(is_cpp_ref_or_boxed{}, ilist, static_cast<Args&&>(args)...);
    type_ = field_type_;
    return get_value();
  }

 private:
  FOLLY_ERASE void throw_if_unset() const {
    if (!has_value()) {
      apache::thrift::detail::throw_on_bad_union_field_access();
    }
  }

  FOLLY_ERASE value_type& get_value() const {
    return get_value(is_cpp_ref_or_boxed{});
  }
  FOLLY_ERASE value_type& get_value(std::false_type) const {
    return storage_value_;
  }
  FOLLY_ERASE value_type& get_value(std::true_type) const {
    if (storage_value_ == nullptr) {
      // This can only happen if user used setter/getter to clear cpp.ref
      // pointer. It won't happen if user didn't use setter/getter API at all.
      apache::thrift::detail::throw_on_nullptr_dereferencing();
    }
    return *storage_value_;
  }

  template <class... Args>
  FOLLY_ERASE void emplace_impl(std::false_type, Args&&... args) {
    ::new (&storage_value_) storage_value_type(static_cast<Args&&>(args)...);
  }

  template <class... Args>
  FOLLY_ERASE void emplace_impl(std::true_type, Args&&... args) {
    ::new (&storage_value_) storage_value_type();
    // TODO: use make_shared to initialize cpp.ref_type = "shared" field
    storage_value_.reset(new element_type(static_cast<Args&&>(args)...));
  }

  storage_value_type& storage_value_;
  int_t& type_;
  const int field_type_;
  owner owner_;
  const vtable& vtable_;
};

template <typename T1, typename T2>
bool operator==(union_field_ref<T1> a, union_field_ref<T2> b) {
  return a && b ? *a == *b : a.has_value() == b.has_value();
}

template <typename T1, typename T2>
bool operator!=(union_field_ref<T1> a, union_field_ref<T2> b) {
  return !(a == b);
}

template <typename T1, typename T2>
bool operator<(union_field_ref<T1> a, union_field_ref<T2> b) {
  if (a.has_value() != b.has_value()) {
    return a.has_value() < b.has_value();
  }
  return a ? *a < *b : false;
}

template <typename T1, typename T2>
bool operator>(union_field_ref<T1> a, union_field_ref<T2> b) {
  return b < a;
}

template <typename T1, typename T2>
bool operator<=(union_field_ref<T1> a, union_field_ref<T2> b) {
  return !(a > b);
}

template <typename T1, typename T2>
bool operator>=(union_field_ref<T1> a, union_field_ref<T2> b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator==(union_field_ref<T> a, const U& b) {
  return a ? *a == b : false;
}

template <typename T, typename U>
bool operator!=(union_field_ref<T> a, const U& b) {
  return !(a == b);
}

template <typename T, typename U>
bool operator==(const U& a, union_field_ref<T> b) {
  return b == a;
}

template <typename T, typename U>
bool operator!=(const U& a, union_field_ref<T> b) {
  return b != a;
}

template <typename T, typename U>
bool operator<(union_field_ref<T> a, const U& b) {
  return a ? *a < b : true;
}

template <typename T, typename U>
bool operator>(union_field_ref<T> a, const U& b) {
  return a ? *a > b : false;
}

template <typename T, typename U>
bool operator<=(union_field_ref<T> a, const U& b) {
  return !(a > b);
}

template <typename T, typename U>
bool operator>=(union_field_ref<T> a, const U& b) {
  return !(a < b);
}

template <typename T, typename U>
bool operator<(const U& a, union_field_ref<T> b) {
  return b > a;
}

template <typename T, typename U>
bool operator<=(const U& a, union_field_ref<T> b) {
  return b >= a;
}

template <typename T, typename U>
bool operator>(const U& a, union_field_ref<T> b) {
  return b < a;
}

template <typename T, typename U>
bool operator>=(const U& a, union_field_ref<T> b) {
  return b <= a;
}

namespace detail {
struct union_value_unsafe_fn {
  template <typename T>
  auto&& operator()(union_field_ref<T> ref) const {
    return static_cast<typename union_field_ref<T>::reference_type>(
        ref.get_value());
  }
};
} // namespace detail

// A reference to a terse field of the possibly const-qualified type
// std::remove_reference_t<T> in a Thrift-generated struct. Note, a terse field
// does not need isset since we do not need to distinguish if the field is set
// or unset.
template <typename T>
class terse_field_ref {
  static_assert(std::is_reference<T>::value, "not a reference");

  template <typename U>
  friend class terse_field_ref;

 public:
  using value_type = std::remove_reference_t<T>;
  using reference_type = T;

  FOLLY_ERASE terse_field_ref(reference_type value) noexcept : value_(value) {}

  template <typename U, typename = detail::EnableIfImplicit<T, U>>
  FOLLY_ERASE /* implicit */ terse_field_ref(
      const terse_field_ref<U>& other) noexcept
      : value_(other.value_) {}

  template <
      typename U,
      std::enable_if_t<
          std::is_same<T, U&&>{} || std::is_same<T, const U&&>{},
          int> = 0>
  FOLLY_ERASE explicit terse_field_ref(
      const terse_field_ref<U&>& other) noexcept
      : value_(other.value_) {}

  template <typename U = value_type>
  FOLLY_ERASE std::
      enable_if_t<std::is_assignable<value_type&, U&&>::value, terse_field_ref&>
      operator=(U&& value) noexcept(
          std::is_nothrow_assignable<value_type&, U&&>::value) {
    value_ = static_cast<U&&>(value);
    return *this;
  }

  // Workaround for https://bugs.llvm.org/show_bug.cgi?id=49442
  FOLLY_ERASE terse_field_ref& operator=(value_type&& value) noexcept(
      std::is_nothrow_move_assignable<value_type>::value) {
    value_ = static_cast<value_type&&>(value);
    return *this;
    value.~value_type(); // Force emit destructor...
  }

  template <typename U>
  FOLLY_ERASE void copy_from(const terse_field_ref<U>& other) noexcept(
      std::is_nothrow_assignable<value_type&, U>::value) {
    value_ = other.value_;
  }

  template <typename U>
  FOLLY_ERASE void move_from(terse_field_ref<U> other) noexcept(
      std::is_nothrow_assignable<value_type&, std::remove_reference_t<U>&&>::
          value) {
    value_ = static_cast<std::remove_reference_t<U>&&>(other.value_);
  }

  FOLLY_ERASE reference_type value() const noexcept {
    return static_cast<reference_type>(value_);
  }

  FOLLY_ERASE reference_type operator*() const noexcept {
    return static_cast<reference_type>(value_);
  }

  template <typename U = value_type>
  [[deprecated(
      "Please use `foo.value().bar()` instead of `foo->bar()` "
      "since const is not propagated correctly in `operator->` API")]] FOLLY_ERASE
      detail::EnableIfNonConst<U>*
      operator->() const noexcept {
    return &value_;
  }

  template <typename U = value_type>
  FOLLY_ERASE detail::EnableIfConst<U>* operator->() const noexcept {
    return &value_;
  }

  FOLLY_ERASE value_type* operator->() noexcept { return &value_; }

  template <typename Index>
  FOLLY_ERASE auto operator[](const Index& index) const -> decltype(auto) {
    return value_[index];
  }

  template <typename... Args>
  FOLLY_ERASE value_type& emplace(Args&&... args) {
    value_ = value_type(static_cast<Args&&>(args)...);
    return value_;
  }

  template <class U, class... Args>
  FOLLY_ERASE std::enable_if_t<
      std::is_constructible<value_type, std::initializer_list<U>, Args&&...>::
          value,
      value_type&>
  emplace(std::initializer_list<U> ilist, Args&&... args) {
    value_ = value_type(ilist, static_cast<Args&&>(args)...);
    return value_;
  }

 private:
  value_type& value_;
};

template <typename T, typename U>
bool operator==(terse_field_ref<T> lhs, terse_field_ref<U> rhs) {
  return *lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(terse_field_ref<T> lhs, terse_field_ref<U> rhs) {
  return *lhs != *rhs;
}

template <typename T, typename U>
bool operator<(terse_field_ref<T> lhs, terse_field_ref<U> rhs) {
  return *lhs < *rhs;
}

template <typename T, typename U>
bool operator>(terse_field_ref<T> lhs, terse_field_ref<U> rhs) {
  return *lhs > *rhs;
}

template <typename T, typename U>
bool operator<=(terse_field_ref<T> lhs, terse_field_ref<U> rhs) {
  return *lhs <= *rhs;
}

template <typename T, typename U>
bool operator>=(terse_field_ref<T> lhs, terse_field_ref<U> rhs) {
  return *lhs >= *rhs;
}

template <typename T, typename U>
bool operator==(terse_field_ref<T> lhs, const U& rhs) {
  return *lhs == rhs;
}

template <typename T, typename U>
bool operator!=(terse_field_ref<T> lhs, const U& rhs) {
  return *lhs != rhs;
}

template <typename T, typename U>
bool operator<(terse_field_ref<T> lhs, const U& rhs) {
  return *lhs < rhs;
}

template <typename T, typename U>
bool operator>(terse_field_ref<T> lhs, const U& rhs) {
  return *lhs > rhs;
}

template <typename T, typename U>
bool operator<=(terse_field_ref<T> lhs, const U& rhs) {
  return *lhs <= rhs;
}

template <typename T, typename U>
bool operator>=(terse_field_ref<T> lhs, const U& rhs) {
  return *lhs >= rhs;
}

template <typename T, typename U>
bool operator==(const T& lhs, terse_field_ref<U> rhs) {
  return lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(const T& lhs, terse_field_ref<U> rhs) {
  return lhs != *rhs;
}

template <typename T, typename U>
bool operator<(const T& lhs, terse_field_ref<U> rhs) {
  return lhs < *rhs;
}

template <typename T, typename U>
bool operator>(const T& lhs, terse_field_ref<U> rhs) {
  return lhs > *rhs;
}

template <typename T, typename U>
bool operator<=(const T& lhs, terse_field_ref<U> rhs) {
  return lhs <= *rhs;
}

template <typename T, typename U>
bool operator>=(const T& lhs, terse_field_ref<U> rhs) {
  return lhs >= *rhs;
}

} // namespace thrift
} // namespace apache
