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

#include <thrift/lib/thrift/detail/protocol.h>

#include <folly/Hash.h>
#include <folly/Indestructible.h>
#include <folly/io/IOBuf.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/ThriftTypeInfo.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_detail_types.h>

namespace apache::thrift::protocol::detail {

template <class Base>
const char* ObjectWrapper<Base>::__fbthrift_thrift_uri() {
  static const folly::Indestructible<std::string> ret = uri<Base>();
  return ret->c_str();
}

template <class Base>
const char* ValueWrapper<Base>::__fbthrift_thrift_uri() {
  static const folly::Indestructible<std::string> ret = uri<Base>();
  return ret->c_str();
}

std::optional<std::size_t> hash_value(const Value& s) {
  auto value_type = s.getType();
  switch (value_type) {
    case Value::Type::boolValue:
      return folly::hash::hash_combine(value_type, s.as_bool());
    case Value::Type::byteValue:
      return folly::hash::hash_combine(value_type, s.as_byte());
    case Value::Type::i16Value:
      return folly::hash::hash_combine(value_type, s.as_i16());
    case Value::Type::i32Value:
      return folly::hash::hash_combine(value_type, s.as_i32());
    case Value::Type::i64Value:
      return folly::hash::hash_combine(value_type, s.as_i64());
    case Value::Type::floatValue:
      return folly::hash::hash_combine(value_type, s.as_float());
    case Value::Type::doubleValue:
      return folly::hash::hash_combine(value_type, s.as_double());
    case Value::Type::stringValue:
      return folly::hash::hash_combine(value_type, s.as_string());
    case Value::Type::binaryValue:
      return folly::hash::hash_combine(
          value_type, folly::IOBufHash()(s.as_binary()));
    case Value::Type::listValue:
      [[fallthrough]];
    case Value::Type::setValue:
      [[fallthrough]];
    case Value::Type::mapValue:
      [[fallthrough]];
    case Value::Type::objectValue:
      [[fallthrough]];
    case Value::Type::__EMPTY__:
      return std::nullopt;
  }
}

template const char* ObjectWrapper<detail::Object>::__fbthrift_thrift_uri();
template const char* ValueWrapper<detail::Value>::__fbthrift_thrift_uri();

} // namespace apache::thrift::protocol::detail
