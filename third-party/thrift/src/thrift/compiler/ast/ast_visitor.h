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

#include <cassert>
#include <exception>
#include <functional>
#include <type_traits>
#include <vector>

#include <thrift/compiler/ast/detail/ast_visitor.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_interface.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>

namespace apache {
namespace thrift {
namespace compiler {

// A list of visitor that accept the given arguments.
template <typename... Args>
using visitor_list = std::vector<std::function<void(Args...)>>;

template <bool is_const, typename... Args>
class basic_ast_visitor;

// A class that can traverse ast nodes, invoking registered visitors for each
// node visited.
//
// Visits AST nodes in 'preorder', visiting the parent node before children
// nodes.
//
// For each concrete node type, provides the following functions:
// - an operator() overload for visiting the node:
//     void operator()(args..., t_{name}&) const;
// - a function to add a node-specific visitor:
//     void add_{name}_visitor(std::function<void(args..., t_{name}&)>);
//
// Also provides helper functions for registering a visitor for multiple node
// types. For example: all interface, structured_declaration, and
// declaration visitors.
using ast_visitor = basic_ast_visitor<false>;

// Same as ast_visitor, except traverse a const AST.
using const_ast_visitor = basic_ast_visitor<true>;

// A class that can traverse an AST, calling registered visitors.
// See ast_visitor.
template <bool is_const, typename... Args>
class basic_ast_visitor {
  template <typename N>
  using node_type = ast_detail::node_type<is_const, N>;

 public:
  // Adds visitor for all interface node types.
  //
  // For example: t_service and t_interaction.
  template <typename V>
  void add_interface_visitor(V&& visitor) {
    add_service_visitor(visitor);
    add_interaction_visitor(std::forward<V>(visitor));
  }

  // Adds a visitor for all structured IDL definition node types.
  //
  // For example: t_struct, t_union, and t_exception.
  // Does not include other t_structured nodes like t_paramlist.
  template <typename V>
  void add_structured_definition_visitor(V&& visitor) {
    add_struct_visitor(visitor);
    add_union_visitor(visitor);
    add_exception_visitor(std::forward<V>(visitor));
  }

  // Adds a visitor for root IDL definition node types.
  template <typename V>
  void add_root_definition_visitor(V&& visitor) {
    add_interface_visitor(visitor);
    add_structured_definition_visitor(visitor);
    add_enum_visitor(visitor);
    add_const_visitor(visitor);
    add_typedef_visitor(std::forward<V>(visitor));
  }

  // Adds a visitor for all IDL definition node types.
  template <typename V>
  void add_definition_visitor(V&& visitor) {
    add_root_definition_visitor(visitor);
    add_function_visitor(visitor);
    add_field_visitor(visitor);
    add_enum_value_visitor(visitor);
    add_program_visitor(std::forward<V>(visitor));
  }

  template <typename V>
  void add_container_visitor(V&& visitor) {
    add_set_visitor(visitor);
    add_list_visitor(visitor);
    add_map_visitor(std::forward<V>(visitor));
  }

  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(program) {
    begin_visit(program_visitors_, node, args...);
    visit_children_ptrs(node.services(), args...);
    visit_children_ptrs(node.interactions(), args...);
    // TODO(afuller): Split structs and unions in t_program accessors.
    // Note: Loop must be resilient to visitor causing push_back calls.
    for (size_t i = 0; i < node.structs().size(); ++i) {
      auto* struct_or_union = node.structs()[i];
      if (auto* tunion = ast_detail::as<t_union>(struct_or_union)) {
        this->operator()(args..., *tunion);
      } else {
        this->operator()(args..., *struct_or_union);
      }
    }
    visit_children_ptrs(node.exceptions(), args...);
    visit_children_ptrs(node.typedefs(), args...);
    visit_children_ptrs(node.enums(), args...);
    visit_children_ptrs(node.consts(), args...);
    // Note: Loop must be resilient to visitor causing push_back calls.
    for (size_t i = 0; i < node.type_instantiations().size(); ++i) {
      auto& type_inst = node.type_instantiations()[i];
      if (auto* set_node = ast_detail::as<t_set>(&type_inst)) {
        visit_child(*set_node, args...);
      } else if (auto* list_node = ast_detail::as<t_list>(&type_inst)) {
        visit_child(*list_node, args...);
      } else if (auto* map_node = ast_detail::as<t_map>(&type_inst)) {
        visit_child(*map_node, args...);
      } else {
        std::terminate(); // Should be unreachable.
      }
    }
    end_visit(node, args...);
  }

