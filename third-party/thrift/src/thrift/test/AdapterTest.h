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

#include <chrono>
#include <stdexcept>
#include <string>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift::test {

namespace basic {
enum class AdaptedEnum {
  Zero = 0,
  One,
  Two,
};
}

template <typename T>
struct Wrapper {
  T value;

  Wrapper& operator=(const T&) = delete;

  bool operator==(const Wrapper& other) const { return value == other.value; }
  bool operator<(const Wrapper& other) const { return value < other.value; }
};

struct TemplatedTestAdapter {
  template <typename T>
  static Wrapper<T> fromThrift(T value) {
    return {value};
  }

  template <typename Wrapper>
  static auto&& toThrift(Wrapper&& wrapper) {
    return std::forward<Wrapper>(wrapper).value;
  }
};

struct AdaptTestMsAdapter {
  static std::chrono::milliseconds fromThrift(int64_t ms) {
    return std::chrono::milliseconds{ms};
  }

  static int64_t toThrift(std::chrono::milliseconds duration) {
    return duration.count();
  }
};

struct Num {
  int64_t val = 13;

 private:
  friend bool operator==(const Num& lhs, const Num& rhs) {
    return lhs.val == rhs.val;
  }
  friend bool operator<(const Num& lhs, const Num& rhs) {
    return lhs.val < rhs.val;
  }
};

struct String {
  std::string val;
};

template <class T>
struct Indirection {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(val);
  T val;
};

using IndirectionString = Indirection<std::string>;
using IndirectionIOBuf = Indirection<folly::IOBuf>;

struct OverloadedAdapter {
  static Num fromThrift(int64_t val) { return Num{val}; }
  static String fromThrift(std::string&& val) { return String{std::move(val)}; }

  static int64_t toThrift(const Num& num) { return num.val; }
  static const std::string& toThrift(const String& str) { return str.val; }
};

struct CustomProtocolAdapter {
  static Num fromThrift(const folly::IOBuf& data) {
    if (data.length() < sizeof(int64_t)) {
      throw std::invalid_argument("CustomProtocolAdapter parse error");
    }
    return {*reinterpret_cast<const int64_t*>(data.data())};
  }

  static folly::IOBuf toThrift(const Num& num) {
    return folly::IOBuf::wrapBufferAsValue(&num.val, sizeof(int64_t));
  }
};

// TODO(afuller): Move this to a shared location.
template <typename AdaptedT>
struct IndirectionAdapter {
  template <typename ThriftT>
  FOLLY_ERASE static constexpr AdaptedT fromThrift(ThriftT&& value) {
    AdaptedT adapted;
    toThrift(adapted) = std::forward<ThriftT>(value);
    return adapted;
  }
  FOLLY_ERASE static constexpr decltype(auto)
  toThrift(AdaptedT& adapted) noexcept(
      noexcept(::apache::thrift::apply_indirection(adapted))) {
    return ::apache::thrift::apply_indirection(adapted);
  }
};

template <typename T, typename Struct, int16_t FieldId>
struct AdaptedWithContext {
  T value = {};
  int16_t fieldId = 0;
  std::string* meta = nullptr;
  AdaptedWithContext() = default;
  explicit AdaptedWithContext(T value_) : value(value_) {}
  AdaptedWithContext(T value_, int16_t fieldId_, std::string* meta_)
      : value(value_), fieldId(fieldId_), meta(meta_) {}
};

template <typename T, typename Struct, int16_t FieldId>
inline bool operator==(
    const AdaptedWithContext<T, Struct, FieldId>& lhs,
    const AdaptedWithContext<T, Struct, FieldId>& rhs) {
  return lhs.value == rhs.value;
}

template <typename T, typename Struct, int16_t FieldId>
inline bool operator<(
    const AdaptedWithContext<T, Struct, FieldId>& lhs,
    const AdaptedWithContext<T, Struct, FieldId>& rhs) {
  return lhs.value < rhs.value;
}

struct TemplatedTestFieldAdapter {
  template <typename T, typename Struct, int16_t FieldId>
  static void construct(
      AdaptedWithContext<T, Struct, FieldId>& field,
      apache::thrift::FieldContext<Struct, FieldId>&&) {
    field.fieldId = apache::thrift::FieldContext<Struct, FieldId>::kFieldId;
  }

  template <typename T, typename Struct, int16_t FieldId>
  static AdaptedWithContext<T, Struct, FieldId> fromThriftField(
      T value, apache::thrift::FieldContext<Struct, FieldId>&&) {
    return {
        value,
        apache::thrift::FieldContext<Struct, FieldId>::kFieldId,
        nullptr,
    };
  }

  template <typename T, typename Struct, int16_t FieldId>
  static T toThrift(const AdaptedWithContext<T, Struct, FieldId>& adapted) {
    return adapted.value;
  }
};

struct AdapterWithContext {
  template <typename T, typename Struct, int16_t FieldId>
  static void construct(
      AdaptedWithContext<T, Struct, FieldId>& field,
      apache::thrift::FieldContext<Struct, FieldId>&& ctx) {
    field.fieldId = apache::thrift::FieldContext<Struct, FieldId>::kFieldId;
    field.meta = &*ctx.object.meta_ref();
  }

