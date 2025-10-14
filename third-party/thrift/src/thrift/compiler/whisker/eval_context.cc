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
#include <iterator>
#include <utility>

#include <fmt/core.h>

namespace w = whisker::make;

namespace whisker {

namespace {

/**
 * A class representing the bag of properties at the global scope (even before
 * the root scope).
 *
 * This is a bespoke implementation primarily for debugging purposes.
 */
class global_scope_object : public map {
 public:
  explicit global_scope_object(map::raw properties)
      : properties_(std::move(properties)) {}

  std::optional<object> lookup_property(
      std::string_view identifier) const override {
    if (auto property = properties_.find(identifier);
        property != properties_.end()) {
      return property->second;
    }
    return std::nullopt;
  }

  void print_to(tree_printer::scope& scope, const object_print_options& options)
      const override {
    scope.print("<global scope> (size={})", properties_.size());
    for (const auto& [key, value] : properties_) {
      whisker::print_to(value, scope.make_child("'{}' → ", key), options);
    }
  }

 private:
  map::raw properties_;
};

} // namespace

namespace detail {

std::optional<object> find_property(
    diagnostics_engine& diags,
    const object& self,
    const ast::variable_component& component) {
  using result = std::optional<object>;
  return self.visit(
      [](null) -> result { return std::nullopt; },
      [](i64) -> result { return std::nullopt; },
      [](f64) -> result { return std::nullopt; },
      [](const string&) -> result { return std::nullopt; },
      [](boolean) -> result { return std::nullopt; },
      [](const array::ptr&) -> result { return std::nullopt; },
      [&](const map::ptr& m) -> result {
        // A map doesn't have a prototype, so we treat the lookup as just a
        // simple string key regardless of qualifier.
        // Specifically for the case of mstch_compat, mstch_object_proxy is in
        // fact a Whisker map, so this also provides backwards compatibility for
        // legacy mstch properties which have colons in the name.
        return m->lookup_property(component.as_string());
      },
      [](const native_function::ptr&) -> result { return std::nullopt; },
      [&](const native_handle<>& h) -> result {
        if (const auto* descriptor = h.proto()->find_descriptor(
                component.qualifier.has_value() ? component.qualifier->name
                                                : "",
                component.property.name)) {
          return detail::variant_match(
              *descriptor,
              [&](const prototype<>::property& prop) -> object {
                return prop.function->invoke(native_function::context{
                    component.loc,
                    diags,
                    self,
                    {} /* positional arguments */,
                    {} /* named arguments */,
                });
              },
              [&](const prototype<>::fixed_object& fixed) -> object {
                return fixed.value;
              });
        }
        return std::nullopt;
      });
}

} // namespace detail

std::optional<object> eval_context::lexical_scope::lookup_property(
    diagnostics_engine& diags, const ast::variable_component& component) {
  if (auto local = locals_.find(component.as_string());
      local != locals_.end()) {
    return local->second;
  }
  return detail::find_property(diags, this_ref_, component);
}

eval_context::eval_context(diagnostics_engine& diags, object globals)
    : diags_(diags),
      global_scope_(std::move(globals)),
      stack_({lexical_scope(global_scope_)}) {}

eval_context::eval_context(diagnostics_engine& diags, map::raw globals)
    : eval_context(
          diags, w::make_map<global_scope_object>(std::move(globals))) {}

/* static */ eval_context eval_context::with_root_scope(
    diagnostics_engine& diags, object root_scope, map::raw globals) {
  eval_context result{diags, std::move(globals)};
  result.push_scope(std::move(root_scope));
  return result;
}

eval_context::~eval_context() noexcept = default;

std::size_t eval_context::stack_depth() const {
  // The global scope is always on the stack but should not count towards the
  // depth (it's at depth 0).
  return stack_.size() - 1;
}

void eval_context::push_scope(object object) {
  stack_.emplace_back(std::move(object));
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

expected<eval_context::lookup_result, eval_context::lookup_error>
eval_context::look_up_object(const ast::variable_lookup& lookup) {
  assert(!stack_.empty());
  using result = expected<lookup_result, lookup_error>;

  return detail::variant_match(
      lookup.chain,
      [&](ast::variable_lookup::this_ref) -> result {
        return lookup_result::without_parent(stack_.back().this_ref());
      },
      [&](const std::vector<ast::variable_component>& path) -> result {
        const auto make_searched_scopes = [&]() -> std::vector<object> {
          std::vector<object> result;
          result.reserve(stack_.size());
          // Searching happened in reverse order.
          std::transform(
              stack_.rbegin(),
              stack_.rend(),
              std::back_inserter(result),
              [](const auto& scope) { return scope.this_ref(); });
          return result;
        };

        using component_iterator =
            std::vector<ast::variable_component>::const_iterator;
        const auto make_success_path =
            [](component_iterator begin,
               component_iterator end) -> std::vector<std::string> {
          std::vector<std::string> result;
          result.reserve(std::distance(begin, end));
          std::transform(
              begin,
              end,
              std::back_inserter(result),
              [](const auto& component) { return component.as_string(); });
          return result;
        };

        std::optional<object> current;
        // Crawl up through the scope stack since names can be shadowed.
        for (auto scope = stack_.rbegin(); scope != stack_.rend(); ++scope) {
          try {
            if (auto result = scope->lookup_property(diags_, path.front())) {
              current = result;
              break;
            }
          } catch (const eval_error& err) {
            return unexpected(eval_scope_lookup_error(
                path.front().as_string(),
                make_searched_scopes(),
                err.what() /* cause */));
          }
        }

        if (!current.has_value()) {
          return unexpected(eval_scope_lookup_error(
              path.front().as_string(), make_searched_scopes()));
        }
        object parent = whisker::make::null;

        for (auto component = std::next(path.begin()); component != path.end();
             ++component) {
          try {
            std::optional<object> next =
                detail::find_property(diags_, *current, *component);
            if (!next.has_value()) {
              return unexpected(eval_property_lookup_error(
                  *current, /* missing_from */
                  make_success_path(path.begin(), component),
                  component->as_string() /* missing_name */));
            }
            parent = *current;
            current = next;
          } catch (const eval_error& err) {
            return unexpected(eval_property_lookup_error(
                *current, /* missing_from */
                make_success_path(path.begin(), component),
                component->as_string() /* missing_name */,
                err.what() /* cause */));
          }
        }

        assert(current.has_value());
        return lookup_result{*current, parent};
      });
}

} // namespace whisker
