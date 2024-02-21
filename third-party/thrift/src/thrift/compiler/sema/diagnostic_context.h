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

#include <map>
#include <type_traits>
#include <typeindex>
#include <utility>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/diagnostic.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_node;

// A cache for metadata associated with an AST node.
//
// Useful for avoiding re-computation during a traversal of an AST.
class node_metadata_cache {
  template <typename F>
  using element_type = typename decltype(std::declval<F>()())::element_type;
  template <typename T, typename... Args>
  using if_is_constructible =
      std::enable_if_t<std::is_constructible<T, Args...>::value, T&>;
  using key_type = std::pair<const t_node*, std::type_index>;

 public:
  // Gets or creates a cache entry of the given type T for the specified node.
  //
  // If the entry is not already present in the cache, a new entry is created
  // using the provided loader, which must return a std::unique_ptr<T>.
  template <typename..., typename N, typename F>
  element_type<F>& get(N& node, const F& loader) { // `node` must be an lvalue.
    using T = element_type<F>;
    key_type key(&node, typeid(T));
    auto itr = data_.find(key);
    if (itr == data_.end()) {
      itr = data_.emplace(std::move(key), to_any_data(loader())).first;
    }
    return *static_cast<T*>(itr->second.get());
  }

  // Gets or creates a cache entry of the given type T for the specified node.
  //
  // T must be constructible in exactly one of the following ways:
  // - T()
  // - T(const N&)
  // - T(node_metadata_cache&, const N&)
  template <typename T, typename N>
  if_is_constructible<T> get(N& node) { // `node` must be an lvalue.
    return get(node, [] { return std::make_unique<T>(); });
  }
  template <typename T, typename N>
  if_is_constructible<T, N&> get(N& node) { // `node` must be an lvalue.
    return get(node, [&node] { return std::make_unique<T>(node); });
  }
  template <typename T, typename N>
  if_is_constructible<T, node_metadata_cache&, const N&> get(
      N& node) { // `node` must be an lvalue.
    return get(node, [this, &node] {
      return std::make_unique<T>(*this, static_cast<const N&>(node));
    });
  }

 private:
  // TODO(afuller): Use std::any when c++17 can be used.
  using any_data = std::unique_ptr<void, void (*const)(void*)>;
  std::map<key_type, any_data> data_;

  template <typename T>
  any_data to_any_data(std::unique_ptr<T> value) {
    return {value.release(), [](void* ptr) { delete static_cast<T*>(ptr); }};
  }
};

// A context aware reporter for diagnostic results.
class diagnostic_context : public diagnostics_engine,
                           public const_visitor_context {
 public:
  using diagnostics_engine::diagnostics_engine;

  explicit diagnostic_context(diagnostics_engine& diags)
      : diagnostics_engine(
            diags.source_mgr(),
            [&diags](diagnostic diag) { diags.report(std::move(diag)); },
            diags.params()) {}

  // A cache for traversal-specific metadata.
  node_metadata_cache& cache() { return cache_; }

  using diagnostics_engine::warning;
  template <typename... T>
  void warning(fmt::format_string<T...> msg, T&&... args) {
    warning(*current(), msg, std::forward<T>(args)...);
  }

  using diagnostics_engine::error;
  template <typename... T>
  void error(fmt::format_string<T...> msg, T&&... args) {
    error(*current(), msg, std::forward<T>(args)...);
  }

  using diagnostics_engine::check;
  template <typename... T>
  bool check(bool condition, fmt::format_string<T...> msg, T&&... args) {
    return check(condition, *current(), msg, std::forward<T>(args)...);
  }

  using diagnostics_engine::report;

 private:
  node_metadata_cache cache_;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
