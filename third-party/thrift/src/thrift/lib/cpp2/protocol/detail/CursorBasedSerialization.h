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

template <typename T>
struct LastFieldId {
  static constexpr FieldId value = [] {
    FieldId max = FieldId{0};
    op::for_each_ordinal<T>([&](auto ord) {
      using Ord = decltype(ord);
      max = std::max(max, op::get_field_id_v<T, Ord>);
    });
    return max;
  }();
};

template <typename T>
constexpr auto FieldAfterLast =
    FieldId{folly::to_underlying(LastFieldId<T>::value) + 1};

template <typename T, typename ProtocolReader, bool Contiguous>
class StructuredCursorReader;
template <typename Tag, typename ProtocolReader, bool Contiguous>
class ContainerCursorReader;
template <typename Tag, typename ProtocolReader, bool Contiguous>
class ContainerCursorIterator;

template <typename T>
class StructuredCursorWriter;
template <typename Tag>
class ContainerCursorWriter;
class StringCursorWriter;
class ManagedStringViewWithConversions;

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
  using ElementTag = VTag;
  // This is initializer_list because that's what skip_n accepts.
  static constexpr std::initializer_list<protocol::TType> wireTypes = {
      op::typeTagToTType<ElementTag>};

  static void write(BinaryProtocolWriter& protocol, const ElementType& value) {
    op::encode<VTag>(protocol, value);
  }
};
template <typename KTag>
struct ContainerTraits<type::set<KTag>> {
  using ElementType = type::native_type<KTag>;
  using ElementTag = KTag;
  static constexpr std::initializer_list<protocol::TType> wireTypes = {
      op::typeTagToTType<ElementTag>};

  static void write(BinaryProtocolWriter& protocol, const ElementType& value) {
    op::encode<KTag>(protocol, value);
  }
};
template <typename KTag, typename VTag>
struct ContainerTraits<type::map<KTag, VTag>> {
  using ElementType =
      std::pair<type::native_type<KTag>, type::native_type<VTag>>;
  using ElementTag = KTag;
  using KeyTag = KTag;
  using ValueTag = VTag;
  static constexpr std::initializer_list<protocol::TType> wireTypes = {
      op::typeTagToTType<KeyTag>, op::typeTagToTType<ValueTag>};

  static void write(BinaryProtocolWriter& protocol, const ElementType& key) {
    op::encode<KTag>(protocol, key.first);
    op::encode<VTag>(protocol, key.second);
  }
};
template <typename Tag, typename Type>
struct ContainerTraits<type::cpp_type<Type, Tag>> : ContainerTraits<Tag> {};

template <typename ProtocolReader>
class BaseCursorReader {
 protected:
  ProtocolReader* protocol_;
  enum class State {
    Active,
    Child, // Reading a nested struct or container
    Abandoned,
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
          case State::Abandoned:
            return "Reader abandoned";
          case State::Done:
            return "Reader already finalized";
          default:
            return "State is unknown";
        }
      }());
    }
  }

  explicit BaseCursorReader(ProtocolReader* p) : protocol_(p) {}
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

  void abandon() {
    if (state_ != State::Active && state_ != State::Abandoned) {
      folly::throw_exception<std::runtime_error>(
          "Unexpected state when abandoning reader");
    }
    state_ = State::Done;
  }

 public:
  BaseCursorReader(const BaseCursorReader&) = delete;
  BaseCursorReader& operator=(const BaseCursorReader&) = delete;
};

template <typename ProtocolWriter>
class BaseCursorWriter {
 protected:
  ProtocolWriter* protocol_;
  enum class State {
    Active,
    Child,
    Done,
    Abandoned,
  };
  State state_ = State::Active;

