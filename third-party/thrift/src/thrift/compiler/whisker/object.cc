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

#include <thrift/common/detail/string.h>
#include <thrift/compiler/whisker/detail/overload.h>

#include <cassert>
#include <ostream>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <boost/core/demangle.hpp>

namespace whisker {

namespace {
using apache::thrift::detail::escape;

std::string demangle(const std::type_info& type) {
  return boost::core::demangle(type.name());
}

class to_string_visitor {
 public:
  explicit to_string_visitor(const object_print_options& opts) : opts_(opts) {}

  // Prevent implicit conversion to whisker::object. Otherwise, we can silently
  // compile an infinitely recursive visit() chain if there is a missing
  // overload for one of the alternatives in the variant.
  void visit(const std::same_as<object> auto& value, tree_printer::scope& scope)
      const {
    value.visit(
        [&](const array::ptr& a) { visit_maybe_truncate(a, scope); },
        [&](const map::ptr& m) { visit_maybe_truncate(m, scope); },
        [&](const native_function::ptr& f) { visit_maybe_truncate(f, scope); },
        [&](const native_handle<>& h) { visit_maybe_truncate(h, scope); },
        // All other types are printed inline so no truncation is necessary.
        [&](auto&& alternative) { visit(alternative, scope); });
  }

 private:
  template <typename T>
  void visit_maybe_truncate(const T& value, tree_printer::scope& scope) const {
    if (at_max_depth(scope)) {
      scope.print("...");
      return;
    }
    visit(value, scope);
  }

  void visit(i64 value, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    scope.print("i64({})", value);
  }

  void visit(f64 value, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    scope.print("f64({})", value);
  }

  void visit(const std::string& value, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    scope.print("'{}'", escape(value));
  }

  void visit(bool value, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    scope.print("{}", value ? "true" : "false");
  }

  void visit(null, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    scope.print("null");
  }

  void visit(const map::ptr& m, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    m->print_to(scope, opts_);
  }

  void visit(const array::ptr& a, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    a->print_to(scope, opts_);
  }

  void visit(const native_function::ptr& f, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    f->print_to(scope, opts_);
  }

  void visit(const native_handle<>& handle, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    scope.print("<native_handle type='{}'>", demangle(handle.type()));
    if (const prototype<>::ptr& proto = handle.proto()) {
      visit_maybe_truncate(proto, scope.make_child());
    }
  }

  void visit(const prototype<>::ptr& proto, tree_printer::scope& scope) const {
    require_within_max_depth(scope);
    std::set<std::string> keys = proto->keys();
    scope.print("<prototype (size={})>", keys.size());
    for (const auto& key : keys) {
      scope.make_child("'{}'", key);
    }
    if (const prototype<>::ptr& parent = proto->parent()) {
      visit_maybe_truncate(parent, scope.make_child());
    }
  }

  [[nodiscard]] bool at_max_depth(const tree_printer::scope& scope) const {
    return scope.depth() == opts_.max_depth;
  }

  void require_within_max_depth(
      [[maybe_unused]] const tree_printer::scope& scope) const {
    assert(scope.depth() <= opts_.max_depth);
  }

  const object_print_options& opts_;
};

} // namespace

namespace {
class basic_map final : public map {
 public:
  std::optional<object> lookup_property(
      std::string_view identifier) const final {
    if (auto found = raw_.find(identifier); found != raw_.end()) {
      return found->second;
    }
    return std::nullopt;
  }

  std::optional<std::set<std::string>> keys() const final {
    std::set<std::string> keys;
    for (const auto& [key, _] : raw_) {
      keys.insert(key);
    }
    return keys;
  }

  void print_to(tree_printer::scope& scope, const object_print_options& options)
      const final {
    default_print_to("map", *keys(), scope, options);
  }

  std::string describe_type() const final {
    // The built-in map type does not need to be more descriptive.
    return "map";
  }

  explicit basic_map(map::raw raw) : raw_(std::move(raw)) {}

