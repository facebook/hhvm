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

#include <folly/Traits.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/lib/cpp2/visitation/for_each.h>

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
    hydrate_const(key, *pair.second);
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
} // namespace compiler
} // namespace thrift
} // namespace apache
