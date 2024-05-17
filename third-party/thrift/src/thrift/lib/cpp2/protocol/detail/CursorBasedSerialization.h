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

#include <string.h>
#include <array>
#include <utility>

#include <folly/lang/Bits.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

namespace apache::thrift {

template <typename T, bool Contiguous>
class StructuredCursorReader;
template <typename Tag>
class ContainerCursorReader;
template <typename Tag>
class ContainerCursorIterator;

template <typename T>
class StructuredCursorWriter;
class StringCursorWriter;

namespace detail {

template <typename T>
T& maybe_emplace(T& t) {
  return t;
}
template <typename T>
T& maybe_emplace(std::optional<T>& t) {
  return t.emplace();
}

template <typename Tag>
struct ContainerTraits;
template <typename VTag>
struct ContainerTraits<type::list<VTag>> {
  using ElementType = type::native_type<VTag>;
  using ValueTag = VTag;
  // This is initializer_list becuase that's what skip_n accepts.
  static constexpr std::initializer_list<protocol::TType> wireTypes = {
      op::typeTagToTType<ValueTag>};
};
template <typename KTag>
struct ContainerTraits<type::set<KTag>> {
  using ElementType = type::native_type<KTag>;
  using KeyTag = KTag;
  static constexpr std::initializer_list<protocol::TType> wireTypes = {
      op::typeTagToTType<KeyTag>};
};
template <typename KTag, typename VTag>
struct ContainerTraits<type::map<KTag, VTag>> {
  using ElementType =
      std::pair<type::native_type<KTag>, type::native_type<VTag>>;
  using KeyTag = KTag;
  using ValueTag = VTag;
  static constexpr std::initializer_list<protocol::TType> wireTypes = {
      op::typeTagToTType<KeyTag>, op::typeTagToTType<ValueTag>};
};

class BaseCursorReader {
 protected:
  BinaryProtocolReader protocol_;
  enum class State {
    Active,
    Child, // Reading a nested struct or container
    Done,
  };
  State state_ = State::Active;

  void checkState(State expected) const {
    if (state_ != expected) {
      folly::throw_exception<std::runtime_error>([&]() {
        switch (state_) {
          case State::Active:
            return "No child reader is active";
          case State::Child:
            return "Child reader not passed to endRead";
          case State::Done:
            return "Reader already finalized";
        }
      }());
    }
  }

  explicit BaseCursorReader(BinaryProtocolReader&& p)
      : protocol_(std::move(p)) {}
  BaseCursorReader() = default;

  ~BaseCursorReader() {
    DCHECK(state_ == State::Done) << "Reader must be passed to endRead";
  }

  BaseCursorReader(BaseCursorReader&& other) noexcept {
    protocol_ = std::move(other.protocol_);
    state_ = other.state_;
    other.state_ = State::Done;
  }
  BaseCursorReader& operator=(BaseCursorReader&& other) noexcept {
    if (this != &other) {
      DCHECK(state_ == State::Done) << "Reader must be passed to endRead";
      protocol_ = std::move(other.protocol_);
      state_ = other.state_;
      other.state_ = State::Done;
    }
    return *this;
  }

 public:
  BaseCursorReader(const BaseCursorReader&) = delete;
  BaseCursorReader& operator=(const BaseCursorReader&) = delete;
};

class BaseCursorWriter {
 protected:
  BinaryProtocolWriter protocol_;
  enum class State {
    Active,
    Child,
    Done,
  };
  State state_ = State::Active;

  explicit BaseCursorWriter(BinaryProtocolWriter&& p)
      : protocol_(std::move(p)) {}

  void checkState(State expected) const {
    if (state_ != expected) {
      folly::throw_exception<std::runtime_error>([&]() {
        switch (state_) {
          case State::Active:
            return "No child writer is active";
          case State::Child:
            return "Child writer not passed to endWrite";
          case State::Done:
            return "Writer already finalized";
        }
      }());
    }
  }

  ~BaseCursorWriter() {
    DCHECK(state_ == State::Done) << "Writer must be passed to endWrite";
  }

