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

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/sema/sema_context.h>

namespace apache {
namespace thrift {
namespace compiler {

// Mutators have mutable access to the AST.
struct mutator_context : visitor_context {
  t_program_bundle* bundle;
};

// An AST mutator is a ast_visitor that collects diagnostics and can
// change the AST.
class ast_mutator
    : public basic_ast_visitor<false, sema_context&, mutator_context&> {
  using base = basic_ast_visitor<false, sema_context&, mutator_context&>;

 public:
  using base::base;

  void mutate(sema_context& ctx, t_program_bundle& bundle) {
    mutator_context mctx;
    mctx.bundle = &bundle;
    for (auto itr = bundle.programs().rbegin(); itr != bundle.programs().rend();
         ++itr) {
      operator()(ctx, mctx, *itr);
    }
  }
};

/// An AST mutator that replaces placeholder_typedefs with resolved types.
struct type_ref_resolver {
  void resolve_in_place(t_type_ref& ref) {
    unresolved_ = !ref.resolve() || unresolved_;
  }
  [[nodiscard]] t_type_ref resolve(t_type_ref ref) {
    resolve_in_place(ref);
    return ref;
  }
  bool run(sema_context& ctx, t_program_bundle& bundle) {
    ast_mutator mutator;

    auto resolve_const_value = [&](t_const_value& node, auto& recurse) -> void {
      node.set_ttype(resolve(node.ttype()));

      if (node.kind() == t_const_value::CV_MAP) {
        for (auto& map_val : node.get_map()) {
          recurse(*map_val.first, recurse);
          recurse(*map_val.second, recurse);
        }
      } else if (node.kind() == t_const_value::CV_LIST) {
        for (auto& list_val : node.get_list()) {
          recurse(*list_val, recurse);
        }
      }
    };

    mutator.add_field_visitor(
        [&](sema_context&, mutator_context&, t_field& node) {
          node.set_type(resolve(node.type()));

          if (auto* dflt = node.get_default_value()) {
            resolve_const_value(*dflt, resolve_const_value);
          }
        });

    mutator.add_typedef_visitor(
        [&](sema_context&, mutator_context&, t_typedef& node) {
          node.set_type(resolve(node.type()));
        });

    mutator.add_function_visitor(
        [&](sema_context& ctx, mutator_context& mctx, t_function& node) {
          resolve_in_place(node.return_type());
          resolve_in_place(node.interaction());
          for (auto& field : node.params().fields()) {
            mutator(ctx, mctx, field);
          }
        });
    mutator.add_throws_visitor(
        [&](sema_context& ctx, mutator_context& mctx, t_throws& node) {
          for (auto& field : node.fields()) {
            mutator(ctx, mctx, field);
          }
        });
    mutator.add_stream_visitor(
        [&](sema_context&, mutator_context&, t_stream& node) {
          resolve_in_place(node.elem_type());
        });
    mutator.add_sink_visitor(
        [&](sema_context&, mutator_context&, t_sink& node) {
          resolve_in_place(node.elem_type());
          resolve_in_place(node.final_response_type());
        });

    mutator.add_map_visitor([&](sema_context&, mutator_context&, t_map& node) {
      resolve_in_place(node.key_type());
      resolve_in_place(node.val_type());
    });
    mutator.add_set_visitor([&](sema_context&, mutator_context&, t_set& node) {
      resolve_in_place(node.elem_type());
    });
    mutator.add_list_visitor(
        [&](sema_context&, mutator_context&, t_list& node) {
          resolve_in_place(node.elem_type());
        });

    mutator.add_const_visitor(
        [&](sema_context&, mutator_context&, t_const& node) {
          resolve_in_place(node.type_ref());
          resolve_const_value(*node.value(), resolve_const_value);
        });

    mutator.mutate(ctx, bundle);
    return !unresolved_;
  }

 private:
  bool unresolved_{false};
};

} // namespace compiler
} // namespace thrift
} // namespace apache
