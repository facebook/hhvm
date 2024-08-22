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
#include <map>

#include <fmt/core.h>

namespace whisker {

namespace {
std::optional<object> find_property(
    const object& o, std::string_view identifier) {
  using result = std::optional<object>;
  return o.visit(
      [](null) -> result { return std::nullopt; },
      [](i64) -> result { return std::nullopt; },
      [](f64) -> result { return std::nullopt; },
      [](const string&) -> result { return std::nullopt; },
      [](boolean) -> result { return std::nullopt; },
      [](const array&) -> result { return std::nullopt; },
      [identifier](const native_object::ptr& o) -> result {
        return o->lookup_property(identifier);
      },
      [identifier](const map& m) -> result {
        if (auto it = m.find(identifier); it != m.end()) {
          return it->second;
        }
        return std::nullopt;
      });
}
} // namespace

std::optional<object> eval_context::lexical_scope::lookup_property(
    std::string_view identifier) {
  if (auto local = locals_.find(identifier); local != locals_.end()) {
    return local->second;
  }
  return find_property(this_ref_, identifier);
}

eval_context::eval_context(object root_scope)
    : stack_({lexical_scope(std::move(root_scope))}) {}

eval_context::~eval_context() noexcept = default;

std::size_t eval_context::stack_depth() const {
  return stack_.size();
}

void eval_context::push_scope(object object) {
  stack_.emplace_back(std::move(object));
}

void eval_context::pop_scope() {
  // The root scope cannot be popped.
  assert(stack_depth() > 1);
  stack_.pop_back();
}

void eval_context::bind_local(std::string name, object value) {
  assert(!stack_.empty());
  if (auto [_, inserted] =
          stack_.back().locals().insert(std::pair{name, std::move(value)});
      !inserted) {
    throw eval_name_already_bound_error(std::move(name));
  }
}

object eval_context::lookup_object(const std::vector<std::string>& path) {
  assert(!stack_.empty());

  if (path.empty()) {
    // Lookup is {{.}}
    return stack_.back().this_ref();
  }

  auto current = std::invoke([&]() -> std::optional<object> {
    auto id = path.front();
    // Crawl up through the scope stack since names can be shadowed.
    for (auto scope = stack_.rbegin(); scope != stack_.rend(); ++scope) {
      if (auto result = scope->lookup_property(id)) {
        return result;
      }
    }
    return std::nullopt;
  });

  if (!current.has_value()) {
    std::vector<object> searched_scopes;
    searched_scopes.reserve(stack_.size());
    // Searching happened in reverse order.
    std::transform(
        stack_.rbegin(),
        stack_.rend(),
        std::back_inserter(searched_scopes),
        [](const auto& scope) { return scope.this_ref(); });
    throw eval_scope_lookup_error(path.front(), std::move(searched_scopes));
  }

  for (auto component = std::next(path.begin()); component != path.end();
       ++component) {
    std::optional<object> next = find_property(*current, *component);
    if (!next.has_value()) {
      throw eval_property_lookup_error(
          *current, /* missing_from */
          std::vector<std::string>(path.begin(), component), /* success_path */
          *component /* missing_name */);
    }
    current.emplace(std::move(*next));
  }

  assert(current.has_value());
  return *current;
}

eval_scope_lookup_error::eval_scope_lookup_error(
    std::string property_name, std::vector<object> searched_scopes)
    : eval_error(fmt::format("name '{}' not found", property_name)),
      property_name_(std::move(property_name)),
      searched_scopes_(std::move(searched_scopes)) {}

eval_property_lookup_error::eval_property_lookup_error(
    object missing_from,
    std::vector<std::string> success_path,
    std::string property_name)
    : eval_error(fmt::format("name '{}' not found", property_name)),
      missing_from_(std::move(missing_from)),
      success_path_(std::move(success_path)),
      property_name_(std::move(property_name)) {}

eval_name_already_bound_error::eval_name_already_bound_error(std::string name)
    : eval_error(fmt::format("name '{}' is already bound", name)),
      name_(std::move(name)) {}

} // namespace whisker