 private:
  map::raw raw_;
};
} // namespace
/* static */ map::ptr map::of(map::raw raw) {
  return std::make_shared<basic_map>(std::move(raw));
}

void map::print_to(
    tree_printer::scope& scope, const object_print_options& options) const {
  std::optional<std::set<std::string>> property_names = keys();
  if (!property_names.has_value()) {
    scope.print("map [custom] (not enumerable)");
    return;
  }
  default_print_to(
      "map [custom]", std::move(property_names).value(), scope, options);
}

std::string map::describe_type() const {
  return fmt::format("map [custom]='{}'>", demangle(typeid(*this)));
}

bool operator==(const map& lhs, const map& rhs) {
  if (std::addressof(lhs) == std::addressof(rhs)) {
    return true;
  }

  auto lhs_keys = lhs.keys();
  auto rhs_keys = rhs.keys();
  const bool keys_equal =
      lhs_keys.has_value() && rhs_keys.has_value() && *lhs_keys == *rhs_keys;
  if (!keys_equal) {
    return false;
  }
  for (const std::string& key : *lhs_keys) {
    std::optional<object> lhs_value = lhs.lookup_property(key);
    std::optional<object> rhs_value = rhs.lookup_property(key);
    // These should always be present because we are only attempting to fetch
    // enumerable keys.
    assert(lhs_value.has_value());
    assert(rhs_value.has_value());
    if (*lhs_value != *rhs_value) {
      return false;
    }
  }
  return true;
}

void map::default_print_to(
    std::string_view name,
    const std::set<std::string>& property_names,
    tree_printer::scope& scope,
    const object_print_options& options) const {
  assert(scope.depth() <= options.max_depth);
  const auto size = property_names.size();
  scope.print("{} (size={})", name, size);

  for (const std::string& key : property_names) {
    auto cached = lookup_property(key);
    assert(cached.has_value());
    tree_printer::scope& element_scope = scope.make_child();
    element_scope.print("'{}' → ", key);
    whisker::print_to(*cached, element_scope, options);
  }
}

namespace {
class basic_array final : public array {
 public:
  std::size_t size() const final { return raw_.size(); }
  object at(std::size_t index) const final { return raw_.at(index); }

  void print_to(tree_printer::scope& scope, const object_print_options& options)
      const final {
    default_print_to("array", scope, options);
  }

  std::string describe_type() const final {
    // The built-in array type does not need to be more descriptive.
    return "array";
  }

  explicit basic_array(array::raw raw) : raw_(std::move(raw)) {}

 private:
  array::raw raw_;
};
} // namespace
/* static */ array::ptr array::of(array::raw raw) {
  return std::make_shared<basic_array>(std::move(raw));
}

void array::print_to(
    tree_printer::scope& scope, const object_print_options& options) const {
  default_print_to("array [custom]", scope, options);
}

std::string array::describe_type() const {
  return fmt::format("array [custom]='{}'", demangle(typeid(*this)));
}

bool operator==(const array& lhs, const array& rhs) {
  if (std::addressof(lhs) == std::addressof(rhs)) {
    return true;
  }

  std::size_t size = lhs.size();
  if (size != rhs.size()) {
    return false;
  }
  for (std::size_t i = 0; i < size; ++i) {
    object lhs_value = lhs.at(i);
    object rhs_value = rhs.at(i);
    if (lhs_value != rhs_value) {
      return false;
    }
  }
  return true;
}

void array::default_print_to(
    std::string_view name,
    tree_printer::scope& scope,
    const object_print_options& options) const {
  assert(scope.depth() <= options.max_depth);

  const auto sz = size();
  scope.print("{} (size={})", name, sz);
  for (std::size_t i = 0; i < sz; ++i) {
    tree_printer::scope& element_scope = scope.make_child();
    element_scope.print("[{}] ", i);
    whisker::print_to(at(i), element_scope, options);
  }
}

void native_function::print_to(
    tree_printer::scope& scope, const object_print_options&) const {
  scope.print("<native_function>");
}

std::string native_function::describe_type() const {
  return fmt::format("<native_function type='{}'>", demangle(typeid(*this)));
}

/* static */ prototype<>::ptr prototype<>::from(
    descriptors_map descriptors,
    prototype::ptr parent,
    const std::string_view& name) {
  return std::make_shared<basic_prototype<>>(
      std::move(descriptors), std::move(parent), name);
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
      [](const array::ptr& a) -> std::string { return a->describe_type(); },
      [](const map::ptr& m) -> std::string { return m->describe_type(); },
      [](const native_function::ptr& f) -> std::string {
        return f->describe_type();
      },
      [](const native_handle<>& h) -> std::string {
        return h.describe_type();
      });
}

std::string to_string(const object& obj, const object_print_options& options) {
  auto scope = tree_printer::scope::make_root();
  print_to(obj, scope, options);
  return tree_printer::to_string(scope);
}

void print_to(
    const object& obj,
    tree_printer::scope& scope,
    const object_print_options& options) {
  to_string_visitor(options).visit(obj, scope);
}

std::ostream& operator<<(std::ostream& out, const object& o) {
  return out << to_string(o);
}

} // namespace whisker
