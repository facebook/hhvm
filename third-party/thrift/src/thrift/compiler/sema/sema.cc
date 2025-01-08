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

#include <thrift/compiler/sema/sema.h>

#include <algorithm>
#include <functional>
#include <type_traits>
#include <unordered_set>

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/detail/pluggable_functions.h>
#include <thrift/compiler/generate/cpp/util.h>
#include <thrift/compiler/sema/schematizer.h>
#include <thrift/compiler/sema/sema_context.h>
#include <thrift/compiler/sema/standard_validator.h>

namespace apache::thrift::compiler {
namespace {

// Mutators have mutable access to the AST.
struct mutator_context : visitor_context {
  t_program_bundle* bundle;
};

// An AST mutator is an ast_visitor that collects diagnostics and
// may change the AST.
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
      (*this)(ctx, mctx, *itr);
    }
  }
};

/// An AST mutator that replaces placeholder_typedefs with resolved types.
class type_ref_resolver {
 private:
  bool unresolved_ = false;

 public:
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
};

void match_type_with_const_value(
    sema_context& ctx,
    const t_program& program,
    const t_type* long_type,
    t_const_value* value) {
  const t_type* type = long_type->get_true_type();
  if (!type) {
    // type is unresolved, which will fail in validation.
    return;
  }

  switch (type->get_type_value()) {
    case t_type::type::t_list: {
      auto* elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
      if (value->kind() == t_const_value::CV_LIST) {
        for (auto list_val : value->get_list()) {
          match_type_with_const_value(ctx, program, elem_type, list_val);
        }
      }
      break;
    }
    case t_type::type::t_set: {
      auto* elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
      if (value->kind() == t_const_value::CV_LIST) {
        for (auto set_val : value->get_list()) {
          match_type_with_const_value(ctx, program, elem_type, set_val);
        }
      }
      break;
    }
    case t_type::type::t_map: {
      auto* key_type = dynamic_cast<const t_map*>(type)->get_key_type();
      auto* val_type = dynamic_cast<const t_map*>(type)->get_val_type();
      if (value->kind() == t_const_value::CV_MAP) {
        for (auto map_val : value->get_map()) {
          match_type_with_const_value(ctx, program, key_type, map_val.first);
          match_type_with_const_value(ctx, program, val_type, map_val.second);
        }
      }
      break;
    }
    case t_type::type::t_structured: {
      const auto* structured = dynamic_cast<const t_structured*>(type);
      if (auto ttype = value->ttype();
          ttype && ttype->get_true_type() != type) {
        ctx.error(
            value->ref_range().begin,
            "type mismatch: expected {}, got {}",
            type->get_full_name(),
            ttype->get_full_name());
      }
      if (value->kind() == t_const_value::CV_IDENTIFIER) {
        const std::string& id = value->get_identifier();
        const t_const* constant = program.scope()->find<t_const>(id);
        if (!constant) {
          ctx.error(
              value->ref_range().begin,
              "use of undeclared identifier '{}'",
              id);
          return;
        }
        value->assign(t_const_value(*constant->value()));
      }
      if (value->kind() == t_const_value::CV_MAP) {
        for (const auto& [map_key, map_val] : value->get_map()) {
          bool resolved = map_key->kind() != t_const_value::CV_IDENTIFIER;
          auto name =
              resolved ? map_key->get_string() : map_key->get_identifier();
          auto field = structured->get_field_by_name(name);
          if (!field) {
            // Error reported by const_checker.
            return;
          }
          if (!resolved) {
            map_key->convert_identifier_to_string();
          }
          match_type_with_const_value(ctx, program, field->get_type(), map_val);
        }
      }
      break;
    }
    case t_type::type::t_enum:
      // Set constant value types as enums when they are declared with integers
      // or identifiers.
      if (!value->is_enum()) {
        value->set_is_enum();
        auto enm = dynamic_cast<const t_enum*>(type);
        value->set_enum(enm);
        if (value->kind() == t_const_value::CV_INTEGER) {
          if (const auto* enum_value = enm->find_value(value->get_integer())) {
            value->set_enum_value(enum_value);
          }
        } else if (value->kind() == t_const_value::CV_IDENTIFIER) {
          // Resolve enum values defined after use.
          const std::string& id = value->get_identifier();
          const t_const* constant = program.scope()->find<t_const>(id);
          if (!constant) {
            constant =
                program.scope()->find<t_const>(value->program().scope_name(id));
          }
          if (!constant) {
            // Try to resolve enum values from typedefs.
            auto last_dot_pos = id.find_last_of('.');
            std::string enum_name = id.substr(0, last_dot_pos);
            if (std::count(enum_name.begin(), enum_name.end(), '.') == 0) {
              enum_name = value->program().scope_name(enum_name);
            }
            if (auto* def = dynamic_cast<const t_type*>(
                    program.scope()->find(enum_name))) {
              if (auto* enum_def =
                      dynamic_cast<const t_enum*>(def->get_true_type())) {
                constant =
                    enum_def->find_const_by_name(id.substr(last_dot_pos + 1));
              }
            }
          }
          if (!constant) {
            ctx.error(
                value->ref_range().begin,
                "use of undeclared identifier '{}'",
                id);
            return;
          }
          value->assign(t_const_value(*constant->value()));
        }
      }
      break;
    case t_type::type::t_bool:
    case t_type::type::t_byte:
    case t_type::type::t_i16:
    case t_type::type::t_i32:
    case t_type::type::t_i64:
    case t_type::type::t_float:
    case t_type::type::t_double:
    case t_type::type::t_string:
    case t_type::type::t_binary:
      // Remove enum_value if type is a base_type to use the integer instead.
      if (value->is_enum()) {
        value->set_enum_value(nullptr);
      }
      break;
    case t_type::type::t_void:
    case t_type::type::t_service:
      assert(false);
  }

  value->set_ttype(t_type_ref::from_req_ptr(type));
}

