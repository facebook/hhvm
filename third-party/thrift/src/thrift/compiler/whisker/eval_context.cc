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

#include <thrift/compiler/whisker/eval_context.h>

#include <algorithm>
#include <functional>
#include <iterator>

#include <fmt/core.h>

namespace w = whisker::make;

namespace whisker {

namespace {

object::ptr find_property(const object& o, std::string_view identifier) {
  using result = object::ptr;
  return o.visit(
      [](null) -> result { return nullptr; },
      [](i64) -> result { return nullptr; },
      [](f64) -> result { return nullptr; },
      [](const string&) -> result { return nullptr; },
      [](boolean) -> result { return nullptr; },
      [](const array&) -> result { return nullptr; },
      [identifier](const native_object::ptr& o) -> result {
        if (auto map_like = o->as_map_like()) {
          return map_like->lookup_property(identifier);
        }
        return nullptr;
      },
      [](const native_function::ptr&) -> result { return nullptr; },
      [identifier](const map& m) -> result {
        if (auto it = m.find(identifier); it != m.end()) {
          return manage_as_static(it->second);
        }
        return nullptr;
      });
}

/**
 * A class representing the bag of properties at the global scope (even before
 * the root scope).
 *
 * This could be a w::map but for debugging purposes, a native_object with a
 * custom print_to function is beneficial.
 */
class global_scope_object
    : public native_object,
      public native_object::map_like,
      public std::enable_shared_from_this<global_scope_object> {
 public:
  explicit global_scope_object(map properties)
      : properties_(std::move(properties)) {}

  native_object::map_like::ptr as_map_like() const override {
    return shared_from_this();
  }

  object::ptr lookup_property(std::string_view identifier) const override {
    if (auto property = properties_.find(identifier);
        property != properties_.end()) {
      return manage_as_static(property->second);
    }
    return nullptr;
  }

  void print_to(tree_printer::scope scope, const object_print_options& options)
      const override {
    scope.println("<global scope> (size={})", properties_.size());
    for (const auto& [key, value] : properties_) {
      auto element_scope = scope.open_transparent_property();
      element_scope.println("'{}'", key);
      whisker::print_to(value, element_scope.open_node(), options);
    }
  }

 private:
  map properties_;
};

} // namespace

object::ptr eval_context::lexical_scope::lookup_property(
    std::string_view identifier) {
  if (auto local = locals_.find(identifier); local != locals_.end()) {
    return manage_as_static(local->second);
  }
  return find_property(this_ref_, identifier);
}

eval_context::eval_context(map globals)
    : global_scope_(
          w::make_native_object<global_scope_object>(std::move(globals))),
      stack_({lexical_scope(global_scope_)}) {}

/* static */ eval_context eval_context::with_root_scope(
    const object& root_scope, map globals) {
  eval_context result{std::move(globals)};
  result.push_scope(root_scope);
  return result;
}

eval_context::~eval_context() noexcept = default;

std::size_t eval_context::stack_depth() const {
  // The global scope is always on the stack but should not count towards the
  // depth (it's at depth 0).
  return stack_.size() - 1;
}

void eval_context::push_scope(const object& object) {
  stack_.emplace_back(object);
}

void eval_context::pop_scope() {
  // The global scope cannot be popped.
  assert(stack_depth() > 0);
  stack_.pop_back();
}

const object& eval_context::global_scope() const {
  return global_scope_;
}

expected<std::monostate, eval_name_already_bound_error>
eval_context::bind_local(std::string name, object value) {
  assert(!stack_.empty());
  if (auto [_, inserted] =
          stack_.back().locals().insert(std::pair{name, std::move(value)});
      !inserted) {
    return unexpected(eval_name_already_bound_error(std::move(name)));
  }
  return {};
}

expected<
    object::ptr,
    std::variant<eval_scope_lookup_error, eval_property_lookup_error>>
eval_context::lookup_object(const std::vector<std::string>& path) {
  assert(!stack_.empty());

  if (path.empty()) {
    // Lookup is {{.}}
    return manage_as_static(stack_.back().this_ref());
  }

  auto current = std::invoke([&]() -> object::ptr {
    const auto& id = path.front();
    // Crawl up through the scope stack since names can be shadowed.
    for (auto scope = stack_.rbegin(); scope != stack_.rend(); ++scope) {
      if (auto result = scope->lookup_property(id)) {
        return result;
      }
    }
    return nullptr;
  });

  if (current == nullptr) {
    std::vector<object> searched_scopes;
    searched_scopes.reserve(stack_.size());
    // Searching happened in reverse order.
    std::transform(
        stack_.rbegin(),
        stack_.rend(),
        std::back_inserter(searched_scopes),
        [](const auto& scope) { return scope.this_ref(); });
    return unexpected(
        eval_scope_lookup_error(path.front(), std::move(searched_scopes)));
  }

  for (auto component = std::next(path.begin()); component != path.end();
       ++component) {
    object::ptr next = find_property(*current, *component);
    if (next == nullptr) {
      return unexpected(eval_property_lookup_error(
          *current, /* missing_from */
          std::vector<std::string>(path.begin(), component), /* success_path */
          *component /* missing_name */));
    }
    current = next;
  }

  assert(current != nullptr);
  return current;
}

} // namespace whisker
