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
#include <thrift/conformance/cpp2/AnyRef.h>
#include <thrift/conformance/cpp2/Protocol.h>

namespace apache::thrift::conformance {

// A serializer interface for any type.
//
// For example:
//    Serializer<>* serializer = ...
//    std::string data = serializer->encode(myValue);
//    auto value = serializer->decode<MyType>(data);
class AnySerializer {
 public:
  virtual ~AnySerializer() = default;

  // The protocol used by this serializer.
  virtual const Protocol& getProtocol() const = 0;

  // Enocde a value.
  //
  // Throws std::bad_any_cast if the type is not supported.
  virtual void encode(
      any_ref value, folly::io::QueueAppender&& appender) const = 0;

  // Decode a value.
  //
  // Throws std::bad_any_cast if the type is not supported.
  virtual void decode(
      const std::type_info& typeInfo,
      folly::io::Cursor& cursor,
      any_ref value) const = 0;

  // std::any overloads.
  std::any decode(
      const std::type_info& typeInfo, folly::io::Cursor& cursor) const;

  // Compile-time type overloads.
  template <typename T>
  T decode(folly::io::Cursor& cursor) const;

 protected:
  // Throws std::bad_any_cast if the types are not equal.
  static void checkType(
      const std::type_info& actual, const std::type_info& expected);
};

// A helper base class for a serializer of a single type.
//
// Type specific calls are forward to the following functions, required to
// be publicially accessible on Derived:
// - void encode(const T&, folly::io::QueueAppender&&) const
// - T decode(folly::io::Cursor& cursor) const
template <typename T, typename Derived>
class BaseTypedAnySerializer : public AnySerializer {
  using Base = AnySerializer;

 public:
  using Base::encode;
  void encode(any_ref value, folly::io::QueueAppender&& appender) const final {
    derived().encode(any_cast<const T&>(value), std::move(appender));
  }

  using Base::decode;
  void decode(
      const std::type_info& typeInfo,
      folly::io::Cursor& cursor,
      any_ref value) const final;

 private:
  const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

// Implementation.

template <typename T>
T AnySerializer::decode(folly::io::Cursor& cursor) const {
  T result;
  decode(typeid(T), cursor, result);
  return result;
}

template <typename T, typename Derived>
void BaseTypedAnySerializer<T, Derived>::decode(
    const std::type_info& typeInfo,
    folly::io::Cursor& cursor,
    any_ref value) const {
  checkType(typeInfo, typeid(T));
  if (auto* obj = any_cast_exact<T>(&value)) {
    derived().decode(cursor, *obj);
  } else {
    auto& any = any_cast_exact<std::any&>(value);
    derived().decode(cursor, any.emplace<T>());
  }
}

} // namespace apache::thrift::conformance
