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

#include <type_traits>
#include <utility>

#include <folly/FBString.h>
#include <folly/Traits.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/lib/cpp2/visitation/for_each.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache {
namespace thrift {
namespace compiler {

// Assigns a t_const_value to a concrete Thrift type.
// Must include generated _visitation.h header for that type.

inline void hydrate_const(bool& out, const t_const_value& val) {
  out = val.get_bool();
}
template <typename T>
std::enable_if_t<std::is_integral_v<T>> hydrate_const(
    T& out, const t_const_value& val) {
  out = val.get_integer();
}
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>> hydrate_const(
    T& out, const t_const_value& val) {
  out = val.get_double();
}
inline void hydrate_const(std::string& out, const t_const_value& val) {
  out = val.get_string();
}
inline void hydrate_const(folly::fbstring& out, const t_const_value& val) {
  out = val.get_string();
}
template <typename T> // list
folly::void_t<decltype(std::declval<T>().emplace_back())> hydrate_const(
    T& out, const t_const_value& val) {
  for (auto* item : val.get_list()) {
    auto& entry = out.emplace_back();
    hydrate_const(entry, *item);
  }
}
template <typename T> // set
std::enable_if_t<std::is_same_v<typename T::key_type, typename T::value_type>>
hydrate_const(T& out, const t_const_value& val) {
  for (auto* item : val.get_list()) {
    typename T::key_type value;
    hydrate_const(value, *item);
    out.emplace(std::move(value));
  }
}
template <typename T> // map
folly::void_t<typename T::mapped_type> hydrate_const(
    T& out, const t_const_value& val) {
  for (const auto& pair : val.get_map()) {
    typename T::key_type key;
    hydrate_const(key, *pair.first);
    typename T::mapped_type value;
    hydrate_const(value, *pair.second);
    out.emplace(std::move(key), std::move(value));
  }
}
template <typename T>
std::enable_if_t<std::is_enum_v<T>> hydrate_const(
    T& out, const t_const_value& val) {
  out = static_cast<T>(val.get_integer());
}
template <typename T>
std::enable_if_t<is_thrift_class_v<T>> hydrate_const(
    T& out, const t_const_value& val) {
  assert(val.get_type() == t_const_value::t_const_value_type::CV_MAP);
  std::unordered_map<std::string, t_const_value*> map;
  for (const auto& pair : val.get_map()) {
    map[pair.first->get_string()] = pair.second;
  }

  for_each_field(out, [&](const metadata::ThriftField& meta, auto field_ref) {
    if (!map.count(*meta.name())) {
      return;
    }

    hydrate_const(field_ref.ensure(), *map.at(*meta.name()));
  });
}
template <typename T>
folly::void_t<decltype(std::declval<T>().toThrift())> hydrate_const(
    T& out, const t_const_value& val) {
  hydrate_const(out.toThrift(), val);
}

// Assigns a t_const_value to a Value.
// Currently only uses bool/i64/double/string/list/map.
// TODO: allow increasing type fidelity.
inline protocol::Value const_to_value(const t_const_value& val) {
  protocol::Value ret;
  switch (val.get_type()) {
    case t_const_value::CV_BOOL:
      ret.emplace_bool();
      ret.as_bool() = val.get_bool();
      break;
    case t_const_value::CV_INTEGER:
      ret.emplace_i64();
      ret.as_i64() = val.get_integer();
      break;
    case t_const_value::CV_DOUBLE:
      ret.emplace_double();
      ret.as_double() = val.get_double();
      break;
    case t_const_value::CV_STRING:
      ret.emplace_string();
      ret.as_string() = val.get_string();
      break;
    case t_const_value::CV_MAP:
      ret.emplace_map();
      for (const auto& map_elem : val.get_map()) {
        ret.as_map().emplace(
            const_to_value(*map_elem.first), const_to_value(*map_elem.second));
      }
      break;
    case t_const_value::CV_LIST:
      ret.emplace_list();
      for (const auto& list_elem : val.get_list()) {
        ret.as_list().push_back(const_to_value(*list_elem));
      }
      break;
  }
  return ret;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
