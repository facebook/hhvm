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

#include <utility>

#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/detail/overload.h>

namespace apache::thrift::compiler::detail {

template <typename... Visitors>
decltype(auto) visit_type(const t_type& ty, Visitors&&... visitors) {
  auto f = overload(std::forward<Visitors>(visitors)...);
  if (ty.is<t_typedef>()) {
    return std::invoke(f, dynamic_cast<const t_typedef&>(ty));
  }
  switch (ty.get_type_value()) {
    case t_type::type::t_void:
    case t_type::type::t_bool:
    case t_type::type::t_byte:
    case t_type::type::t_i16:
    case t_type::type::t_i32:
    case t_type::type::t_i64:
    case t_type::type::t_float:
    case t_type::type::t_double:
    case t_type::type::t_string:
    case t_type::type::t_binary:
      return std::invoke(f, dynamic_cast<const t_primitive_type&>(ty));
    case t_type::type::t_list:
      return std::invoke(f, dynamic_cast<const t_list&>(ty));
    case t_type::type::t_set:
      return std::invoke(f, dynamic_cast<const t_set&>(ty));
    case t_type::type::t_map:
      return std::invoke(f, dynamic_cast<const t_map&>(ty));
    case t_type::type::t_enum:
      return std::invoke(f, dynamic_cast<const t_enum&>(ty));
    case t_type::type::t_structured: {
      if (const auto* s = ty.try_as<t_struct>()) {
        return std::invoke(f, static_cast<const t_struct&>(*s));
      } else if (const auto* u = ty.try_as<t_union>()) {
        return std::invoke(f, static_cast<const t_union&>(*u));
      } else if (const auto* ex = ty.try_as<t_exception>()) {
        return std::invoke(f, static_cast<const t_exception&>(*ex));
      }
      throw std::logic_error("Missing visitor specialization for t_structured");
    }
    case t_type::type::t_service:
      return std::invoke(f, dynamic_cast<const t_service&>(ty));
  }
  abort();
}

} // namespace apache::thrift::compiler::detail
