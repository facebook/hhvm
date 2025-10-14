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

#include <concepts>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <folly/FBString.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::compiler {

// Assigns a t_const_value to a concrete Thrift type.

inline void hydrate_const(bool& out, const t_const_value& val) {
  out =
      val.kind() == t_const_value::CV_BOOL ? val.get_bool() : val.get_integer();
}
template <std::integral T>
void hydrate_const(T& out, const t_const_value& val) {
  out = val.get_integer();
}
template <std::floating_point T>
void hydrate_const(T& out, const t_const_value& val) {
  out = val.kind() == t_const_value::t_const_value_kind::CV_DOUBLE
      ? val.get_double()
      : val.get_integer();
}
inline void hydrate_const(std::string& out, const t_const_value& val) {
  out = val.get_string();
}
inline void hydrate_const(folly::fbstring& out, const t_const_value& val) {
  out = val.get_string();
}
inline void hydrate_const(folly::IOBuf& out, const t_const_value& val) {
  out = folly::IOBuf(folly::IOBuf::CopyBufferOp{}, val.get_string());
}
template <typename T>
  requires requires(T t) { t.emplace_back(); } // list
void hydrate_const(T& out, const t_const_value& val) {
  for (auto* item : val.get_list()) {
    auto& entry = out.emplace_back();
    hydrate_const(entry, *item);
  }
}
template <typename T>
  requires requires {
    typename T::key_type;
    typename T::value_type;
  } && std::same_as<typename T::key_type, typename T::value_type> // set
void hydrate_const(T& out, const t_const_value& val) {
  for (auto* item : val.get_list_or_empty_map()) {
    typename T::key_type value;
    hydrate_const(value, *item);
    out.emplace(std::move(value));
  }
}
template <typename T>
  requires requires { typename T::mapped_type; } // map
void hydrate_const(T& out, const t_const_value& val) {
  for (const auto& pair : val.get_map()) {
    typename T::key_type key;
    hydrate_const(key, *pair.first);
    typename T::mapped_type value;
    hydrate_const(value, *pair.second);
    out.emplace(std::move(key), std::move(value));
  }
}
template <typename T>
  requires std::is_enum_v<T>
void hydrate_const(T& out, const t_const_value& val) {
  out = static_cast<T>(val.get_integer());
}
template <typename T>
decltype(auto) ensure(std::unique_ptr<T>& t) {
  return t ? *t : *(t = std::make_unique<T>()); // cpp.ref
}
template <typename T>
decltype(auto) ensure(terse_field_ref<T> t) {
  return *t;
}
template <typename T>
decltype(auto) ensure(T t) {
  return t.ensure(); // *field_ref
}
template <typename T>
  requires is_thrift_class_v<T>
void hydrate_const(T& out, const t_const_value& val) {
  assert(val.kind() == t_const_value::t_const_value_kind::CV_MAP);
  std::unordered_map<std::string_view, t_const_value*> map;
  for (const auto& pair : val.get_map()) {
    map[pair.first->get_string()] = pair.second;
  }

  op::for_each_ordinal<T>([&](auto id) {
    using Id = decltype(id);
    auto name = op::get_name_v<T, Id>;
    if (!map.count(name)) {
      return;
    }
    hydrate_const(ensure(op::get<Id>(out)), *map.at(name));
  });
}

template <typename T>
  requires requires(T t) { t.toThrift(); }
void hydrate_const(T& out, const t_const_value& val) {
  hydrate_const(out.toThrift(), val);
}
inline void hydrate_const(protocol::Value& out, const t_const_value& val) {
  protocol::detail::detail::Value inner;
  hydrate_const(inner, val);
  out = protocol::Value::fromThrift(std::move(inner));
}
inline void hydrate_const(protocol::Object& out, const t_const_value& val) {
  protocol::detail::detail::Object inner;
  hydrate_const(inner, val);
  out = protocol::Object(std::move(inner));
}

