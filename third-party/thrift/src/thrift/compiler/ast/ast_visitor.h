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

#include <array>
#include <cassert>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_union.h>

namespace apache::thrift::compiler {
namespace ast_detail {

// Visitation and registration functions for concrete AST nodes.
#define FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(name)               \
 private:                                                           \
  using name##_type = node_type<t_##name>;                          \
  ast_detail::visitor_list<Args..., name##_type&> name##_visitors_; \
                                                                    \
 public:                                                            \
  void add_##name##_visitor(                                        \
      std::function<void(Args..., name##_type&)> visitor) {         \
    name##_visitors_.emplace_back(std::move(visitor));              \
  }                                                                 \
  void operator()(Args... args, name##_type& node) const

// The type to use when traversing the given node type N.
template <bool is_const, typename N>
using node_type = std::conditional_t<is_const, const N, N>;

// Helper that to propagate constness through a dynamic_cast.
template <typename N>
N* as(t_node* node) {
  return dynamic_cast<N*>(node);
}
template <typename N>
const N* as(const t_node* node) {
  return dynamic_cast<const N*>(node);
}

// Checks if the context type Context has begin/end_visit methods.
template <typename Context, typename = void>
struct has_visit_methods : std::false_type {};
template <typename Context>
struct has_visit_methods<
    Context,
    std::void_t<
        decltype(std::declval<Context>().begin_visit(std::declval<t_node&>())),
        decltype(std::declval<Context>().end_visit(std::declval<t_node&>()))>>
    : std::true_type {};

// Helper to call begin/end_visit if supported by the visitation context C.
template <typename Node, typename C>
void begin_visit(Node& node, C& context) {
  if constexpr (has_visit_methods<C>::value) {
    context.begin_visit(node);
  }
}
template <typename Node, typename C>
void end_visit(Node& node, C& context) {
  if constexpr (has_visit_methods<C>::value) {
    context.end_visit(node);
  }
}

// A list of visitor that accept the given arguments.
template <typename... Args>
using visitor_list = std::vector<std::function<void(Args...)>>;

} // namespace ast_detail

/**
 * A class that can traverse Thrift IDL AST nodes, invoking registered visitors
 * for each node visited.
 *
 * Visits AST nodes in 'preorder', visiting the parent node before children
 * nodes.
 *
 * For each concrete node type, provides the following functions:
 * - an operator() overload for visiting the node:
 *     void operator()(args..., t_{name}&) const;
 * - a function to add a node-specific visitor:
 *     void add_{name}_visitor(std::function<void(args..., t_{name}&)>);
 *
 * The operator() overloads provide the dispatching logic typically associated
 * with the `accept(...)` methods in the traditional Visitor pattern - except
 * that the visitors are not passed as an argument, but previously set via the
 * `add_*_visitor(...)` functions.
 *
 * Also provides helper functions for registering a visitor for multiple node
 * types. For example: all interface, structured_declaration, and
 * declaration visitors.
 */
template <bool is_const, typename... Args>
class basic_ast_visitor {
  template <typename N>
  using node_type = ast_detail::node_type<is_const, N>;

  // Enables operator() to distinguish structured fields from function params
  // and exceptions, which also use t_field but don't belong in field_visitor.
  struct function_param {
    node_type<t_field>& wrapped;
  };
  struct thrown_exception {
    node_type<t_field>& wrapped;
  };

 public:
  /**
   * Adds visitor for all interface node types.
   *
   * For example, visits: t_service and t_interaction.
   *
   * @param V `std::function<void(Args..., [const] NodeType&)>` where
   *        NodeType is any ancestor type of t_service (included), i.e.:
   *        t_service, t_interface, t_type, t_named, t_node
   */
  template <typename V>
  void add_interface_visitor(V&& visitor) {
    add_service_visitor(visitor);
    add_interaction_visitor(std::forward<V>(visitor));
  }

  /**
   * Adds a visitor for all structured IDL definition node types.
   *
   * For example, visits: t_struct, t_union, and t_exception.
   * Does not include other t_structured nodes like t_paramlist.
   *
   * @param V `std::function<void(Args..., [const] NodeType&)>` where
   *        NodeType is any ancestor type of t_structured (included), i.e.:
   *        t_structured, t_type, t_named, t_node
   */
  template <typename V>
  void add_structured_definition_visitor(V&& visitor) {
    add_struct_visitor(visitor);
    add_union_visitor(visitor);
    add_exception_visitor(std::forward<V>(visitor));
  }

