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

#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

namespace apache {
namespace thrift {
namespace type {

// TODO(afuller): Add an 'Any' class that can be any of:
// - Deserialized-Value (AnyValue)
// - Deserialized-Reference (AnyRef)
// - Serialized-Value (AnyData)
// - Serialized-Reference (AnyData?),  Such a rep would be able to
// efficiently 'snip' serialized values out of other serialized values, without
// deserializing. Note: As type::ByteBuffer is actually a folly::IOBuf, AnyData
// might already be sufficent to represent this. In which case all that would be
// needed is the 'snipping' logic.

// An Any value, that may not contain all the required information
// to deserialize the value.
//
// TODO(afuller): Add native wrappers.
using SemiAny = SemiAnyStruct;

// A serialized Any value, with all the required information to deserialize the
// value.
class AnyData : public detail::Wrap<AnyStruct> {
  using Base = detail::Wrap<AnyStruct>;

 public:
  using Base::Base;

  // Throws an std::runtime_error if the SemiAny is missing the type or
  // protocol.
  explicit AnyData(SemiAny semiAny);

  const Type& type() const { return *data_.type(); }
  const Protocol& protocol() const { return *data_.protocol(); }
  const folly::IOBuf& data() const { return *data_.data(); }
  SemiAny moveToSemiAny() &&;
  template <typename Tag, StandardProtocol protocol = StandardProtocol::Compact>
  static AnyData toAny(const native_type<Tag>&);

  template <StandardProtocol protocol = StandardProtocol::Compact, typename T>
  static AnyData toAny(const T& v) {
    return toAny<infer_tag<T>, protocol>(v);
  }

  template <typename Tag>
  void get(native_type<Tag>& t) const;

  template <typename T>
  void get(T& v) const {
    get<infer_tag<T>>(v);
  }

  bool isValid() const noexcept { return isValid(data_); }

  static bool isValid(const AnyStruct& any) noexcept;

 private:
  friend bool operator==(AnyData lhs, AnyData rhs) noexcept {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator!=(AnyData lhs, AnyData rhs) noexcept {
    return lhs.data_ != rhs.data_;
  }
  friend bool operator<(AnyData lhs, AnyData rhs) noexcept {
    return lhs.data_ < rhs.data_;
  }

  [[noreturn]] static void throwTypeMismatchException(
      const Type& want, const Type& actual);
};

template <typename Tag, StandardProtocol Protocol>
AnyData AnyData::toAny(const native_type<Tag>& v) {
  static_assert(
      Protocol == StandardProtocol::Binary ||
          Protocol == StandardProtocol::Compact,
      "Unsupported protocol");

  using Writer = std::conditional_t<
      Protocol == StandardProtocol::Binary,
      BinaryProtocolWriter,
      CompactProtocolWriter>;

  Writer writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  ::apache::thrift::op::encode<Tag>(writer, v);

  SemiAny builder;
  builder.data() = queue.moveAsValue();
  builder.protocol() = Protocol;
  builder.type() = Tag{};
  return AnyData{std::move(builder)};
}

template <typename Tag>
void AnyData::get(native_type<Tag>& v) const {
  if (type() != Type{Tag{}}) {
    throwTypeMismatchException(Type{Tag{}}, type());
  }

  if (protocol() == Protocol::get<StandardProtocol::Binary>()) {
    BinaryProtocolReader reader;
    reader.setInput(&data());
    op::decode<Tag>(reader, v);
  } else if (protocol() == Protocol::get<StandardProtocol::Compact>()) {
    CompactProtocolReader reader;
    reader.setInput(&data());
    op::decode<Tag>(reader, v);
  } else {
    folly::throw_exception<std::runtime_error>(
        "Unsupported protocol: " + std::string(protocol().name()));
  }
}

inline bool AnyData::isValid(const AnyStruct& any) noexcept {
  if (!any.data().is_set()) {
    return true;
  }
  return any.type()->isValid() && !any.protocol()->empty();
}

} // namespace type

template <>
class Cpp2Ops<type::AnyData> {
 private:
  using S = apache::thrift::type::AnyStruct;

 public:
  using T = type::AnyData;

  template <class P>
  static uint32_t write(P* p, const T* t) {
    return Cpp2Ops<S>::write(p, &t->toThrift());
  }

  template <class P>
  static void read(P* p, T* t) {
    return Cpp2Ops<S>::read(p, &t->toThrift());
  }

  template <class P>
  static uint32_t serializedSize(const P* p, const T* t) {
    return Cpp2Ops<S>::serializedSize(p, &t->toThrift());
  }

  template <class P>
  static uint32_t serializedSizeZC(const P* p, const T* t) {
    return Cpp2Ops<S>::serializedSizeZC(p, &t->toThrift());
  }

  static constexpr apache::thrift::protocol::TType thriftType() {
    return Cpp2Ops<S>::thriftType();
  }
};

} // namespace thrift
} // namespace apache