  template <typename T, typename Struct, int16_t FieldId>
  static AdaptedWithContext<T, Struct, FieldId> fromThriftField(
      T value, apache::thrift::FieldContext<Struct, FieldId>&& ctx) {
    return {
        value,
        apache::thrift::FieldContext<Struct, FieldId>::kFieldId,
        &*ctx.object.meta_ref()};
  }

  template <typename T, typename Struct, int16_t FieldId>
  static T toThrift(const AdaptedWithContext<T, Struct, FieldId>& adapted) {
    return adapted.value;
  }
};

struct AdapterWithContextOptimized {
  template <typename T, typename Struct, int16_t FieldId>
  static void construct(
      AdaptedWithContext<T, Struct, FieldId>& field,
      apache::thrift::FieldContext<Struct, FieldId>&& ctx) {
    field.fieldId = apache::thrift::FieldContext<Struct, FieldId>::kFieldId;
    field.meta = &*ctx.object.meta_ref();
  }

  template <typename T, typename Struct, int16_t FieldId>
  static AdaptedWithContext<T, Struct, FieldId> fromThriftField(
      T value, apache::thrift::FieldContext<Struct, FieldId>&& ctx) {
    return {
        value,
        apache::thrift::FieldContext<Struct, FieldId>::kFieldId,
        &*ctx.object.meta_ref()};
  }

  template <typename T, typename Struct, int16_t FieldId>
  static T toThrift(const AdaptedWithContext<T, Struct, FieldId>& adapted) {
    return adapted.value;
  }

  template <typename T, typename Struct, int16_t FieldId>
  static void clear(AdaptedWithContext<T, Struct, FieldId>& field) {
    field.value = {};
  }

  template <typename T, typename Struct, int16_t FieldId>
  static bool isEmpty(const AdaptedWithContext<T, Struct, FieldId>&) {
    return false;
  }
};

template <class T>
struct IdentityAdapter {
  static T fromThrift(T i) { return std::move(i); }
  static const T& toThrift(const T& i) { return i; }
};

template <
    class T,
    class U = std::remove_reference_t<decltype(*std::declval<T>().field())>>
struct TaggedWrapper;

struct MemberAccessAdapter {
  template <class T>
  static TaggedWrapper<T> fromThrift(const T& t) {
    return {*t.field()};
  }
  template <class T>
  static T toThrift(const TaggedWrapper<T>& w) {
    T ret;
    ret.field() = w.val;
    return ret;
  }
};

template <class T, class U>
struct TaggedWrapper {
  U val;

  bool operator==(const TaggedWrapper& other) const { return val == other.val; }
  bool operator<(const TaggedWrapper& other) const { return val < other.val; }
};

struct MoveOnlyAdapter {
  template <class T>
  static std::unique_ptr<T> fromThrift(const T& i) {
    return std::make_unique<T>(i);
  }
  template <class T>
  static const T& toThrift(const std::unique_ptr<T>& i) {
    return *i;
  }
};

template <bool hasSerializedSize, typename T>
struct CountingAdapter {
  static inline int count = 0;
  static T fromThrift(T i) { return i; }
  static T fromThriftField(T i) { return i; }
  static T toThrift(T i) {
    ++count;
    return i;
  }
  template <bool ZC, typename Protocol, bool Enable = hasSerializedSize>
  static std::enable_if_t<Enable, uint32_t> serializedSize(Protocol& prot, T) {
    return prot.serializedSizeI64();
  }
};

template <typename T>
struct VariableWrapper {
  VariableWrapper(const VariableWrapper&) = delete;

  VariableWrapper(const T& v) : value{v} {}
  VariableWrapper(const T& v, std::string n, std::string u)
      : value{v}, name{n}, uri{u} {}

  T value;
  std::string name = "";
  std::string uri = "";
};

struct VariableAdapter {
  // This is necessary as type hint, since adapted_t doesn't support forwarding
  // annotation
  template <typename T>
  static VariableWrapper<T> fromThrift(T value) {
    return {value};
  }

  template <typename T, typename Anno>
  static VariableWrapper<T> fromThrift(
      T value, Anno annotation, std::string uri) {
    return {value, *annotation.name(), uri};
  }
};

template <typename T>
struct NonComparableWrapper {
  T value;
};

struct NonComparableWrapperAdapter {
  template <typename T>
  static NonComparableWrapper<T> fromThrift(T value) {
    return {value};
  }

  template <typename Wrapper>
  static auto&& toThrift(Wrapper&& wrapper) {
    return std::forward<Wrapper>(wrapper).value;
  }
};

class Timestamp {
 public:
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(value());

  uint64_t value() const { return v_; }
  uint64_t& value() { return v_; }

  bool operator==(Timestamp that) const { return v_ == that.v_; }
  bool operator<(Timestamp that) const { return v_ < that.v_; }

 private:
  uint64_t v_{0};
};

} // namespace apache::thrift::test