  /**
   * Adds a visitor for root IDL definition node types.
   *
   * @param V `std::function<void(Args..., [const] NodeType&)>` where
   *        NodeType is any ancestor type of t_named (included), i.e.:
   *        t_named, t_node
   */
  template <typename V>
  void add_root_definition_visitor(V&& visitor) {
    add_interface_visitor(visitor);
    add_structured_definition_visitor(visitor);
    add_enum_visitor(visitor);
    add_const_visitor(visitor);
    add_typedef_visitor(std::forward<V>(visitor));
  }

  /**
   * Adds a visitor for all IDL definition node types.
   *
   * This includes the root IDL definition nodes, as well as functions, fields,
   * function parameters, thrown exceptions and enum values.
   *
   * @param V `std::function<void(Args..., [const] NodeType&)>` where
   *        NodeType is any ancestor type of t_named (included), i.e.:
   *        t_named, t_node
   */
  template <typename V>
  void add_definition_visitor(V&& visitor) {
    add_root_definition_visitor(visitor);
    add_function_visitor(visitor);
    add_field_visitor(visitor);
    add_function_param_visitor(visitor);
    add_thrown_exception_visitor(visitor);
    add_enum_value_visitor(std::forward<V>(visitor));
  }

  /**
   * Adds a visitor for all IDL named node types.
   *
   * @param V `std::function<void(Args..., [const] NodeType&)>` where
   *        NodeType is any ancestor type of t_named (included), i.e.:
   *        t_named, t_node
   */
  template <typename V>
  void add_named_visitor(V&& visitor) {
    add_definition_visitor(visitor);
    add_program_visitor(std::forward<V>(visitor));
  }

  /**
   * Adds a visitor for all container IDL node types.
   *
   * @param V `std::function<void(Args..., [const] NodeType&)>` where
   *        NodeType is any ancestor type of t_container (included), i.e.:
   *        t_container, t_type, t_named, t_node
   */
  template <typename V>
  void add_container_visitor(V&& visitor) {
    add_set_visitor(visitor);
    add_list_visitor(visitor);
    add_map_visitor(std::forward<V>(visitor));
  }

