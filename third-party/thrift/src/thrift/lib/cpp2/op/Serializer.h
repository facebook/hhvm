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

#include <any>
#include <string>
#include <string_view>
#include <typeinfo>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/type/AnyValue.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Runtime.h>

namespace apache::thrift::op {

// A serializer interface for any type.
//
// For example, to encode an AnyValue, you would:
//    const Serializer& serializer = ...
//    folly::io::QueueAppender appender = ...
//    serializer.encode(myValue, appender);
//
// Then to decode it later:
//    folly::io::Cursor cursor = ...
//    AnyValue myValue = serializer.decode(myType, cursor);
//
class Serializer {
 public:
  virtual ~Serializer() = default;

  // The protocol used by this serializer.
  virtual const type::Protocol& getProtocol() const = 0;

  // Encode a value.
  //
  // Throws std::bad_any_cast if the type is not supported.
  // TODO(afuller): Figure out if appender should really be accepted by r-value,
  // which was copied from the *Protocol::setOutput interface.
  virtual void encode(
      type::ConstRef value, folly::io::QueueAppender&& appender) const = 0;
  virtual void encode(
      type::AnyConstRef value, folly::io::QueueAppender&& appender) const = 0;

  // Decode a value.
  //
  // Throws std::bad_any_cast if the type is not supported.
  // TODO(afuller): Consider accepting a cursor by value, and returning a new˝
  // cursor instead of having one in/out param.
  virtual void decode(folly::io::Cursor& cursor, type::Ref value) const = 0;
  virtual void decode(
      const type::Type& type,
      folly::io::Cursor& cursor,
      type::AnyValue& value) const = 0;
  virtual void decode(folly::io::Cursor& cursor, type::AnyRef value) const = 0;
  type::AnyValue decode(
      const type::Type& type, folly::io::Cursor& cursor) const;
  void decode(folly::io::Cursor& cursor, type::AnyValue& value) const {
    decode(value.type(), cursor, value);
  }
  template <typename Tag>
  type::native_type<Tag> decode(folly::io::Cursor& cursor) const {
    type::native_type<Tag> result;
    decode(cursor, type::Ref::to<Tag>(result));
    return result;
  }

 protected:
  // Throws std::bad_any_cast if the types are not equal.
  static void checkType(const type::Type& actual, const type::Type& expected);
};

// A base class for a serializer of a single type Tag.
//
// Implementations provide the following type-specific functions:
// - void encode(const T&, folly::io::QueueAppender&&) const
// - void decode(folly::io::Cursor&, T&) const
// where T = type::native_type<Tag>
template <typename Tag>
class TagSerializer : public Serializer {
  using Base = Serializer;
  using T = type::native_type<Tag>;

 public:
  using Base::encode;
  void encode(
      type::ConstRef value, folly::io::QueueAppender&& appender) const final {
    encode(value.as<Tag>(), std::move(appender));
  }
  void encode(type::AnyConstRef value, folly::io::QueueAppender&& appender)
      const final {
    encode(value.as<Tag>(), std::move(appender));
  }
  virtual void encode(
      const T& value, folly::io::QueueAppender&& appender) const = 0;

  using Base::decode;
  void decode(folly::io::Cursor& cursor, type::Ref value) const final {
    checkType(value.type(), type::Type::get<Tag>());
    decode(cursor, value.mut<Tag>());
  }
  void decode(
      const type::Type& type,
      folly::io::Cursor& cursor,
      type::AnyValue& value) const final {
    checkType(type, type::Type::get<Tag>());
    if (auto* obj = value.try_as<Tag>()) {
      decode(cursor, *obj);
    } else {
      value = type::AnyValue::create<Tag>();
      decode(cursor, value.as<Tag>());
    }
  }

  void decode(folly::io::Cursor& cursor, type::AnyRef value) const final {
    checkType(value.type(), type::Type::get<Tag>());
    decode(cursor, value.as<Tag>());
  }
  virtual void decode(folly::io::Cursor& cursor, T& value) const = 0;
};

// A serializer for any class that knows how to read and write itself using a
// Thrift protocol.
template <typename Tag, typename Reader, typename Writer>
class ProtocolSerializer : public TagSerializer<Tag> {
  using Base = TagSerializer<Tag>;
  using T = type::native_type<Tag>;

 public:
  using Base::encode;
  void encode(
      const T& value, folly::io::QueueAppender&& appender) const override {
    Writer writer;
    writer.setOutput(std::move(appender));
    op::encode<Tag>(writer, value);
  }

  using Base::decode;
  void decode(folly::io::Cursor& cursor, T& value) const override {
    Reader reader;
    reader.setInput(cursor);
    op::decode<Tag>(reader, value);
    cursor = reader.getCursor();
  }
};

} // namespace apache::thrift::op
