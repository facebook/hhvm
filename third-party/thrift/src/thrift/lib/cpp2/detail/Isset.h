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
#include <cassert>
#include <type_traits>
#include <folly/synchronization/AtomicUtil.h>

namespace apache::thrift::detail {

// IntWrapper is a wrapper of integer that's always copy/move assignable
// even if integer is atomic
template <typename T>
struct IntWrapper {
  static_assert(
      !std::is_reference_v<T>,
      "IntWrapper cannot wrap reference types. "
      "Wrapping a reference would create a dangling reference: "
      "`T& value{0}` creates a temporary and binds to it, which is "
      "immediately destroyed, causing heap-use-after-free.");

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
            other.is_atomic_ ? IssetBitSet(*other.value_.atomic_ptr)
                             : IssetBitSet(*other.value_.non_atomic_ptr)),
        bit_index_(other.bit_index_),
        is_atomic_(other.is_atomic_) {}

#if FOLLY_MOBILE
  // We have this attribute to prevent binary size regression
  // TODO: Remove special attribute for MOBILE
  FOLLY_ERASE
#endif
  void operator=(bool flag) {
    if (is_atomic_) {
      setBit(*value_.atomic_ptr, bit_index_, flag);
    } else {
      setBit(*value_.non_atomic_ptr, bit_index_, flag);
    }
  }

  explicit operator bool() const {
    if (is_atomic_) {
      return getBit(*value_.atomic_ptr, bit_index_);
    } else {
      return getBit(*value_.non_atomic_ptr, bit_index_);
    }
  }

 private:
  static bool getBit(const uint8_t& isset, uint8_t bit) {
    return isset & (uint8_t(1) << bit);
  }

  static bool getBit(const std::atomic<uint8_t>& isset, uint8_t bit) {
    return isset.load(std::memory_order_relaxed) & (uint8_t(1) << bit);
  }

  static void setBit(uint8_t& isset, uint8_t bit, bool flag) {
    if (flag) {
      isset |= (uint8_t(1) << bit);
    } else {
      isset &= ~(uint8_t(1) << bit);
    }
  }

  static void setBit(std::atomic<uint8_t>& isset, uint8_t bit, bool flag) {
    if (flag) {
      folly::atomic_fetch_set(isset, bit, std::memory_order_relaxed);
    } else {
      folly::atomic_fetch_reset(isset, bit, std::memory_order_relaxed);
    }
  }

  union IssetBitSet {
    explicit IssetBitSet(Isset& isset) : non_atomic_ptr(&isset) {}
    explicit IssetBitSet(AtomicIsset& isset) : atomic_ptr(&isset) {}
    Isset* non_atomic_ptr;
    AtomicIsset* atomic_ptr;
  } value_;

  const uint8_t bit_index_;
  const bool is_atomic_ = false;
};

enum class IssetBitsetOption {
  Unpacked,
  Packed,
  PackedWithAtomic,
};

template <
    size_t NumBits,
    IssetBitsetOption kOption = IssetBitsetOption::Unpacked>
class isset_bitset {
 private:
  using IntType = std::conditional_t<
      kOption == IssetBitsetOption::PackedWithAtomic,
      std::atomic<uint8_t>,
      uint8_t>;

 public:
  bool get(size_t field_index) const {
    check(field_index);
    return array_isset[field_index / kBits][field_index % kBits];
  }

  void set(size_t field_index, bool isset_flag) {
    check(field_index);
    array_isset[field_index / kBits][field_index % kBits] = isset_flag;
  }

  const IntType& at(size_t field_index) const {
    check(field_index);
    return array_isset[field_index / kBits].value();
  }

  IntType& at(size_t field_index) {
    check(field_index);
    return array_isset[field_index / kBits].value();
  }

  uint8_t bit(size_t field_index) const {
    check(field_index);
    return field_index % kBits;
  }

  static constexpr ptrdiff_t get_offset() {
    return offsetof(isset_bitset, array_isset);
  }

 private:
  static void check([[maybe_unused]] size_t field_index) {
    assert(field_index / kBits < NumBits);
  }

  static constexpr size_t kBits =
      kOption == IssetBitsetOption::Unpacked ? 1 : 8;
  std::array<
      apache::thrift::detail::BitSet<IntType>,
      (NumBits + kBits - 1) / kBits>
      array_isset;
};

} // namespace apache::thrift::detail