  /** Accepts node: [const] t_program& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(program) {
    begin_visit_and_call_visitors(program_visitors_, node, args...);
    visit_children_ptrs(node.services(), args...); // accept: t_service
    visit_children_ptrs(node.interactions(), args...); // accept: t_interation

    // Loop must be resilient to visitor calling push_back.
    for (size_t i = 0; i < node.structs_and_unions().size(); ++i) {
      t_structured* struct_or_union = node.structs_and_unions()[i];
      if (auto* tunion = ast_detail::as<t_union>(struct_or_union)) {
        (*this)(args..., *tunion); // accept: t_union
        continue;
      }

      // If node is not a union, then it must be a struct.
      auto* tstruct = ast_detail::as<t_struct>(struct_or_union);
      assert(tstruct != nullptr);
      (*this)(args..., *tstruct); // accept: t_struct
    }

    visit_children_ptrs(node.exceptions(), args...); // accept: t_exception
    visit_children_ptrs(node.typedefs(), args...); // accept: t_typedef
    visit_children_ptrs(node.enums(), args...); // accept: t_enum
    visit_children_ptrs(node.consts(), args...); // accept: t_const

    // Loop must be resilient to visitor calling push_back.
    for (size_t i = 0; i < node.type_instantiations().size(); ++i) {
      auto& type_inst = node.type_instantiations()[i];
      if (auto* set_node = ast_detail::as<t_set>(&type_inst)) {
        visit_child(*set_node, args...); // accept: t_set
      } else if (auto* list_node = ast_detail::as<t_list>(&type_inst)) {
        visit_child(*list_node, args...); // accept: t_list
      } else if (auto* map_node = ast_detail::as<t_map>(&type_inst)) {
        visit_child(*map_node, args...); // accept: t_map
      } else {
        std::terminate(); // Should be unreachable.
      }
    }
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_service& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(service) {
    assert(typeid(node) == typeid(service_type)); // Must actually be a service.
    begin_visit_and_call_visitors(service_visitors_, node, args...);

    visit_children(node.functions(), args...); // accept: t_function
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_interaction& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(interaction) {
    begin_visit_and_call_visitors(interaction_visitors_, node, args...);

    visit_children(node.functions(), args...); // accept: t_function
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_function& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(function) {
    begin_visit_and_call_visitors(function_visitors_, node, args...);

    auto* sink_or_stream = node.sink_or_stream();
    if (sink_or_stream == nullptr) {
      // Do nothing.
    } else if (auto* sink = ast_detail::as<t_sink>(sink_or_stream)) {
      visit_child(*sink, args...); // accept: t_sink
    } else if (auto* stream = ast_detail::as<t_stream>(sink_or_stream)) {
      visit_child(*stream, args...); // accept: t_stream
    }
    visit_children_wrapped<function_param>(
        node.params().fields(), args...); // accept: function_param
    if (auto throws = node.exceptions()) {
      visit_children_wrapped<thrown_exception>(
          throws->fields(), args...); // accept: thrown_exception
    }
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_sink& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(sink) {
    begin_visit_and_call_visitors(sink_visitors_, node, args...);

    if (auto sink_exceptions = node.sink_exceptions()) {
      visit_children_wrapped<thrown_exception>(
          sink_exceptions->fields(), args...); // accept: thrown_exception
    }
    if (auto final_response_exceptions = node.final_response_exceptions()) {
      visit_children_wrapped<thrown_exception>(
          final_response_exceptions->fields(),
          args...); // accept: thrown_exception
    }
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_stream& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(stream) {
    begin_visit_and_call_visitors(stream_visitors_, node, args...);

    if (auto exceptions = node.exceptions()) {
      visit_children_wrapped<thrown_exception>(
          exceptions->fields(), args...); // accept: thrown_exception
    }
    end_visit(node, args...);
  }

  // Types

  /** Accepts node: [const] t_struct&  */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(struct) {
    assert(typeid(node) == typeid(t_struct));
    begin_visit_and_call_visitors(struct_visitors_, node, args...);
    visit_children(node.fields(), args...); // accept: t_field
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_union& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(union) {
    begin_visit_and_call_visitors(union_visitors_, node, args...);
    visit_children(node.fields(), args...); // accept: t_field
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_exception& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(exception) {
    begin_visit_and_call_visitors(exception_visitors_, node, args...);
    visit_children(node.fields(), args...); // accept: t_field
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_field& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(field) {
    begin_visit_and_call_visitors(field_visitors_, node, args...);
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_enum& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(enum) {
    begin_visit_and_call_visitors(enum_visitors_, node, args...);
    visit_children(node.values(), args...); // accept: t_enum_value
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_enum_value */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(enum_value) {
    begin_visit_and_call_visitors(enum_value_visitors_, node, args...);
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_const& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(const) {
    begin_visit_and_call_visitors(const_visitors_, node, args...);
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_typedef */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(typedef) {
    begin_visit_and_call_visitors(typedef_visitors_, node, args...);
    end_visit(node, args...);
  }

  // Function parameters and exceptions.
 private:
  ast_detail::visitor_list<Args..., field_type&> function_param_visitors_;
  ast_detail::visitor_list<Args..., field_type&> thrown_exception_visitors_;

 public:
  void add_function_param_visitor(std::function<void(Args..., field_type&)> v) {
    function_param_visitors_.emplace_back(std::move(v));
  }
  void add_thrown_exception_visitor(
      std::function<void(Args..., field_type&)> v) {
    thrown_exception_visitors_.emplace_back(std::move(v));
  }

  /** Accepts node: function_param (wraps a t_field) */
  void operator()(Args... args, function_param&& param) const {
    begin_visit_and_call_visitors(
        function_param_visitors_, param.wrapped, args...);
    end_visit(param.wrapped, args...);
  }

  /** Accepts node: thrown_exception (wraps a t_field) */
  void operator()(Args... args, thrown_exception&& exn) const {
    begin_visit_and_call_visitors(
        thrown_exception_visitors_, exn.wrapped, args...);
    end_visit(exn.wrapped, args...);
  }

  // Container types.

  /** Accepts node: [const] t_set& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(set) {
    begin_visit_and_call_visitors(set_visitors_, node, args...);
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_list& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(list) {
    begin_visit_and_call_visitors(list_visitors_, node, args...);
    end_visit(node, args...);
  }

  /** Accepts node: [const] t_map& */
  FBTHRIFT_AST_DETAIL_AST_VISITOR_NODE_T_(map) {
    begin_visit_and_call_visitors(map_visitors_, node, args...);
    end_visit(node, args...);
  }

 private:
  /**
   * Calls `begin_visit` on every `arg` (if applicable), and then calls every
   * visitor (in the given order), passing the given args and node.
   */
  template <typename TNode>
  static void begin_visit_and_call_visitors(
      const ast_detail::visitor_list<Args..., TNode&>& visitors,
      TNode& node,
      Args... args) {
    (ast_detail::begin_visit(node, args), ...);

    for (auto&& visitor : visitors) {
      visitor(args..., node);
    }
  }

  /**
   * Ends the visitation of the given `node` by calling `end_visit` on every
   * given `arg` (when available), in reverse order.
   */
  template <typename TNode>
  static void end_visit(TNode& node, Args... args) {
    constexpr std::size_t NumArgs = sizeof...(Args);

    if constexpr (NumArgs == 0) {
      return;
    }

    // In order to call end_visit in the reverse order, we need to create an
    // array with the given `args`.
    //
    // make_func is a functor that takes an argument (from args), and returns
    // another functor which calls `end_visit` on the given `node`, with the
    // given arg
    auto make_func = [&](auto& arg) {
      return std::function<void()>([&] { ast_detail::end_visit(node, arg); });
    };
    std::array<std::function<void()>, NumArgs> funcs = {make_func(args)...};

    // Execute end_visit calls in reverse order
    for (size_t i = NumArgs; i > 0; --i) {
      funcs[i - 1]();
    }
  }

  /**
   * "Accepts" the given `child` for visitation.
   *
   * Here, "accepts" is meant in the traditional Visitor pattern sense: the
   * `operator()` overload in this class (`basic_ast_visitor`) that accepts the
   * target type `C` will be called to drive the visitation of `child` (by
   * calling the appropriate visitors and asking futher child nodes to accept
   * visitation, as relevant.
   *
   * The given `args` are passed to both the accept (`operator()`) and
   * visitation methods.
   */
  template <typename C>
  void visit_child(C& child, Args... args) const {
    (*this)(args..., child);
  }

  /**
   * Accepts all the children in the given collection for visitation.
   *
   * @see `visit_child(...)`
   */
  template <typename C>
  void visit_children(const C& children, Args... args) const {
    // Loop must be resilient to visitor calling push_back.
    for (size_t i = 0; i < children.size(); ++i) {
      (*this)(args..., children[i]);
    }
  }

  /**
   * Accepts all the children in the given collection for visitation, wrapping
   * each one in the specified `Wrapper` type (to disambiguate between different
   * semantics for identical node types).
   */
  template <typename Wrapper, typename C>
  void visit_children_wrapped(const C& children, Args... args) const {
    // Loop must be resilient to visitor calling push_back.
    for (size_t i = 0; i < children.size(); ++i) {
      (*this)(args..., Wrapper{children[i]});
    }
  }

  /**
   * Same as `visit_children(...)`, but the given collection contains pointers.
   */
  template <typename C>
  void visit_children_ptrs(const C& children, Args... args) const {
    // Loop must be resilient to visitor calling push_back.
    for (size_t i = 0; i < children.size(); ++i) {
      (*this)(args..., *children[i]);
    }
  }
};

template <bool is_const>
class basic_visitor_context {
  using node_type = ast_detail::node_type<is_const, t_node>;
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

/**
 * Thrift visitor over mutable (non-const) AST.
 *
 * @see basic_ast_visitor
 */
using ast_visitor = basic_ast_visitor<false>;

/**
 * Same as ast_visitor, except traverse a const AST.
 */
using const_ast_visitor = basic_ast_visitor<true>;

using visitor_context = basic_visitor_context<false>;
using const_visitor_context = basic_visitor_context<true>;

} // namespace apache::thrift::compiler
