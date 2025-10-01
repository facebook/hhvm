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

namespace apache::thrift::compiler {

class t_node;

// A cache for metadata associated with an AST node.
//
// Useful for avoiding re-computation during a traversal of an AST.
class node_metadata_cache {
  template <typename F>
  using element_type = typename decltype(std::declval<F>()())::element_type;
  template <typename T, typename... Args>
  using if_is_constructible =
      std::enable_if_t<std::is_constructible_v<T, Args...>, T&>;
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

struct sema_params {
  /**
   * Action to take on extra validation failure (see `sema_params`).
   */
  enum class validation_level {
    /** Do not take any action. */
    none,

    /** Issue a diagnostic warning. */
    warn,

    /** Issue a diagnostic error (i.e., will fail to compile). */
    error,
  };

  /**
   * Returns the validation level with the given human readable name, or
   * throws if none exists.
   *
   * See also: `validation_level_to_string(...)`.
   */
  static validation_level parse_validation_level(std::string_view name);

  /**
   * Returns a (human-readable) representation of the given validation level.
   *
   * This method is safe to call with any value of the underlying type of
   * validation_level, but will only return a stable, well-known name if `lvl`
   * is one of the explicitly declared values of the enum type. In that case,
   * the returned name will match the input of `parse_validation_level` that
   * returns the given `lvl`.
   *
   * Otherwise, returns human-readable but unspecified representation that is
   * not guaranteed to be stable across invocations, and will throw if passed
   * to `parse_validation_level` (eg. "<unknown validation_level: 42>")
   */
  static std::string validation_level_to_string(validation_level lvl);

  bool skip_lowering_annotations = false;

  bool skip_lowering_cpp_type_annotations = false;

  // If true, will issue a warning if a default value is explicitly specified
  // for a field, but that value is equal to the intrinsic default value.
  validation_level redundant_custom_default_values = validation_level::none;

  bool forbid_unstructured_annotations = false;

  // Action to take when @cpp.EnableCustomTypeOrdering is present on a
  // (structured) type that does not need it.
  validation_level unnecessary_enable_custom_type_ordering =
      validation_level::none;

  // Action to take on typedef with URI (but without the annotation that
  // explicitly allows it, i.e. @thrift.AllowLegacyTypedefUri).
  validation_level nonallowed_typedef_with_uri = validation_level::warn;

  // Action to take on optional fields in structs (and exceptions) that have a
  // custom default value.
  validation_level struct_optional_field_custom_default =
      validation_level::error;

  // Action to take on union fields (which are implicitly optional) that have a
  // custom default value.
  validation_level union_field_custom_default = validation_level::warn;

  // Action to take on `required` (struct and exception) fields.
  validation_level required_field_qualifier = validation_level::warn;

  // Action to take on files without a package.
  validation_level missing_package = validation_level::warn;
};

// An AST visitor context for semantic analysis. It combines diagnostics
// reporting and node metadata cache.
class sema_context : public diagnostics_engine, public const_visitor_context {
 public:
  using diagnostics_engine::diagnostics_engine;

  explicit sema_context(diagnostics_engine& diags)
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

  const sema_params& sema_parameters() const { return params_; }
  sema_params& sema_parameters() { return params_; }

 private:
  node_metadata_cache cache_;
  sema_params params_;
};

} // namespace apache::thrift::compiler