  BaseCursorWriter(BaseCursorWriter&& other) noexcept {
    protocol_ = std::move(other.protocol_);
    state_ = other.state_;
    other.state_ = State::Done;
  }
  BaseCursorWriter& operator=(BaseCursorWriter&& other) noexcept {
    if (this != &other) {
      DCHECK(state_ == State::Done) << "Writer must be passed to endWrite";
      protocol_ = std::move(other.protocol_);
      state_ = other.state_;
      other.state_ = State::Done;
    }
    return *this;
  }

 public:
  BaseCursorWriter(const BaseCursorWriter&) = delete;
  BaseCursorWriter& operator=(const BaseCursorWriter&) = delete;

  template <typename T>
  friend class StructuredCursorWriter;
};

// std::swap isn't constexpr until C++20 so we need to reimplement :(
template <typename T>
constexpr void constexprSwap(T& a, T& b) {
  T tmp = std::move(a);
  a = std::move(b);
  b = std::move(tmp);
}

// std::is_sorted isn't constexpr until C++20 so we need to reimplement :(
template <typename T, size_t N>
constexpr bool constexprIsSorted(const std::array<T, N>& array) {
  for (size_t i = 1; i < N; ++i) {
    if (array[i] < array[i - 1]) {
      return false;
    }
  }
  return true;
}

// std::sort isn't constexpr until C++20 so we need to reimplement :(
template <typename T, size_t N>
constexpr void constexprQuickSort(
    std::array<T, N>& array, ssize_t min_idx, ssize_t max_idx) {
  if (max_idx <= min_idx) {
    return;
  }

  T pivot_value = array[min_idx];
  ssize_t fwd_idx = min_idx;
  ssize_t rev_idx = max_idx + 1;
  while (true) {
    while (array[++fwd_idx] < pivot_value && fwd_idx != max_idx) {
    }
    while (pivot_value < array[--rev_idx] && rev_idx != min_idx) {
    }
    if (fwd_idx >= rev_idx) {
      break;
    }

    constexprSwap(array[fwd_idx], array[rev_idx]);
  }

  constexprSwap(array[min_idx], array[rev_idx]);

  constexprQuickSort(array, min_idx, rev_idx - 1);
  constexprQuickSort(array, rev_idx + 1, max_idx);
}

template <typename Tag>
struct DefaultValueWriter {
  using T = type::native_type<Tag>;
  struct Field {
    FieldId id;
    void (*write)(StructuredCursorWriter<Tag>&) = nullptr;
    constexpr bool operator<(const Field& other) const { return id < other.id; }
  };

  static constexpr std::array<Field, op::size_v<T>> fields = [] {
    std::array<Field, op::size_v<T>> fields;
    op::for_each_ordinal<T>([&](auto ord) {
      using Ord = decltype(ord);
      using Id = op::get_field_id<T, Ord>;
      using FTag = op::get_type_tag<T, Ord>;
      fields[type::toPosition(Ord::value)] = {
          Id::value, [](StructuredCursorWriter<Tag>& writer) {
            if constexpr (type::is_optional_or_union_field_v<T, Ord>) {
              return;
            }
            const auto& val = *op::get<Id>(op::getDefault<T>());
            writer.template writeField<Id>(
                [&] { op::encode<FTag>(writer.protocol_, val); }, val);
          }};
    });
    constexprQuickSort(fields, 0, fields.size() - 1);
    assert(constexprIsSorted(fields));
    return fields;
  }();
};

inline constexpr FieldId increment(FieldId id) {
  assert(folly::to_underlying(id) + 1 > folly::to_underlying(id));
  return static_cast<FieldId>(folly::to_underlying(id) + 1);
}

/** Supports writing containers whose size is not known until after
 * serialization. */
class DelayedSizeCursorWriter : public BaseCursorWriter {
 protected:
  void* size_;

  constexpr static size_t kSizeLen = 4;

  explicit DelayedSizeCursorWriter(BinaryProtocolWriter&& p)
      : BaseCursorWriter(std::move(p)) {}

  void writeSize() {
    static_assert(
        std::is_same_v<decltype(protocol_), BinaryProtocolWriter>,
        "Using internals of binary protocol.");
    size_ = protocol_.ensure(kSizeLen);
    protocol_.advance(kSizeLen);
  }

  void finalize(int32_t actualSize) {
    checkState(State::Active);
    state_ = State::Done;
    actualSize = folly::Endian::big(actualSize);
    memcpy(size_, &actualSize, kSizeLen);
  }
};

} // namespace detail
} // namespace apache::thrift
