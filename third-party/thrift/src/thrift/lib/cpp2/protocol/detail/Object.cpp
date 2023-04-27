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

#include <thrift/lib/cpp2/protocol/detail/Object.h>

namespace apache {
namespace thrift {
namespace protocol {
namespace detail {

type::Type toType(const protocol::Value& value) {
  switch (value.getType()) {
    case Value::Type::boolValue:
      return type::Type::create<type::bool_t>();
    case Value::Type::byteValue:
      return type::Type::create<type::byte_t>();
    case Value::Type::i16Value:
      return type::Type::create<type::i16_t>();
    case Value::Type::i32Value:
      return type::Type::create<type::i32_t>();
    case Value::Type::i64Value:
      return type::Type::create<type::i64_t>();
    case Value::Type::floatValue:
      return type::Type::create<type::float_t>();
    case Value::Type::doubleValue:
      return type::Type::create<type::double_t>();
    case Value::Type::stringValue:
      return type::Type::create<type::string_t>();
    case Value::Type::binaryValue:
      return type::Type::create<type::binary_t>();
    case Value::Type::listValue: {
      const auto& c = *value.listValue_ref();
      return type::Type::create<type::list_c>(
          !c.empty() ? toType(c[0]) : type::Type{});
    }
    case Value::Type::mapValue: {
      const auto& c = *value.mapValue_ref();
      type::Type keyType, valueType;
      if (!c.empty()) {
        keyType = toType(c.begin()->first);
        valueType = toType(c.begin()->second);
      }
      return type::Type::create<type::map_c>(keyType, valueType);
    }
    case Value::Type::setValue: {
      const auto& c = *value.setValue_ref();
      return type::Type::create<type::set_c>(
          !c.empty() ? toType(*c.begin()) : type::Type{});
    }
    case Value::Type::objectValue: {
      const std::string& uri = *value.objectValue_ref()->type();
      return uri.empty() ? type::Type{}
                         : type::Type::create<type::struct_c>(uri);
    }
    default: {
      TProtocolException::throwInvalidFieldData();
    }
  }
}

} // namespace detail
} // namespace protocol
} // namespace thrift
} // namespace apache
