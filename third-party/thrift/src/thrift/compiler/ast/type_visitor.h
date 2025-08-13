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
  } else if (ty.is<t_primitive_type>()) {
    return std::invoke(f, dynamic_cast<const t_primitive_type&>(ty));
  } else if (ty.is<t_list>()) {
    return std::invoke(f, dynamic_cast<const t_list&>(ty));
  } else if (ty.is<t_set>()) {
    return std::invoke(f, dynamic_cast<const t_set&>(ty));
  } else if (ty.is<t_map>()) {
    return std::invoke(f, dynamic_cast<const t_map&>(ty));
  } else if (ty.is<t_enum>()) {
    return std::invoke(f, dynamic_cast<const t_enum&>(ty));
  } else if (ty.is<t_structured>()) {
    if (const auto* s = ty.try_as<t_struct>()) {
      return std::invoke(f, static_cast<const t_struct&>(*s));
    } else if (const auto* u = ty.try_as<t_union>()) {
      return std::invoke(f, static_cast<const t_union&>(*u));
    } else if (const auto* ex = ty.try_as<t_exception>()) {
      return std::invoke(f, static_cast<const t_exception&>(*ex));
    }
    throw std::logic_error("Missing visitor specialization for t_structured");
  } else if (ty.is<t_service>()) {
    return std::invoke(f, dynamic_cast<const t_service&>(ty));
  }
  abort();
}

} // namespace apache::thrift::compiler::detail
