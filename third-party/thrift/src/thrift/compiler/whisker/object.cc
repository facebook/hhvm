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

#include <thrift/compiler/whisker/object.h>

#include <thrift/compiler/whisker/detail/overload.h>

#include <cassert>
#include <ostream>
#include <sstream>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <boost/core/demangle.hpp>

namespace whisker {

namespace {

std::string demangle(const std::type_info& type) {
  return boost::core::demangle(type.name());
}

class to_string_visitor {
 public:
  explicit to_string_visitor(const object_print_options& opts) : opts_(opts) {}

  // Prevent implicit conversion to whisker::object. Otherwise, we can silently
  // compile an infinitely recursive visit() chain if there is a missing
  // overload for one of the alternatives in the variant.
  template <
      typename T = object,
      typename = std::enable_if_t<std::is_same_v<T, object>>>
  void visit(const T& value, tree_printer::scope scope) const {
    value.visit(
        [&](const array& a) { visit_maybe_truncate(a, std::move(scope)); },
        [&](const map& m) { visit_maybe_truncate(m, std::move(scope)); },
        [&](const native_object::ptr& o) {
          visit_maybe_truncate(o, std::move(scope));
        },
        [&](const native_function::ptr& f) {
          visit_maybe_truncate(f, std::move(scope));
        },
        [&](const native_handle<>& h) {
          visit_maybe_truncate(h, std::move(scope));
        },
        // All other types are printed inline so no truncation is necessary.
        [&](auto&& alternative) { visit(alternative, std::move(scope)); });
  }

 private:
  template <typename T>
  void visit_maybe_truncate(const T& value, tree_printer::scope scope) const {
    if (at_max_depth(scope)) {
      scope.println("...");
      return;
    }
    visit(value, scope);
  }

  void visit(i64 value, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("i64({})", value);
  }

  void visit(f64 value, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("f64({})", value);
  }

  void visit(const std::string& value, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("'{}'", tree_printer::escape(value));
  }

  void visit(bool value, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("{}", value ? "true" : "false");
  }

  void visit(null, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("null");
  }

  void visit(const array& arr, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("array (size={})", arr.size());
    for (std::size_t i = 0; i < arr.size(); ++i) {
      auto element_scope = scope.open_transparent_property();
      element_scope.println("[{}]", i);
      visit(arr[i], element_scope.open_node());
    }
  }

  void visit(const map& m, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("map (size={})", m.size());
    for (const auto& [key, value] : m) {
      auto element_scope = scope.open_transparent_property();
      element_scope.println("'{}'", key);
      visit(value, element_scope.open_node());
    }
  }

  void visit(const native_object::ptr& o, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    o->print_to(std::move(scope), opts_);
  }

  void visit(const native_function::ptr& f, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    f->print_to(std::move(scope), opts_);
  }

  void visit(const native_handle<>& handle, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    scope.println("<native_handle type='{}'>", demangle(handle.type()));
    if (const prototype<>::ptr& proto = handle.proto()) {
      visit_maybe_truncate(proto, scope.open_node());
    }
  }

  void visit(const prototype<>::ptr& proto, tree_printer::scope scope) const {
    require_within_max_depth(scope);
    std::set<std::string> keys = proto->keys();
    scope.println("<prototype (size={})>", keys.size());
    for (const auto& key : keys) {
      auto element_scope = scope.open_transparent_property();
      element_scope.println("'{}'", key);
    }
    if (const prototype<>::ptr& parent = proto->parent()) {
      visit_maybe_truncate(parent, scope.open_node());
    }
  }

  [[nodiscard]] bool at_max_depth(const tree_printer::scope& scope) const {
    return scope.semantic_depth() == opts_.max_depth;
  }

  void require_within_max_depth(
      [[maybe_unused]] const tree_printer::scope& scope) const {
    assert(scope.semantic_depth() <= opts_.max_depth);
  }