  explicit BaseCursorWriter(ProtocolWriter* p) : protocol_(p) {}

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
          default:
            return "State is unknown";
        }
      }());
    }
  }

  ~BaseCursorWriter() {
    DCHECK(state_ == State::Done || state_ == State::Abandoned)
        << "Writer must be passed to endWrite";
  }

  BaseCursorWriter(BaseCursorWriter&& other) noexcept {
    protocol_ = std::move(other.protocol_);
    state_ = other.state_;
    other.state_ = State::Done;
  }
  BaseCursorWriter& operator=(BaseCursorWriter&& other) noexcept {
    if (this != &other) {
      DCHECK(state_ == State::Done || state_ == State::Abandoned)
          << "Writer must be passed to endWrite";
      protocol_ = std::move(other.protocol_);
      state_ = other.state_;
      other.state_ = State::Done;
    }
    return *this;
  }

  void abandon() {
    if (state_ != State::Active && state_ != State::Abandoned) {
      folly::throw_exception<std::runtime_error>(
          "Unexpected state when abandoning writer");
    }
    state_ = State::Done;
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

  static constexpr std::array<Field, op::num_fields<T>> fields = [] {
    std::array<Field, op::num_fields<T>> fields{};
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
                [&] { op::encode<FTag>(*writer.protocol_, val); }, val);
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
class DelayedSizeCursorWriter : public BaseCursorWriter<BinaryProtocolWriter> {
 protected:
  void* size_;

  constexpr static size_t kSizeLen = 4;

  explicit DelayedSizeCursorWriter(BinaryProtocolWriter* p)
      : BaseCursorWriter(p) {}

  void writeSize() {
    static_assert(
        std::is_same_v<decltype(protocol_), BinaryProtocolWriter*>,
        "Using internals of binary protocol.");
    size_ = protocol_->ensure(kSizeLen);
    protocol_->advance(kSizeLen);
  }

  void finalize(int32_t actualSize) {
    checkState(State::Active);
    state_ = State::Done;
    actualSize = folly::Endian::big(actualSize);
    memcpy(size_, &actualSize, kSizeLen);
  }
};

/** Converts std::string to std::string_view when Contiguous = true, and returns
 * the original type otherwise. */
template <typename T, bool Contiguous>
using lift_view_t = std::conditional_t<
    Contiguous && std::is_same_v<T, std::string>,
    std::string_view,
    T>;

template <typename ProtocolReader>
std::string_view readStringView(ProtocolReader& protocol) {
  int32_t size;
  protocol.readI32(size);
  if (size < 0) {
    TProtocolException::throwNegativeSize();
  }
  folly::io::Cursor c = protocol.getCursor();
  if (static_cast<size_t>(size) >= c.length()) {
    TProtocolException::throwTruncatedData();
  }
  protocol.skipBytes(size);
  return std::string_view(reinterpret_cast<const char*>(c.data()), size);
}

template <typename Tag, typename ProtocolReader, typename T>
void decodeTo(ProtocolReader& protocol, T& t) {
  if constexpr (
      std::is_same_v<T, std::string_view> &&
      type::is_a_v<Tag, type::string_c>) {
    t = readStringView(protocol);
  } else {
    op::decode<Tag>(protocol, t);
  }
}

template <typename T, typename = void>
struct HasValueType {
  constexpr static bool value = false;
};
template <typename T>
struct HasValueType<T, std::void_t<typename T::value_type>> {
  constexpr static bool value = true;
};

template <typename Tag>
struct IsSupportedCppType {
  constexpr static bool value = true;
};
template <typename T, typename Tag>
struct IsSupportedCppType<type::cpp_type<T, Tag>> {
  constexpr static bool value = [] {
    if constexpr (type::is_a_v<Tag, type::integral_c>) {
      return std::is_integral_v<T>;
    } else if constexpr (type::is_a_v<Tag, type::string_c>) {
      return std::is_same_v<T, folly::IOBuf> ||
          std::is_same_v<T, std::unique_ptr<folly::IOBuf>> ||
          std::is_same_v<T, folly::fbstring> ||
          std::is_same_v<T, std::string> ||
          std::is_same_v<T, ManagedStringViewWithConversions>;
    } else if constexpr (type::is_a_v<Tag, type::container_c>) {
      return HasValueType<T>::value;
    }
    return false;
  }();
};

template <typename T>
constexpr bool validateCppTypes() {
  op::for_each_ordinal<T>([](auto ord) {
    using Ord = decltype(ord);
    static_assert(
        IsSupportedCppType<op::get_type_tag<T, Ord>>::value,
        "Unsupported cpp.Type. Consider using cpp.Adapter instead.");
  });
  return true;
}

} // namespace detail
} // namespace apache::thrift