// Assigns a t_const_value to a Value.
inline protocol::Value const_to_value(const t_const_value& val) {
  protocol::Value ret;
  auto type = val.ttype() ? val.ttype()->get_type_value() : [&] {
    switch (val.kind()) {
      case t_const_value::CV_BOOL:
        return t_type::type::t_bool;
      case t_const_value::CV_INTEGER:
        return t_type::type::t_i64;
      case t_const_value::CV_DOUBLE:
        return t_type::type::t_double;
      case t_const_value::CV_STRING:
        return t_type::type::t_string;
      case t_const_value::CV_LIST:
        return t_type::type::t_list;
      case t_const_value::CV_MAP:
        return t_type::type::t_map;
      case t_const_value::CV_IDENTIFIER:
        assert(false);
        return t_type::type::t_void;
    }
  }();
  switch (type) {
    case t_type::type::t_bool:
      ret.emplace_bool();
      if (val.kind() == t_const_value::CV_BOOL) {
        ret.as_bool() = val.get_bool();
      } else if (val.kind() == t_const_value::CV_INTEGER) {
        auto value = val.get_integer();
        assert(value == 0 || value == 1);
        ret.as_bool() = value;
      }
      break;
    case t_type::type::t_byte:
      ret.emplace_byte(val.get_integer());
      break;
    case t_type::type::t_i16:
      ret.emplace_i16(val.get_integer());
      break;
    case t_type::type::t_i32:
      ret.emplace_i32(val.get_integer());
      break;
    case t_type::type::t_i64:
      ret.emplace_i64(val.get_integer());
      break;
    case t_type::type::t_float:
      ret.emplace_float(
          val.kind() == t_const_value::t_const_value_kind::CV_DOUBLE
              ? val.get_double()
              : val.get_integer());
      break;
    case t_type::type::t_double:
      ret.emplace_double(
          val.kind() == t_const_value::t_const_value_kind::CV_DOUBLE
              ? val.get_double()
              : val.get_integer());
      break;
    case t_type::type::t_string:
      ret.emplace_string(val.get_string());
      break;
    case t_type::type::t_binary:
      ret.emplace_binary(
          folly::IOBuf(folly::IOBuf::CopyBufferOp{}, val.get_string()));
      break;
    case t_type::type::t_list: {
      auto valList = val.get_list_or_empty_map();
      auto& list = ret.emplace_list();
      list.reserve(valList.size());
      for (const auto& list_elem : valList) {
        list.push_back(const_to_value(*list_elem));
      }
      break;
    }
    case t_type::type::t_set: {
      const auto& valList = val.get_list_or_empty_map();
      auto& set = ret.emplace_set();
      set.reserve(valList.size());
      for (const auto& list_elem : val.get_list_or_empty_map()) {
        set.insert(const_to_value(*list_elem));
      }
      break;
    }
    case t_type::type::t_map: {
      auto& map = ret.emplace_map();
      if (val.kind() == t_const_value::CV_MAP) {
        map.reserve(val.get_map().size());
        for (const auto& map_elem : val.get_map()) {
          map.emplace(
              const_to_value(*map_elem.first),
              const_to_value(*map_elem.second));
        }
      }
      break;
    }
    case t_type::type::t_enum:
      ret.emplace_i32(val.get_integer());
      break;
    case t_type::type::t_structured:
      if (val.ttype()) {
        auto& obj = ret.emplace_object();
        const auto& obj_type = *val.ttype()->get_true_type();
        obj.type() = !obj_type.uri().empty() ? obj_type.uri() : obj_type.name();
        auto& strct = static_cast<const t_structured&>(obj_type);
        for (const auto& map_elem : val.get_map()) {
          auto field = strct.get_field_by_name(map_elem.first->get_string());
          if (!field) {
            throw std::out_of_range(fmt::format(
                "invalid field name: {}", map_elem.first->get_string()));
          }
          obj[FieldId{field->id()}] = const_to_value(*map_elem.second);
        }
      } else {
        auto& map = ret.emplace_map();
        map.reserve(val.get_map().size());
        for (const auto& map_elem : val.get_map()) {
          map.emplace(
              const_to_value(*map_elem.first),
              const_to_value(*map_elem.second));
        }
      }
      break;
    case t_type::type::t_void:
    case t_type::type::t_service:
      throw std::runtime_error("Unexpected type");
  }
  return ret;
}

} // namespace apache::thrift::compiler
