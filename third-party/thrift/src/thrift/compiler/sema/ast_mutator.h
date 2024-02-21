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

#include <type_traits>
#include <vector>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/sema/diagnostic_context.h>

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
    : public basic_ast_visitor<false, diagnostic_context&, mutator_context&> {
  using base = basic_ast_visitor<false, diagnostic_context&, mutator_context&>;

 public:
  using base::base;

  void mutate(diagnostic_context& ctx, t_program_bundle& bundle) {
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
  bool operator()(diagnostic_context& ctx, t_program_bundle& bundle) {
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
        [&](diagnostic_context&, mutator_context&, t_field& node) {
          node.set_type(resolve(node.type()));

          if (auto* dflt = node.get_default_value()) {
            resolve_const_value(*dflt, resolve_const_value);
          }
        });

    mutator.add_typedef_visitor(
        [&](diagnostic_context&, mutator_context&, t_typedef& node) {
          node.set_type(resolve(node.type()));
        });

    mutator.add_function_visitor(
        [&](diagnostic_context& ctx, mutator_context& mCtx, t_function& node) {
          resolve_in_place(node.return_type());
          resolve_in_place(node.interaction());
          for (auto& field : node.params().fields()) {
            mutator(ctx, mCtx, field);
          }
        });
    mutator.add_throws_visitor(
        [&](diagnostic_context& ctx, mutator_context& mCtx, t_throws& node) {
          for (auto& field : node.fields()) {
            mutator(ctx, mCtx, field);
          }
        });
    mutator.add_stream_visitor(
        [&](diagnostic_context&, mutator_context&, t_stream& node) {
          resolve_in_place(node.elem_type());
        });
    mutator.add_sink_visitor(
        [&](diagnostic_context&, mutator_context&, t_sink& node) {
          resolve_in_place(node.elem_type());
          resolve_in_place(node.final_response_type());
        });

    mutator.add_map_visitor(
        [&](diagnostic_context&, mutator_context&, t_map& node) {
          resolve_in_place(node.key_type());
          resolve_in_place(node.val_type());
        });
    mutator.add_set_visitor(
        [&](diagnostic_context&, mutator_context&, t_set& node) {
          resolve_in_place(node.elem_type());
        });
    mutator.add_list_visitor(
        [&](diagnostic_context&, mutator_context&, t_list& node) {
          resolve_in_place(node.elem_type());
        });

    mutator.add_const_visitor(
        [&](diagnostic_context&, mutator_context&, t_const& node) {
          resolve_in_place(node.type_ref());
          resolve_const_value(*node.value(), resolve_const_value);
        });

    mutator.mutate(ctx, bundle);
    return !unresolved_;
  }

 private:
  bool unresolved_{false};
};

// A thin wrapper around a vector of mutators that
// knows how to apply those mutators in order.
struct ast_mutators {
 private:
  struct mutation_result {
    bool unresolvable_typeref = false;
  };

  bool use_legacy_type_ref_resolution_;

 public:
  explicit ast_mutators(bool use_legacy_type_ref_resolution)
      : use_legacy_type_ref_resolution_(use_legacy_type_ref_resolution) {}

  std::vector<ast_mutator> stages;

  // Access a specific mutator stage, growing the number of stages if needed.
  template <typename T>
  ast_mutator& operator[](T&& stage) {
    static_assert(std::is_enum<T>::value, "");
    auto index = static_cast<size_t>(stage);
    if (stages.size() <= index) {
      stages.resize(index + 1);
    }
    return stages[index];
  }

  mutation_result operator()(
      diagnostic_context& ctx, t_program_bundle& bundle) {
    for (auto& stage : stages) {
      stage.mutate(ctx, bundle);
    }
    // We have no more mutators, so all type references **must** resolve.
    mutation_result ret;
    ret.unresolvable_typeref = !resolve_all_types(ctx, bundle);
    return ret;
  }

 private:
  // Tries to resolve any unresolved type references, returning true if
  // successful.
  //
  // Any errors are reported via diags.
  bool resolve_all_types(diagnostic_context& diags, t_program_bundle& bundle) {
    bool success = true;
    if (!use_legacy_type_ref_resolution_) {
      success = type_ref_resolver{}(diags, bundle);
    }
    for (auto& td : bundle.root_program()->scope()->placeholder_typedefs()) {
      if (!td.type()) {
        if (use_legacy_type_ref_resolution_) {
          if (td.resolve()) {
            continue;
          }
          success = false;
        }

        diags.error(td, "Type `{}` not defined.", td.name());
        assert(!td.resolve());
        assert(!success);
        success = false;
      }
    }
    return success;
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
