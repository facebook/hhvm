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

} // namespace apache::thrift::protocol
