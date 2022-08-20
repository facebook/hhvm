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

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/detail/view.h>

namespace apache {
namespace thrift {
namespace compiler {

// An ordered list of the aliases for an annotation.
//
// If two aliases are set on a node, the value for the first alias in the span
// will be used.
//
// Like std::span and std::string_view, this class provides access
// to memory it does not own and must not be accessed after the associated
// data is destroyed.
class alias_span : public ast_detail::base_span<alias_span, const std::string> {
  using base = ast_detail::base_span<alias_span, const std::string>;

 public:
  using base::base;
  /* implicit */ constexpr alias_span(
      std::initializer_list<std::string> name) noexcept
      : alias_span(name.begin(), name.size()) {}
  /* implicit */ constexpr alias_span(const std::string& name) noexcept
      : alias_span(&name, 1) {}
  template <
      typename C = std::vector<std::string>,
      typename =
          decltype(std::declval<const C&>().data() + std::declval<const C&>().size())>
  /* implicit */ alias_span(const C& list)
      : alias_span(list.data(), list.size()) {}
  constexpr alias_span(const alias_span&) noexcept = default;
  constexpr alias_span& operator=(const alias_span&) noexcept = default;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
