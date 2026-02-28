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
  if (const t_typedef* typedef_ = ty.try_as<t_typedef>()) {
    return std::invoke(f, *typedef_);
  } else if (
      const t_primitive_type* primitive_type = ty.try_as<t_primitive_type>()) {
    return std::invoke(f, *primitive_type);
  } else if (const t_list* list = ty.try_as<t_list>()) {
    return std::invoke(f, *list);
  } else if (const t_set* set = ty.try_as<t_set>()) {
    return std::invoke(f, *set);
  } else if (const t_map* map = ty.try_as<t_map>()) {
    return std::invoke(f, *map);
  } else if (const t_enum* enum_ = ty.try_as<t_enum>()) {
    return std::invoke(f, *enum_);
  } else if (const t_struct* struct_ = ty.try_as<t_struct>()) {
    return std::invoke(f, *struct_);
  } else if (const t_union* union_ = ty.try_as<t_union>()) {
    return std::invoke(f, *union_);
  } else if (const t_exception* exception = ty.try_as<t_exception>()) {
    return std::invoke(f, *exception);
  } else if (ty.is<t_structured>()) {
    throw std::logic_error("Missing visitor specialization for t_structured");
  } else if (const t_service* service = ty.try_as<t_service>()) {
    return std::invoke(f, *service);
  }
  abort();
}

} // namespace apache::thrift::compiler::detail
