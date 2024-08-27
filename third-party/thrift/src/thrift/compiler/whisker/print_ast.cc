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

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/detail/tree_printer.h>
#include <thrift/compiler/whisker/print_ast.h>

#include <iterator>
#include <memory>
#include <ostream>
#include <string_view>

#include <fmt/core.h>

namespace whisker {

namespace {

namespace tree_printer = detail::tree_printer;

std::string to_string(
    const source_range& loc, const source_manager& src_manager) {
  resolved_location begin(loc.begin, src_manager);
  resolved_location end(loc.end, src_manager);
  // Format of a source location is:
  //   line:<line>:<column>
  // If begin and end are on the same line, the the end source location is:
  //   col:<column>
  return fmt::format(
      "<line:{}:{}, {}>",
      begin.line(),
      begin.column(),
      begin.line() == end.line()
          ? fmt::format("col:{}", end.column())
          : fmt::format("line:{}:{}", end.line(), end.column()));
}

struct ast_visitor {
  std::string location(const source_range& loc) const {
    return to_string(loc, src_manager);
  }

  void visit(const ast::identifier& id, tree_printer::scope scope) const {
    scope.println(" identifier '{}'", id.name);
  }
  void visit(const ast::text& text, tree_printer::scope scope) const {
    scope.println(
        " text {} '{}'", location(text.loc), tree_printer::escape(text.text));
  }
  void visit(const ast::newline& newline, tree_printer::scope scope) const {
    scope.println(
        " newline {} '{}'",
        location(newline.loc),
        tree_printer::escape(newline.text));
  }
  void visit(
      const ast::section_block& section, tree_printer::scope scope) const {
    scope.println(
        " section-block{} {}",
        section.inverted ? " <inverted>" : "",
        location(section.loc));
    visit(section.variable, scope.open_property());
    for (const auto& body : section.bodies) {
      visit(body, scope.open_node());
    }
  }
  void visit(const ast::partial_apply& partial_apply, tree_printer::scope scope)
      const {
    scope.println(
        " partial-apply{} {} '{}'",
        partial_apply.is_standalone ? " <standalone>" : "",
        location(partial_apply.loc),
        partial_apply.path.as_string('/'));
  }
  void visit(const ast::comment& comment, tree_printer::scope scope) const {
    scope.println(
        " comment {} '{}'",
        location(comment.loc),
        tree_printer::escape(comment.text));
  }
  void visit(
      const ast::variable_lookup& variable, tree_printer::scope scope) const {
    scope.println(
        " variable-lookup {} '{}'",
        location(variable.loc),
        variable.path_string());
  }
  void visit(const ast::variable& variable, tree_printer::scope scope) const {
    scope.println(
        " variable {} '{}'", location(variable.loc), variable.path_string());
  }
  void visit(const ast::body& body, tree_printer::scope scope) const {
    // This node is transparent so it does not directly appear in the tree
    detail::variant_match(
        body, [&](const auto& node) { visit(node, std::move(scope)); });
  }
  void visit(const ast::bodies& bodies, tree_printer::scope scope) const {
    // This node is transparent so it does not directly appear in the tree
    for (const auto& body : bodies) {
      visit(body, scope);
    }
  }
  void visit(const ast::root& root, tree_printer::scope scope) const {
    scope.println(
        "root [{}]", resolved_location(root.loc, src_manager).file_name());
    auto root_scope = scope.open_node();
    visit(root.bodies, root_scope);
  }

  const source_manager& src_manager;
};

} // namespace

template <class Node>
void print_ast(
    const Node& root_node,
    const source_manager& src_manager,
    std::ostream& out) {
  ast_visitor{src_manager}.visit(
      root_node, tree_printer::scope::make_root(out));
}

#define WHISKER_PRINT_AST_INSTANTIATE(node_type) \
  template void print_ast(                       \
      const node_type&, const source_manager&, std::ostream&)

WHISKER_PRINT_AST_INSTANTIATE(ast::root);
WHISKER_PRINT_AST_INSTANTIATE(ast::text);
WHISKER_PRINT_AST_INSTANTIATE(ast::comment);
WHISKER_PRINT_AST_INSTANTIATE(ast::identifier);
WHISKER_PRINT_AST_INSTANTIATE(ast::variable_lookup);
WHISKER_PRINT_AST_INSTANTIATE(ast::section_block);
WHISKER_PRINT_AST_INSTANTIATE(ast::partial_apply);

#undef WHISKER_PRINT_AST_INSTANTIATE

} // namespace whisker
