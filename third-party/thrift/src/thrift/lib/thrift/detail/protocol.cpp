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

#include <folly/Indestructible.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_detail_types.h>

#include <xxhash.h>

namespace {
std::string str(const apache::thrift::protocol::detail::Value& value) {
  using apache::thrift::protocol::detail::Value;
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
} // namespace

namespace apache::thrift::protocol::detail {

template <class Base>
const char* ObjectWrapper<Base>::__fbthrift_thrift_uri() {
  static const folly::Indestructible<std::string> ret = uri<Base>();
  return ret->c_str();
}

template <class Base>
bool ObjectWrapper<Base>::__fbthrift_is_empty() const {
  return empty();
}

template <class Base>
const char* ValueWrapper<Base>::__fbthrift_thrift_uri() {
  static const folly::Indestructible<std::string> ret = uri<Base>();
  return ret->c_str();
}

template <class Base>
bool ValueWrapper<Base>::__fbthrift_is_empty() const {
  return ::apache::thrift::empty(toThrift());
}

using ValueList = Value::ValueList;
using BoxedValueList = Value::BoxedValueList;
using ValueSet = Value::ValueSet;
using BoxedValueSet = Value::BoxedValueSet;
using ValueMap = Value::ValueMap;
using BoxedValueMap = Value::BoxedValueMap;

template <>
folly::dynamic Value::toDynamicImpl() const {
  folly::dynamic ret;
  switch (getType()) {
    case Value::Type::boolValue:
      return as_bool();
    case Value::Type::byteValue:
      return as_byte();
    case Value::Type::i16Value:
      return as_i16();
    case Value::Type::i32Value:
      return as_i32();
    case Value::Type::i64Value:
      return as_i64();
    case Value::Type::floatValue:
      return as_float();
    case Value::Type::doubleValue:
      return as_double();
    case Value::Type::stringValue:
      return as_string();
    case Value::Type::binaryValue:
      return as_binary().to<std::string>();
    case Value::Type::listValue:
      ret = folly::dynamic::array();
      ret.reserve(as_list().size());
      for (auto&& v : as_list()) {
        ret.push_back(v.toDynamicImpl());
      }
      return ret;
    case Value::Type::setValue:
      ret = folly::dynamic::array();
      ret.reserve(as_set().size());
      for (auto&& v : as_set()) {
        ret.push_back(v.toDynamicImpl());
      }
      return ret;
    case Value::Type::mapValue:
      ret = folly::dynamic::object();
      for (auto&& [k, v] : as_map()) {
        ret[str(k)] = v.toDynamicImpl();
      }
      return ret;
    case Value::Type::objectValue: {
      ret = folly::dynamic::object();
      for (auto&& [k, v] : as_object()) {
        ret[std::to_string(k)] = v.toDynamicImpl();
      }
      return ret;
    }
    case Value::Type::__EMPTY__:
      return nullptr;
    default:
      folly::throw_exception<std::runtime_error>("Not Implemented.");
  }
}

template <>
folly::dynamic Object::toDynamicImpl() const {
  folly::dynamic ret = folly::dynamic::object();
  for (auto&& [k, v] : *this) {
    ret[std::to_string(k)] = v.toDynamicImpl();
  }
  return ret;
}

template <>
ValueList& Value::emplace_list(BoxedValueList t) {
  return *toThrift().set_listValue(std::move(t));
}

template <>
ValueList& Value::emplace_list(const ValueList& t) {
  return *toThrift().set_listValue(t);
}

template <>
ValueList& Value::emplace_list(ValueList&& t) {
  return *toThrift().set_listValue(std::move(t));
}

template <>
BoxedValueSet& Value::set_setValue(BoxedValueSet t) {
  return toThrift().set_setValue(std::move(t));
}
template <>
BoxedValueSet& Value::set_setValue(const ValueSet& t) {
  return toThrift().set_setValue(t);
}
template <>
BoxedValueSet& Value::set_setValue(ValueSet&& t) {
  return toThrift().set_setValue(std::move(t));
}

template <>
BoxedValueMap& Value::set_mapValue(BoxedValueMap t) {
  return toThrift().set_mapValue(std::move(t));
}
template <>
BoxedValueMap& Value::set_mapValue(const ValueMap& t) {
  return toThrift().set_mapValue(t);
}
template <>
BoxedValueMap& Value::set_mapValue(ValueMap&& t) {
  return toThrift().set_mapValue(std::move(t));
}

static std::size_t hashVector(const std::vector<std::size_t>& a) {
  return XXH3_64bits(a.data(), a.size() * sizeof(a[0]));
}

std::size_t hash_value(const Value& s) {
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
    case Value::Type::listValue: {
      const auto& list = s.as_list();
      std::vector<std::size_t> hashes;
      hashes.reserve(list.size() + 1);
      hashes.push_back(static_cast<std::size_t>(value_type));
      for (auto&& v : list) {
        hashes.push_back(hash_value(v));
      }
      return hashVector(hashes);
    }
    case Value::Type::setValue: {
      const auto& set = s.as_set();
      std::vector<std::size_t> hashes(set.size() + 1);
      hashes[0] = static_cast<std::size_t>(value_type);
      std::size_t* valueHashes = hashes.data() + 1;
      std::size_t i = 0;
      for (auto&& v : set) {
        valueHashes[i++] = hash_value(v);
      }
      std::sort(valueHashes, hashes.data() + hashes.size());
      return hashVector(hashes);
    }
    case Value::Type::mapValue: {
      // Keys and values are sorted independently, so this produces collisions
      // between maps where the same values are mapped to the same keys
      // differently.
      const auto& map = s.as_map();
      std::size_t size = map.size();
      std::vector<std::size_t> hashes(size * 2 + 1);
      hashes[0] = static_cast<std::size_t>(value_type);
      std::size_t* keyHashes = hashes.data() + 1;
      std::size_t* valueHashes = keyHashes + size;
      std::size_t i = 0;
      for (auto&& [k, v] : map) {
        hashes[i] = hash_value(k);
        hashes[size + i] = hash_value(v);
        ++i;
      }
      std::sort(keyHashes, valueHashes);
      std::sort(valueHashes, hashes.data() + hashes.size());
      return hashVector(hashes);
    }
    case Value::Type::objectValue: {
      const Object& object = s.as_object();
      size_t size = object.size();
      std::vector<std::size_t> hashes(size * 2 + 2);
      hashes[0] = static_cast<std::size_t>(value_type);
      hashes[1] = std::hash<std::string>{}(*object.type());
      std::size_t* keyHashes = hashes.data() + 2;
      std::size_t* valueHashes = keyHashes + size;
      std::size_t i = 0;
      for (auto&& [k, v] : object) {
        keyHashes[i] = k; // hashing integer key is the identity
        valueHashes[i] = hash_value(v);
        ++i;
      }
      std::sort(keyHashes, valueHashes);
      std::sort(valueHashes, hashes.data() + hashes.size());
      return hashVector(hashes);
    }
    case Value::Type::__EMPTY__: {
      return 0;
    }
  }
  return 0;
}

const detail::Value* into_inner_value(const Value* v) {
  return v ? &v->toThrift() : nullptr;
}

const detail::Object* into_inner_object(const Object* o) {
  return o ? &o->toThrift() : nullptr;
}

template const char* ObjectWrapper<detail::Object>::__fbthrift_thrift_uri();
template bool ObjectWrapper<detail::Object>::__fbthrift_is_empty() const;
template const char* ValueWrapper<detail::Value>::__fbthrift_thrift_uri();
template bool ValueWrapper<detail::Value>::__fbthrift_is_empty() const;

} // namespace apache::thrift::protocol::detail
