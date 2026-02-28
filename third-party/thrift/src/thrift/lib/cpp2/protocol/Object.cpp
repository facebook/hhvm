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
  val.emplace_object(obj);
  return isIntrinsicDefault(val);
}

namespace {
bool maybeTypeStruct(const protocol::Value& value) {
  // Add static_assert to make sure this function is updated when we changed
  // Thrift.Any's schema in the future.
  static_assert(op::num_fields<type::TypeStruct> == 2);

  // Make sure field id matches
  static_assert(
      op::get_field_id_v<type::TypeStruct, ident::name> == FieldId{1});
  static_assert(
      op::get_field_id_v<type::TypeStruct, ident::params> == FieldId{2});
  static_assert(
      op::get_ordinal_v<type::TypeStruct, field_id<3>> == FieldOrdinal{0});

  // Make sure fields are not optional, not terse write.
  static_assert(std::is_same_v<
                op::get_field_ref<type::TypeStruct, ident::name>,
                field_ref<type::TypeName&>>);
  static_assert(std::is_same_v<
                op::get_field_ref<type::TypeStruct, ident::params>,
                field_ref<std::vector<type::TypeStruct>&>>);

  if (!value.is_object()) {
    return false;
  }

  const auto& obj = value.as_object();
  auto name = folly::get_ptr(*obj.members(), 1);
  auto params = folly::get_ptr(*obj.members(), 2);

  return name && name->is_object() && params && params->is_list();
}
} // namespace

bool maybeAny(const protocol::Object& obj) {
  // Add static_assert to make sure this function is updated when we changed
  // Thrift.Any's schema in the future.
  static_assert(op::num_fields<type::AnyStruct> == 3);

  // Make sure field id matches
  static_assert(op::get_field_id_v<type::AnyStruct, ident::type> == FieldId{1});
  static_assert(
      op::get_field_id_v<type::AnyStruct, ident::protocol> == FieldId{2});
  static_assert(op::get_field_id_v<type::AnyStruct, ident::data> == FieldId{3});
  static_assert(
      op::get_ordinal_v<type::AnyStruct, field_id<4>> == FieldOrdinal{0});

  // Make sure fields are not optional, not terse write.
  static_assert(std::is_same_v<
                op::get_field_ref<type::AnyStruct, ident::type>,
                field_ref<type::Type&>>);
  static_assert(std::is_same_v<
                op::get_field_ref<type::AnyStruct, ident::protocol>,
                field_ref<type::Protocol&>>);
  static_assert(std::is_same_v<
                op::get_field_ref<type::AnyStruct, ident::data>,
                field_ref<folly::IOBuf&>>);

  auto type = folly::get_ptr(*obj.members(), 1);
  auto protocol = folly::get_ptr(*obj.members(), 2);
  auto data = folly::get_ptr(*obj.members(), 3);

  return type && maybeTypeStruct(*type) && protocol && protocol->is_object() &&
      data && data->is_binary() && !obj.contains(FieldId{4});
}
} // namespace apache::thrift::protocol
