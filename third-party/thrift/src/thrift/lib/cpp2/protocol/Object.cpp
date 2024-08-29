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

#include <thrift/lib/cpp2/protocol/Object.h>

#include <thrift/lib/cpp/util/EnumUtils.h>

namespace apache::thrift::protocol {

bool isIntrinsicDefault(const Value& value) {
  switch (value.getType()) {
    case Value::Type::boolValue:
      return value.as_bool() == false;
    case Value::Type::byteValue:
      return value.as_byte() == 0;
    case Value::Type::i16Value:
      return value.as_i16() == 0;
    case Value::Type::i32Value:
      return value.as_i32() == 0;
    case Value::Type::i64Value:
      return value.as_i64() == 0;
    case Value::Type::floatValue:
      return value.as_float() == 0;
    case Value::Type::doubleValue:
      return value.as_double() == 0;
    case Value::Type::stringValue:
      return value.as_string().empty();
    case Value::Type::binaryValue:
      return value.as_binary().empty();
    case Value::Type::listValue:
      return value.as_list().empty();
    case Value::Type::setValue:
      return value.as_set().empty();
    case Value::Type::mapValue:
      return value.as_map().empty();
    case Value::Type::objectValue:
      return std::all_of(
          value.as_object().begin(), value.as_object().end(), [](auto& kv) {
            return isIntrinsicDefault(kv.second);
          });
    case Value::Type::__EMPTY__:
      return true;
    default:
      folly::throw_exception<std::runtime_error>("Not Implemented.");
  }
}

bool isIntrinsicDefault(const Object& obj) {
  Value val;
  val.objectValue_ref() = obj;
  return isIntrinsicDefault(val);
}

static std::string str(const Value& value) {
  std::string ret;
  switch (value.getType()) {
    case Value::Type::boolValue:
      return std::to_string(value.as_bool());
    case Value::Type::byteValue:
      return std::to_string(value.as_byte());
    case Value::Type::i16Value:
      return std::to_string(value.as_i16());
    case Value::Type::i32Value:
      return std::to_string(value.as_i32());
    case Value::Type::i64Value:
      return std::to_string(value.as_i64());
    case Value::Type::floatValue:
      return std::to_string(value.as_float());
    case Value::Type::doubleValue:
      return std::to_string(value.as_double());
    case Value::Type::stringValue:
      return value.as_string();
    case Value::Type::binaryValue:
      return value.as_binary().to<std::string>();
    default:
      folly::throw_exception<std::runtime_error>(
          "str(" + apache::thrift::util::enumNameSafe(value.getType()) +
          ") is not implemented.");
  }
}

folly::dynamic toDynamic(const Value& value) {
  folly::dynamic ret;
  switch (value.getType()) {
    case Value::Type::boolValue:
      return value.as_bool();
    case Value::Type::byteValue:
      return value.as_byte();
    case Value::Type::i16Value:
      return value.as_i16();
    case Value::Type::i32Value:
      return value.as_i32();
    case Value::Type::i64Value:
      return value.as_i64();
    case Value::Type::floatValue:
      return value.as_float();
    case Value::Type::doubleValue:
      return value.as_double();
    case Value::Type::stringValue:
      return value.as_string();
    case Value::Type::binaryValue:
      return value.as_binary().to<std::string>();
    case Value::Type::listValue:
      ret = folly::dynamic::array();
      ret.reserve(value.as_list().size());
      for (auto&& v : value.as_list()) {
        ret.push_back(toDynamic(v));
      }
      return ret;
    case Value::Type::setValue:
      ret = folly::dynamic::array();
      ret.reserve(value.as_set().size());
      for (auto&& v : value.as_set()) {
        ret.push_back(toDynamic(v));
      }
      return ret;
    case Value::Type::mapValue:
      ret = folly::dynamic::object();
      for (auto&& [k, v] : value.as_map()) {
        ret[str(k)] = toDynamic(v);
      }
      return ret;
    case Value::Type::objectValue:
      ret = folly::dynamic::object();
      for (auto&& [k, v] : value.as_object()) {
        ret[std::to_string(k)] = toDynamic(v);
      }
      return ret;
    case Value::Type::__EMPTY__:
      return nullptr;
    default:
      folly::throw_exception<std::runtime_error>("Not Implemented.");
  }
}

folly::dynamic toDynamic(const Object& obj) {
  Value v;
  v.objectValue_ref() = obj;
  return toDynamic(v);
}

} // namespace apache::thrift::protocol