  const object_print_options& opts_;
};

} // namespace

namespace detail {
bool array_eq(const array& lhs, const array& rhs) {
  return lhs == rhs;
}
bool array_eq(const array& lhs, const native_object::array_like::ptr& rhs) {
  if (rhs == nullptr) {
    return false;
  }
  std::size_t size = lhs.size();
  if (size != rhs->size()) {
    return false;
  }
  for (std::size_t i = 0; i < size; ++i) {
    if (lhs[i] != *rhs->at(i)) {
      return false;
    }
  }
  return true;
}
bool array_eq(const native_object::array_like::ptr& lhs, const array& rhs) {
  return array_eq(rhs, lhs);
}
bool array_eq(
    const native_object::array_like::ptr& lhs,
    const native_object::array_like::ptr& rhs) {
  if (lhs == nullptr) {
    return false;
  }
  if (rhs == nullptr) {
    return false;
  }
  std::size_t size = lhs->size();
  if (size != rhs->size()) {
    return false;
  }
  for (std::size_t i = 0; i < size; ++i) {
    object::ptr lhs_value = lhs->at(i);
    object::ptr rhs_value = rhs->at(i);
    // Within size so it must not be null.
    assert(lhs_value != nullptr);
    assert(rhs_value != nullptr);
    if (*lhs_value != *rhs_value) {
      return false;
    }
  }
  return true;
}

bool map_eq(const map& lhs, const map& rhs) {
  return lhs == rhs;
}
bool map_eq(const map& lhs, const native_object::map_like::ptr& rhs) {
  if (rhs == nullptr) {
    return false;
  }
  auto rhs_keys = rhs->keys();
  if (!rhs_keys.has_value()) {
    // Not enumerable
    return false;
  }
  if (lhs.size() != rhs_keys->size()) {
    return false;
  }

  for (const auto& [key, lhs_value] : lhs) {
    if (rhs_keys->find(key) == rhs_keys->end()) {
      return false;
    }
    auto rhs_value = rhs->lookup_property(key);
    // Key was enumerated so it must not be null.
    assert(rhs_value != nullptr);
    if (lhs_value != *rhs_value) {
      return false;
    }
  }
  return true;
}
bool map_eq(const native_object::map_like::ptr& lhs, const map& rhs) {
  return map_eq(rhs, lhs);
}
bool map_eq(
    const native_object::map_like::ptr& lhs,
    const native_object::map_like::ptr& rhs) {
  if (lhs == nullptr || rhs == nullptr) {
    return false;
  }

  auto lhs_keys = lhs->keys();
  auto rhs_keys = rhs->keys();
  const bool keys_equal =
      lhs_keys.has_value() && rhs_keys.has_value() && *lhs_keys == *rhs_keys;
  if (!keys_equal) {
    return false;
  }
  for (const std::string& key : *lhs_keys) {
    object::ptr lhs_value = lhs->lookup_property(key);
    object::ptr rhs_value = rhs->lookup_property(key);
    if (*lhs_value != *rhs_value) {
      return false;
    }
  }
  return true;
}
} // namespace detail

void native_object::map_like::default_print_to(
    std::string_view name,
    const std::set<std::string>& property_names,
    tree_printer::scope scope,
    const object_print_options& options) const {
  assert(scope.semantic_depth() <= options.max_depth);
  const auto size = property_names.size();
  scope.println("{} (size={})", name, size);

  for (const std::string& key : property_names) {
    auto cached = lookup_property(key);
    assert(cached != nullptr);
    auto element_scope = scope.open_transparent_property();
    element_scope.println("'{}'", key);
    whisker::print_to(*cached, element_scope.open_node(), options);
  }
}

void native_object::array_like::default_print_to(
    std::string_view name,
    tree_printer::scope scope,
    const object_print_options& options) const {
  assert(scope.semantic_depth() <= options.max_depth);

  const auto sz = size();
  scope.println("{} (size={})", name, sz);
  for (std::size_t i = 0; i < sz; ++i) {
    auto element_scope = scope.open_transparent_property();
    element_scope.println("[{}]", i);
    whisker::print_to(*at(i), element_scope.open_node(), options);
  }
}

void native_object::print_to(
    tree_printer::scope scope, const object_print_options&) const {
  scope.println("<native_object>");
}

std::string native_object::describe_type() const {
  return fmt::format("<native_object type='{}'>", demangle(typeid(*this)));
}

bool native_object::operator==(const native_object& other) const {
  if (&other == this) {
    return true;
  }
  if (detail::array_eq(as_array_like(), other.as_array_like())) {
    return true;
  }
  return detail::map_eq(as_map_like(), other.as_map_like());
}

void native_function::print_to(
    tree_printer::scope scope, const object_print_options&) const {
  scope.println("<native_function>");
}

std::string native_function::describe_type() const {
  return fmt::format("<native_function type='{}'>", demangle(typeid(*this)));
}

/* static */ prototype<>::ptr prototype<>::from(
    descriptors_map descriptors, prototype::ptr parent) {
  return std::make_shared<basic_prototype<>>(
      std::move(descriptors), std::move(parent));
}

std::string detail::describe_native_handle_for_type(
    const std::type_info& type) {
  return fmt::format("<native_handle type='{}'>", demangle(type));
}

std::string native_handle<void>::describe_type() const {
  return detail::describe_native_handle_for_type(type());
}
/* static */ std::string native_handle<void>::describe_class_type() {
  return "<native_handle type='void'>";
}

std::string object::describe_type() const {
  return visit(
      [](i64) -> std::string { return "i64"; },
      [](f64) -> std::string { return "f64"; },
      [](const string&) -> std::string { return "string"; },
      [](boolean) -> std::string { return "boolean"; },
      [](null) -> std::string { return "null"; },
      [](const array&) -> std::string { return "array"; },
      [](const map&) -> std::string { return "map"; },
      [](const native_object::ptr& o) -> std::string {
        return o->describe_type();
      },
      [](const native_function::ptr& f) -> std::string {
        return f->describe_type();
      },
      [](const native_handle<>& h) -> std::string {
        return h.describe_type();
      });
}

std::string to_string(const object& obj, const object_print_options& options) {
  std::ostringstream out;
  print_to(obj, tree_printer::scope::make_root(out), options);
  return std::move(out).str();
}

void print_to(
    const object& obj,
    tree_printer::scope scope,
    const object_print_options& options) {
  to_string_visitor(options).visit(obj, std::move(scope));
}

std::ostream& operator<<(std::ostream& out, const object& o) {
  return out << to_string(o);
}

} // namespace whisker
