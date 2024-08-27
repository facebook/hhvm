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

#include <ostream>
#include <sstream>

namespace whisker {

namespace {

struct to_string_visitor {
  static void visit(i64 value, tree_printer::scope scope) {
    scope.println("i64({})", value);
  }

  static void visit(f64 value, tree_printer::scope scope) {
    scope.println("f64({})", value);
  }

  static void visit(const std::string& value, tree_printer::scope scope) {
    scope.println("'{}'", tree_printer::escape(value));
  }

  static void visit(bool value, tree_printer::scope scope) {
    scope.println("{}", value ? "true" : "false");
  }

  static void visit(null, tree_printer::scope scope) { scope.println("null"); }

  static void visit(const array& arr, tree_printer::scope scope) {
    scope.println("array (size={})", arr.size());
    for (std::size_t i = 0; i < arr.size(); ++i) {
      auto element_scope = scope.open_property();
      element_scope.println("[{}]", i);
      visit(arr[i], element_scope.open_node());
    }
  }

  static void visit(const map& m, tree_printer::scope scope) {
    scope.println("map (size={})", m.size());
    for (const auto& [key, value] : m) {
      auto element_scope = scope.open_property();
      element_scope.println("'{}'", key);
      visit(value, element_scope.open_node());
    }
  }

  static void visit(const native_object::ptr& o, tree_printer::scope scope) {
    o->print_to(std::move(scope));
  }

  // Prevent implicit conversion to whisker::object. Otherwise, we can silently
  // compile an infinitely recursive visit() chain if there is a missing
  // overload for one of the alternatives in the variant.
  template <
      typename T = object,
      typename = std::enable_if_t<std::is_same_v<T, object>>>
  static void visit(const T& value, tree_printer::scope scope) {
    value.visit([&](auto&& alternative) { visit(alternative, scope); });
  }
};

} // namespace

void native_object::print_to(tree_printer::scope scope) const {
  scope.println("<native_object>");
}

std::string to_string(const object& obj) {
  std::ostringstream out;
  print_to(obj, tree_printer::scope::make_root(out));
  return std::move(out).str();
}

void print_to(const object& obj, tree_printer::scope scope) {
  to_string_visitor::visit(obj, std::move(scope));
}

std::ostream& operator<<(std::ostream& out, const object& o) {
  return out << to_string(o);
}

} // namespace whisker