void maybe_match_type_with_const_value(
    sema_context& ctx,
    mutator_context& mctx,
    const t_type* type,
    t_const_value* value) {
  if (type == nullptr || value == nullptr) {
    return;
  }

  match_type_with_const_value(ctx, mctx.program(), type, value);
}

void match_const_type_with_value(
    sema_context& ctx, mutator_context& mctx, t_const& const_node) {
  maybe_match_type_with_const_value(
      ctx, mctx, const_node.type(), const_node.value());
}

void match_field_type_with_default_value(
    sema_context& ctx, mutator_context& mctx, t_field& field_node) {
  maybe_match_type_with_const_value(
      ctx, mctx, field_node.get_type(), field_node.get_default_value());
}

static void match_annotation_types_with_const_values(
    sema_context& ctx, mutator_context& mctx, t_named& node) {
  for (t_const& tconst : node.structured_annotations()) {
    maybe_match_type_with_const_value(ctx, mctx, tconst.type(), tconst.value());
  }
}

// Only an unqualified field is eligible for terse write.
void mutate_terse_write_annotation_structured(
    sema_context& ctx, mutator_context&, t_structured& node) {
  bool program_has_terse_write =
      ctx.program().inherit_annotation_or_null(node, kTerseWriteUri);
  for (auto& field : node.fields()) {
    bool field_has_terse_write =
        field.find_structured_annotation_or_null(kTerseWriteUri);
    if (!field_has_terse_write && !program_has_terse_write) {
      continue;
    }
    if (field.qualifier() == t_field_qualifier::none) {
      field.set_qualifier(t_field_qualifier::terse);
    } else if (field_has_terse_write) {
      ctx.error(
          field, "`@thrift.TerseWrite` cannot be used with qualified fields");
    }
  }
}

void mutate_inject_metadata_fields(
    sema_context& ctx, mutator_context&, t_structured& node) {
  // TODO(dokwon): Currently field injection doesn't work for structs used as
  // transitive annotations. Skipping as a workaround.
  if (is_transitive_annotation(node)) {
    return;
  }

  const t_const* annotation =
      node.find_structured_annotation_or_null(kInjectMetadataFieldsUri);
  if (!annotation) {
    return;
  }

  std::string type_string;
  try {
    type_string =
        annotation->get_value_from_structured_annotation("type").get_string();
  } catch (const std::exception& e) {
    ctx.error("{}", e.what());
    return;
  }
  // If the specified type and annotation are from the same program, append
  // the current program name.
  if (type_string.find(".") == std::string::npos) {
    type_string = annotation->program()->name() + "." + type_string;
  }

  const auto* ttype = node.program()->scope()->find<t_type>(type_string);
  if (!ttype) {
    ctx.error(
        "Can not find expected type `{}` specified in "
        "`@internal.InjectMetadataFields` in the current scope. "
        "Please check the include.",
        type_string);
    return;
  }

  const auto* structured = dynamic_cast<const t_structured*>(ttype);
  // We only allow injecting fields from a struct type.
  if (structured == nullptr || ttype->is_union() || ttype->is_exception() ||
      ttype->is_paramlist()) {
    ctx.error(
        "`{}` is not a struct type. `@internal.InjectMetadataFields` can be "
        "only used with a struct type.",
        type_string);
    return;
  }
  for (const auto& field : structured->fields()) {
    t_field_id injected_id;
    try {
      injected_id = cpp2::get_internal_injected_field_id(field.id());
    } catch (const std::exception& e) {
      ctx.error("{}", e.what());
      // Iterate all fields to find more errors.
      continue;
    }
    std::unique_ptr<t_field> cloned_field = field.clone_DO_NOT_USE();
    cloned_field->set_injected_id(injected_id);
    ctx.check(
        node.try_append_field(cloned_field),
        "Field id `{}` is already used in `{}`.",
        field.id(),
        node.name());
  }
}