  // Interfaces
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(service) {
    assert(typeid(node) == typeid(service_type)); // Must actually be a service.
    begin_visit(service_visitors_, node, args...);
    visit_children(node.functions(), args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(interaction) {
    begin_visit(interaction_visitors_, node, args...);
    visit_children(node.functions(), args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(function) {
    begin_visit(function_visitors_, node, args...);
    auto* sink_or_stream = node.sink_or_stream();
    if (!sink_or_stream) {
      // Do nothing.
    } else if (auto* sink = ast_detail::as<t_sink>(sink_or_stream)) {
      visit_child(*sink, args...);
    } else if (
        auto* stream = ast_detail::as<t_stream_response>(sink_or_stream)) {
      visit_child(*stream, args...);
    }
    if (node.exceptions()) {
      visit_child(*node.exceptions(), args...);
    }
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(sink) {
    begin_visit(sink_visitors_, node, args...);
    if (auto throws = node.sink_exceptions()) {
      visit_child(*throws, args...);
    }
    if (auto throws = node.final_response_exceptions()) {
      visit_child(*throws, args...);
    }
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(stream_response) {
    begin_visit(stream_response_visitors_, node, args...);
    if (node.exceptions()) {
      visit_child(*node.exceptions(), args...);
    }
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(throws) {
    begin_visit(throws_visitors_, node, args...);
    end_visit(node, args...);
  }

  // Types
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(struct) {
    assert(typeid(node) == typeid(struct_type)); // Must actually be a struct.
    begin_visit(struct_visitors_, node, args...);
    visit_children(node.fields(), args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(union) {
    begin_visit(union_visitors_, node, args...);
    visit_children(node.fields(), args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(exception) {
    begin_visit(exception_visitors_, node, args...);
    visit_children(node.fields(), args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(field) {
    begin_visit(field_visitors_, node, args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(enum) {
    begin_visit(enum_visitors_, node, args...);
    visit_children(node.values(), args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(enum_value) {
    begin_visit(enum_value_visitors_, node, args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(const) {
    begin_visit(const_visitors_, node, args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(typedef) {
    begin_visit(typedef_visitors_, node, args...);
    end_visit(node, args...);
  }

  // Container types.
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(set) {
    begin_visit(set_visitors_, node, args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(list) {
    begin_visit(list_visitors_, node, args...);
    end_visit(node, args...);
  }
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(map) {
    begin_visit(map_visitors_, node, args...);
    end_visit(node, args...);
  }

 private:
  template <typename N>
  static void begin_visit(
      const visitor_list<Args..., N&>& visitors, N& node, Args... args) {
    // TODO(afuller): Replace with c++17 folding syntax when available.
    using _ = int[];
    void(_{0, (ast_detail::begin_visit(node, args), 0)...});

    for (auto&& visitor : visitors) {
      visitor(args..., node);
    }
  }

  template <typename N>
  static void end_visit(N& node, Args... args) {
    auto make_func = [&](auto& arg) {
      return std::function<void()>([&] { ast_detail::end_visit(node, arg); });
    };
    (void)make_func; // Suppress the dead store warning for zero args.
    constexpr size_t num_args = sizeof...(args);
    std::function<void()> funcs[num_args > 0 ? num_args : 1] = {
        make_func(args)...};
    for (size_t i = num_args; i > 0; --i) {
      funcs[i - 1]();
    }
  }

  template <typename C>
  void visit_child(C& child, Args... args) const {
    operator()(args..., child);
  }

  template <typename C>
  void visit_children(const C& children, Args... args) const {
    // Note: Loop must be resilient to visitor causing push_back calls.
    for (size_t i = 0; i < children.size(); ++i) {
      operator()(args..., children[i]);
    }
  }
  template <typename C>
  void visit_children_ptrs(const C& children, Args... args) const {
    // Note: Loop must be resilient to visitor causing push_back calls.
    for (size_t i = 0; i < children.size(); ++i) {
      operator()(args..., *children[i]);
    }
  }
};

template <bool is_const, typename N = t_node>
class basic_visitor_context {
  using node_type = ast_detail::node_type<is_const, N>;
  using program_type = ast_detail::node_type<is_const, t_program>;

 public:
  bool visiting() const noexcept { return !context_.empty(); }

  // The first node visited.
  node_type* root() const noexcept {
    assert(!context_.empty());
    return context_.empty() ? nullptr : context_.front();
  }

  // The node currently being visited, or nullptr.
  node_type* current() const noexcept {
    assert(!context_.empty());
    return context_.empty() ? nullptr : context_.back();
  }

  // The parent of the current node, or nullptr.
  node_type* parent() const noexcept {
    return context_.size() < 2 ? nullptr : context_[context_.size() - 2];
  }

  program_type& program() const {
    if (program_type* program = dynamic_cast<program_type*>(root())) {
      return *program;
    }
    throw std::runtime_error("Could not resolve program.");
  }

  void begin_visit(node_type& node) { context_.emplace_back(&node); }
  void end_visit(node_type& node) {
    (void)(node);
    assert(&node == context_.back());
    context_.pop_back();
  }

  const std::vector<node_type*>& nodes() const noexcept { return context_; }

 private:
  std::vector<node_type*> context_;
};

using visitor_context = basic_visitor_context<false>;
using const_visitor_context = basic_visitor_context<true>;

} // namespace compiler
} // namespace thrift
} // namespace apache
