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

#include <functional>
#include <type_traits>
#include <utility>

#include <thrift/compiler/ast/t_node.h>

// Visitation and registration functions for concrete AST nodes.
#define FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(name)       \
 private:                                                   \
  using name##_type = node_type<t_##name>;                  \
  visitor_list<Args..., name##_type&> name##_visitors_;     \
                                                            \
 public:                                                    \
  void add_##name##_visitor(                                \
      std::function<void(Args..., name##_type&)> visitor) { \
    name##_visitors_.emplace_back(std::move(visitor));      \
  }                                                         \
  void operator()(Args... args, name##_type& node) const

namespace apache {
namespace thrift {
namespace compiler {
namespace ast_detail {

// The type to use when traversing the given node type N.
template <bool is_const, typename N>
using node_type = std::conditional_t<is_const, const N, N>;

// Helper that to propagate constness through a dynamic_cast.
template <typename N>
N* as(t_node* node) {
  return dynamic_cast<N*>(node);
}
template <typename N>
const N* as(const t_node* node) {
  return dynamic_cast<const N*>(node);
}

template <typename... Args>
using void_t = void;

// TODO(afuller): Use a c++ 'concept' when available or switch
// to a stricter form of checking if an argument is an 'observer'
template <typename O, typename = void>
struct is_observer : std::false_type {};
template <typename O>
struct is_observer<
    O,
    void_t<
        decltype(std::declval<O>().begin_visit(std::declval<t_node&>())),
        decltype(std::declval<O>().end_visit(std::declval<t_node&>()))>>
    : std::true_type {};

template <typename O>
using if_observer = std::enable_if_t<is_observer<O>::value>;
template <typename O>
using if_not_observer = std::enable_if_t<!is_observer<O>::value>;

// Helper to call begin/end_visit if supported on the given argument.
template <typename T, typename N = const t_node>
if_observer<T> begin_visit(N& node, T& observer) {
  observer.begin_visit(node);
}
template <typename T, typename N = const t_node>
if_observer<T> end_visit(N& node, T& observer) {
  observer.end_visit(node);
}
template <typename T>
if_not_observer<T> begin_visit(const t_node&, T&) {}
template <typename T>
if_not_observer<T> end_visit(const t_node&, T&) {}

} // namespace ast_detail
} // namespace compiler
} // namespace thrift
} // namespace apache