void lower_deprecated_annotations(
    sema_context& ctx, mutator_context&, t_named& node) {
  if (auto cnst = node.find_structured_annotation_or_null(
          kDeprecatedUnvalidatedAnnotationsUri)) {
    ctx.check(
        node.annotations().empty(),
        "Cannot combine @thrift.DeprecatedUnvalidatedAnnotations with legacy annotation syntax.");
    auto val = cnst->get_value_from_structured_annotation_or_null("items");
    if (!val || val->get_map().empty()) {
      ctx.error(
          "Must specify at least one item in @thrift.DeprecatedUnvalidatedAnnotations.");
      return;
    }
    deprecated_annotation_map map;
    for (auto& [k, v] : val->get_map()) {
      map[k->get_string()] = {{}, v->get_string()};
    }
    node.reset_annotations(std::move(map));
  }
}

void normalize_return_type(
    sema_context& ctx, mutator_context&, t_function& node) {
  if (!node.has_return_type()) {
    return;
  }
  auto& type = node.return_type();
  if (!type.resolve()) {
    ctx.error(node, "Failed to resolve return type of `{}`.", node.name());
    return;
  }

  // Check the (first) response type.
  const t_type* true_type = type->get_true_type();
  if (dynamic_cast<const t_service*>(true_type)) {
    ctx.error("Invalid first response type: {}", type->get_full_name());
  }
}

template <typename Node>
void lower_type_annotations(
    sema_context& ctx, mutator_context& mctx, Node& node) {
  std::map<std::string, std::string> unstructured;

  if (!ctx.sema_parameters().skip_lowering_type_annotations) {
    if (const t_const* annot =
            node.find_structured_annotation_or_null(kCppTypeUri)) {
      if (auto type =
              annot->get_value_from_structured_annotation_or_null("name")) {
        unstructured.insert({"cpp.type", type->get_string()});
      } else if (
          auto tmplate =
              annot->get_value_from_structured_annotation_or_null("template")) {
        unstructured.insert({"cpp.template", tmplate->get_string()});
      }
    }
  }

  if (unstructured.empty()) {
    return;
  }

  const t_type* node_type = node.get_type();
  if (node_type->is_container() ||
      (node_type->is_typedef() &&
       static_cast<const t_typedef*>(node_type)->typedef_kind() !=
           t_typedef::kind::defined) ||
      (node_type->is_primitive_type() && !node_type->annotations().empty())) {
    // This is a new type we can modify in place
    for (auto& pair : unstructured) {
      const_cast<t_type*>(node_type)->set_annotation(pair.first, pair.second);
    }
  } else if (node_type->is_primitive_type()) {
    // Copy type as we don't handle unnamed typedefs to base types :(
    auto& program = mctx.program();
    auto unnamed = std::make_unique<t_primitive_type>(
        *static_cast<const t_primitive_type*>(node_type));
    for (auto& pair : unstructured) {
      unnamed->set_annotation(pair.first, pair.second);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_type(std::move(unnamed));
  } else {
    // Wrap in an unnamed typedef :(
    auto& program = mctx.program();
    auto unnamed = t_typedef::make_unnamed(
        const_cast<t_program*>(node_type->get_program()),
        node_type->get_name(),
        t_type_ref::from_ptr(node_type));
    for (auto& pair : unstructured) {
      unnamed->set_annotation(pair.first, pair.second);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_typedef(std::move(unnamed));
  }
}

void inject_schema_const(sema_context& ctx, mutator_context&, t_program& prog) {
  if (prog.find_structured_annotation_or_null(kDisableSchemaConstUri)) {
    return;
  }

  detail::schematizer::options opts;
  opts.only_root_program_ = true;
  opts.use_hash = true;
  opts.double_writes = false;
  opts.include_docs = false;
  opts.include_source_ranges = false;

  std::string serialized =
      detail::pluggable_functions().call<detail::get_schema_tag>(
          opts, ctx.source_mgr(), prog);
  if (serialized.empty()) {
    throw std::runtime_error(
        "Enabling --inject-schema-const requires using thrift2ast");
  }

  auto cnst = std::make_unique<t_const>(
      &prog,
      t_primitive_type::t_binary(),
      detail::schematizer::name_schema(ctx.source_mgr(), prog),
      std::make_unique<t_const_value>(std::move(serialized)));
  // Since schema injection happens after 'match_const_type_with_value' mutator
  // we need to set the explicitly type to binary here.
  cnst->value()->set_ttype(t_primitive_type::t_binary());
  cnst->set_uri("");
  cnst->set_src_range(prog.src_range());
  cnst->set_generated();
  prog.add_definition(std::move(cnst));
}

void deduplicate_thrift_includes(
    sema_context& ctx, mutator_context&, t_program& prog) {
  std::unordered_set<const t_program*> seen;
  auto& includes = prog.includes();
  auto it = std::remove_if(
      includes.begin(), includes.end(), [&](const auto* include) {
        if (!seen.insert(include->get_program()).second) {
          // TODO (T199931070): make this an error
          ctx.warning(
              *include,
              "Duplicate include of `{}`",
              include->get_program()->path());
          return true;
        }
        return false;
      });
  includes.erase(it, includes.end());
}

std::vector<ast_mutator> standard_mutators() {
  std::vector<ast_mutator> mutators;

  ast_mutator initial;
  initial.add_field_visitor(&lower_type_annotations<t_field>);
  initial.add_typedef_visitor(&lower_type_annotations<t_typedef>);
  initial.add_function_visitor(&normalize_return_type);
  initial.add_named_visitor(&lower_deprecated_annotations);
  mutators.push_back(std::move(initial));

  ast_mutator main;
  main.add_struct_visitor(&mutate_terse_write_annotation_structured);
  main.add_exception_visitor(&mutate_terse_write_annotation_structured);
  main.add_struct_visitor(&mutate_inject_metadata_fields);
  main.add_const_visitor(&match_const_type_with_value);
  main.add_field_visitor(&match_field_type_with_default_value);
  main.add_named_visitor(&match_annotation_types_with_const_values);
  main.add_program_visitor(&deduplicate_thrift_includes);
  mutators.push_back(std::move(main));

  return mutators;
}

} // namespace

bool sema::resolve_all_types(sema_context& diags, t_program_bundle& bundle) {
  bool success = true;
  if (!use_legacy_type_ref_resolution_) {
    success = type_ref_resolver().run(diags, bundle);
  }
  for (auto& td : bundle.root_program()->scope()->placeholder_typedefs()) {
    if (td.type()) {
      continue;
    }

    if (use_legacy_type_ref_resolution_ && !td.resolve()) {
      success = false;
    }

    diags.error(td, "Type `{}` not defined.", td.name());
    assert(!td.resolve());
    success = false;
  }
  return success && check_circular_typedef(diags, bundle);
}

bool sema::check_circular_typedef(
    sema_context& diags, t_program_bundle& bundle) {
  std::unordered_set<const t_type*> checked;
  for (auto& td : bundle.root_program()->scope()->placeholder_typedefs()) {
    if (checked.count(td.get_type())) {
      continue;
    }
    std::unordered_set<const t_type*> visited;
    std::vector<const t_type*> chain;
    if (nullptr != t_typedef::find_type_if(td.get_type(), [&](const t_type* t) {
          // Find the first typedef which insertion failed
          checked.insert(t);
          chain.push_back(t);
          return visited.insert(t).second == false;
        })) {
      std::string msg;
      for (const auto& i : chain) {
        msg += i->name() + (&i != &chain.back() ? " --> " : "");
      }
      diags.error(td, "Circular typedef: {}", msg);
      return false;
    }
  }
  return true;
}

sema::result sema::run(sema_context& ctx, t_program_bundle& bundle) {
  // Resolve types in the root program.
  if (!use_legacy_type_ref_resolution_) {
    type_ref_resolver().run(ctx, bundle);
  }

  t_program& root_program = *bundle.root_program();
  std::string program_prefix = root_program.name() + ".";

  result ret;
  for (t_placeholder_typedef& t :
       root_program.scope()->placeholder_typedefs()) {
    if (!t.resolve() && t.name().find(program_prefix) == 0) {
      ctx.error(t, "Type `{}` not defined.", t.name());
      ret.unresolved_types = true;
    }
  }
  if (ctx.has_errors()) {
    return ret;
  }

  for (auto& mutator : standard_mutators()) {
    mutator.mutate(ctx, bundle);
  }
  // We have no more mutators, so all type references **must** resolve.
  ret.unresolved_types = !resolve_all_types(ctx, bundle);
  if (!ret.unresolved_types) {
    standard_validator()(ctx, *bundle.root_program());
  }
  return ret;
}

void sema::add_schema(sema_context& ctx, t_program_bundle& bundle) {
  mutator_context mctx;
  mctx.bundle = &bundle;
  ast_mutator mutator;
  mutator.add_program_visitor(&inject_schema_const);
  mutator(ctx, mctx, *bundle.root_program());
}

} // namespace apache::thrift::compiler
