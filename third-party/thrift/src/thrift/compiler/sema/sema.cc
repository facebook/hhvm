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

void report_missing_type(sema_context& ctx, const t_named& named) {
  ctx.error(named, "Type `{}` not defined.", named.name());
}

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
  sema_context& ctx_;
  t_program_bundle& bundle_;
  bool unresolved_ = false;

 public:
  explicit type_ref_resolver(sema_context& ctx, t_program_bundle& bundle)
      : ctx_{ctx}, bundle_{bundle} {}

  const t_type* resolve_implicit_includes(const t_placeholder_typedef& td) {
    const scope::identifier id{td.name(), td.src_range()};
    return dynamic_cast<const t_type*>(bundle_.root_program()->find(id));
  }

  void resolve_in_place(t_type_ref& ref) {
    if (ref.resolve()) {
      return;
    }

    if (const auto* node =
            resolve_implicit_includes(*ref.get_unresolved_type())) {
      ref = t_type_ref{*node};
      ref.resolve();
      assert(ref.resolved());
      return;
    }

    unresolved_ = true;
  }

  [[nodiscard]] t_type_ref resolve(t_type_ref ref) {
    resolve_in_place(ref);
    return ref;
  }

  bool run() {
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
        [&](sema_context&, mutator_context&, t_function& node) {
          resolve_in_place(node.return_type());
          resolve_in_place(node.interaction());
        });
    mutator.add_function_param_visitor(
        [&](sema_context& ctx, mutator_context& mctx, t_field& param) {
          // Delegates to field visitor.
          mutator(ctx, mctx, param);
        });
    mutator.add_thrown_exception_visitor(
        [&](sema_context& ctx, mutator_context& mctx, t_field& field) {
          // Delegates to field visitor.
          mutator(ctx, mctx, field);
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

    mutator.mutate(ctx_, bundle_);
    return !unresolved_;
  }
};

const t_const* try_resolve_enum_by_id(
    const scope::identifier id, const t_const_value& value) {
  const auto resolve_enum_alias =
      [&](const scope::identifier enum_or_alias) -> const t_enum* {
    const t_named* enum_ty = value.program().find(enum_or_alias);
    if (const auto* enum_typedef = ast_detail::as<t_typedef>(enum_ty)) {
      return ast_detail::as<t_enum>(enum_typedef->get_true_type());
    }
    return ast_detail::as<t_enum>(enum_ty);
  };

  const auto resolve_value =
      [&](const scope::identifier value_id) -> const t_const* {
    return value.program().find<t_const>(value_id);
  };

  const auto resolve_maybe_aliased_enum_with_value =
      [&](const scope::identifier enum_alias,
          const std::string_view value_name) -> const t_const* {
    if (const t_enum* enum_node = resolve_enum_alias(enum_alias)) {
      return enum_node->find_const_by_name(value_name);
    }
    return nullptr;
  };

  return id.visit(
      [&](const scope::unscoped_id& id) {
        // `id` is just the enum value name, e.g. MyEnum a = SOMETHING;
        // So try to find `SOMETHING`.
        return resolve_value(id);
      },
      [&](const scope::scoped_id& id) -> const t_const* {
        // `id` is either the program scope or the enum name followed by
        // the value e.g.
        // 1. MyEnum a = my_prog.MY_VALUE;
        // 2. MyEnum a = MY_ALIAS.VALUE;

        // (1) should be resolvable as-is.
        if (const auto* node = value.program().find<t_const>(id)) {
          return node;
        }

        // (2) requires resolving the enum alias, then finding its value.
        return resolve_maybe_aliased_enum_with_value(
            scope::identifier{id.scope, id.loc}, id.name);
      },
      [&](const scope::enum_id& id) {
        // `id` is the fully qualified name, e.g.
        // MyEnum a = my_prog.MY_NAME.SOMETHING;
        // `MY_NAME` is either:
        // 1. An enum type
        // 2. A typedef to an enum type
        // So we'll try to resolve the enum type, then find the value.
        return resolve_maybe_aliased_enum_with_value(
            scope::scoped_id{
                .loc = id.loc, .scope = id.scope, .name = id.enum_name},
            id.value_name);
      });
}

void match_type_with_const_value(
    sema_context& ctx,
    mutator_context& mctx,
    const t_type* long_type,
    t_const_value* value) {
  const t_type* type = long_type->get_true_type();
  if (!type) {
    // type is unresolved, which will fail in validation.
    return;
  }

  // Verify that the const value is correctly resolved at this point.
  if (value && value->kind() == t_const_value::CV_IDENTIFIER) {
    const std::string& value_id = value->get_identifier();
    const scope::identifier id{
        value_id, value->src_range().value_or(source_range{})};
    const t_const* constant;
    if (type->is<t_enum>()) {
      // Try to resolve enum values from typedefs
      // or enums defined after use.
      constant = try_resolve_enum_by_id(id, *value);
    } else {
      constant = mctx.program().find<t_const>(id);
    }

    if (!constant) {
      ctx.error(
          value->ref_range().begin,
          "use of undeclared identifier '{}'",
          value_id);
      return;
    }
    value->assign(t_const_value(*constant->value()));
  }

  if (const t_list* list = type->try_as<t_list>()) {
    if (value->kind() == t_const_value::CV_LIST) {
      for (auto list_val : value->get_list()) {
        match_type_with_const_value(
            ctx, mctx, &list->elem_type().deref(), list_val);
      }
    }
  } else if (const t_set* set = type->try_as<t_set>()) {
    if (value->kind() == t_const_value::CV_LIST) {
      for (auto set_val : value->get_list()) {
        match_type_with_const_value(
            ctx, mctx, &set->elem_type().deref(), set_val);
      }
    }
  } else if (const auto* map = type->try_as<t_map>()) {
    if (value->kind() == t_const_value::CV_MAP) {
      for (auto map_val : value->get_map()) {
        match_type_with_const_value(
            ctx, mctx, &map->key_type().deref(), map_val.first);
        match_type_with_const_value(
            ctx, mctx, &map->val_type().deref(), map_val.second);
      }
    }
  } else if (const t_structured* structured = type->try_as<t_structured>()) {
    if (auto ttype = value->ttype()) {
      if (!ttype.resolved()) {
        ctx.error(
            value->ref_range().begin,
            // Here we have expected type which allows us to output more
            // detail to the error. Hence it's better to error here as opposed
            // to later in validator which will only print 'unknown symbol'
            "could not resolve type `{}` (expected `{}`)",
            ttype.get_unresolved_type()->get_full_name(),
            type->get_full_name());
        // Resolve global placeholder to avoid the duplicate message
        for (auto& td : mctx.bundle->root_program()
                            ->global_scope()
                            ->placeholder_typedefs()) {
          if (!td.type() &&
              td.name() == ttype.get_unresolved_type()->get_full_name()) {
            td.set_type(t_type_ref::from_ptr(type));
          }
        }
      } else if (ttype->get_true_type() != type) {
        ctx.error(
            value->ref_range().begin,
            "type mismatch: expected {}, got {}",
            type->get_full_name(),
            ttype->get_full_name());
      }
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
          // TODO(sadroeck) - Deprecate this behavior
          map_key->convert_identifier_to_string();
        }
        match_type_with_const_value(ctx, mctx, field->get_type(), map_val);
      }
    }
  } else if (const t_enum* enm = type->try_as<t_enum>()) {
    // Set constant value types as enums when they are declared with integers
    // or identifiers.
    if (!value->is_enum()) {
      value->set_is_enum();
      value->set_enum(enm);
      if (value->kind() == t_const_value::CV_INTEGER) {
        if (const auto* enum_value = enm->find_value(value->get_integer())) {
          value->set_enum_value(enum_value);
        }
      }
    }
  } else if (type->is<t_primitive_type>()) {
    // Remove enum_value if type is a base_type to use the integer instead.
    if (value->is_enum()) {
      value->set_enum_value(nullptr);
    }
  } else if (type->is<t_service>()) {
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

  match_type_with_const_value(ctx, mctx, type, value);
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
        field.has_structured_annotation(kTerseWriteUri);
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
  source_range loc{};
  try {
    const auto& annotation_value =
        annotation->get_value_from_structured_annotation("type");
    type_string = annotation_value.get_string();
    loc = annotation_value.src_range().value_or(source_range{});
  } catch (const std::exception& e) {
    ctx.error("{}", e.what());
    return;
  }
  const scope::identifier id{type_string, loc};

  const t_type* ttype = node.program()->find<t_type>(id);
  if (!ttype && annotation->program()) {
    // [TEMPORARY] Allow injected metadata fields to use the scope name
    // of the program they're defined in. e.g.
    // @internal.InjectMetadataFields{type = "Bar"} in foo.thrift should be
    // resolved as foo.Bar, not just Bar.
    ttype = annotation->program()->find<t_type>(id);
  }
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
  if (structured == nullptr || ttype->is<t_union>() ||
      ttype->is<t_exception>() || ttype->is<t_paramlist>()) {
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

// Strips haskell annotations and optionally inserts new annotations.
void update_annotations(
    t_named& node,
    std::map<std::string, std::string> new_annotations = {},
    deprecated_annotation_value::origin new_annotation_origin = {}) {
  auto annotations = node.unstructured_annotations();
  // First strip any haskell annotations
  for (auto it = annotations.begin(); it != annotations.end();) {
    if (it->first.find("hs.") == 0) {
      it = annotations.erase(it);
    } else {
      ++it;
    }
  }
  for (auto& [k, v] : new_annotations) {
    annotations[k] = {source_range{}, v, new_annotation_origin};
  }
  node.reset_annotations(std::move(annotations));
}

// Updates field or typedef's type field to hold annotations
template <typename Node>
void add_annotations_to_node_type(
    Node& node,
    std::map<std::string, std::string> annotations,
    t_program& program,
    deprecated_annotation_value::origin origin) {
  const t_type* node_type = node.get_type();

  if (annotations.empty()) {
    if (!node_type->unstructured_annotations().empty()) {
      update_annotations(const_cast<t_type&>(*node_type));
    }
    return;
  }

  if (node_type->is<t_container>() ||
      (node_type->is<t_typedef>() &&
       static_cast<const t_typedef*>(node_type)->typedef_kind() !=
           t_typedef::kind::defined) ||
      (node_type->is<t_primitive_type>() &&
       !node_type->unstructured_annotations().empty())) {
    // This is a new type we can modify in place
    update_annotations(
        const_cast<t_type&>(*node_type), std::move(annotations), origin);
  } else if (
      const t_primitive_type* primitive =
          node_type->try_as<t_primitive_type>()) {
    // Copy type as we don't handle unnamed typedefs to base types :(
    auto unnamed = std::make_unique<t_primitive_type>(*primitive);
    for (auto& pair : annotations) {
      unnamed->set_unstructured_annotation(pair.first, pair.second, {}, origin);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_type(std::move(unnamed));
  } else {
    // Wrap in an unnamed typedef :(
    auto unnamed = t_typedef::make_unnamed(
        const_cast<t_program*>(node_type->program()),
        node_type->name(),
        t_type_ref::from_ptr(node_type));
    for (auto& pair : annotations) {
      unnamed->set_unstructured_annotation(pair.first, pair.second, {}, origin);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_typedef(std::move(unnamed));
  }
}

void lower_deprecated_annotations(
    sema_context& ctx, mutator_context& mCtx, t_named& node) {
  if (auto cnst = node.find_structured_annotation_or_null(
          kDeprecatedUnvalidatedAnnotationsUri)) {
    ctx.check(
        std::all_of(
            node.unstructured_annotations().begin(),
            node.unstructured_annotations().end(),
            [](const auto& pair) { return pair.first.find("hs.") == 0; }),
        "Cannot combine @thrift.DeprecatedUnvalidatedAnnotations with legacy annotation syntax.");
    auto val = cnst->get_value_from_structured_annotation_or_null("items");
    if (!val || val->get_map().empty()) {
      ctx.error(
          "Must specify at least one item in @thrift.DeprecatedUnvalidatedAnnotations.");
      return;
    }
    if (!ctx.sema_parameters().skip_lowering_annotations) {
      deprecated_annotation_map map;
      for (auto& [k, v] : val->get_map()) {
        map[k->get_string()] = {
            {},
            v->get_string(),
            deprecated_annotation_value::origin::lowered_unstructured};
      }

      // The java generator has some interesting logic due to limitations in
      // mustache that are being fixed in whisker. Until that lands, we
      // propagate the affected annotations to the type of a typedef in order to
      // unblock deprecating unstructured annotations.
      if (map.count("java.swift.type") ||
          map.count("java.swift.binary_string")) {
        auto annot = map.count("java.swift.type") ? "java.swift.type"
                                                  : "java.swift.binary_string";
        auto* type = dynamic_cast<t_typedef*>(&node);
        if (!type) {
          ctx.error("Annotation {} is only supported on typedefs.", annot);
          return;
        }

        auto* inner_type = const_cast<t_type*>(type->get_type());
        if (inner_type->is<t_typedef>()) {
          ctx.error("Cannot use {} on typedefs of typedefs", annot);
        } else if (
            !inner_type->is<t_container>() &&
            !inner_type->is<t_primitive_type>()) {
          ctx.error(
              "Annotation {} is only supported on typedefs of primitive or container types.",
              annot);
        } else {
          // Ensure annotations can be added to inner type
          if (inner_type->is<t_primitive_type>() &&
              inner_type->unstructured_annotations().empty()) {
            auto new_type = std::make_unique<t_primitive_type>(
                static_cast<const t_primitive_type&>(*inner_type));
            inner_type = new_type.get();
            type->set_type(t_type_ref::from_ptr(inner_type));
            mCtx.program().add_unnamed_type(std::move(new_type));
          }

          inner_type->set_unstructured_annotation(
              annot,
              map[annot].value,
              {},
              deprecated_annotation_value::origin::lowered_unstructured);
        }

        map.erase(annot);
      }

      node.reset_annotations(std::move(map));
    } else {
      update_annotations(node);
    }
  } else {
    update_annotations(node);
  }
}

void normalize_return_type(
    sema_context& ctx, mutator_context&, t_function& node) {
  auto& type = node.return_type();
  if (!type.resolve()) {
    ctx.error(node, "Failed to resolve return type of `{}`.", node.name());
    return;
  }

  // Check the (first) response type.
  const t_type* true_type = type->get_true_type();
  if (true_type->is<t_service>()) {
    ctx.error("Invalid first response type: {}", type->get_full_name());
  }
}

template <typename Node>
void lower_cpp_type_annotations(
    sema_context& ctx, mutator_context& mctx, Node& node) {
  std::map<std::string, std::string> unstructured;

  if (!ctx.sema_parameters().skip_lowering_cpp_type_annotations) {
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

  add_annotations_to_node_type(
      node,
      std::move(unstructured),
      mctx.program(),
      deprecated_annotation_value::origin::lowered_cpp_type);
}

void inject_schema_const(sema_context& ctx, mutator_context&, t_program& prog) {
  if (prog.has_structured_annotation(kDisableSchemaConstUri)) {
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

// scope.thrift can't use structured annotations for circular dependency reasons
// so inject annotations here.
void add_magic_annotations(sema_context&, mutator_context&, t_struct& node) {
  if (node.uri() == "facebook.com/thrift/annotation/Function") {
    node.set_unstructured_annotation("hack.name", "TFunction");
    node.set_unstructured_annotation("js.name", "TFunction");
  } else if (node.uri() == "facebook.com/thrift/annotation/Const") {
    node.set_unstructured_annotation("hack.name", "TConst");
  } else if (node.uri() == "facebook.com/thrift/annotation/Enum") {
    node.set_unstructured_annotation("py3.hidden", "1");
  } else if (node.uri() == "facebook.com/thrift/annotation/Interface") {
    node.set_unstructured_annotation("hack.name", "TInterface");
  }
}

std::vector<ast_mutator> standard_mutators() {
  std::vector<ast_mutator> mutators;

  ast_mutator initial;
  initial.add_field_visitor(&lower_cpp_type_annotations<t_field>);
  initial.add_typedef_visitor(&lower_cpp_type_annotations<t_typedef>);
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
  main.add_struct_visitor(&add_magic_annotations);
  mutators.push_back(std::move(main));

  return mutators;
}

} // namespace

bool sema::resolve_all_types(sema_context& diags, t_program_bundle& bundle) {
  bool success = true;
  type_ref_resolver resolver(diags, bundle);

  if (!use_legacy_type_ref_resolution_) {
    success = resolver.run();
  }

  for (auto& td :
       bundle.root_program()->global_scope()->placeholder_typedefs()) {
    if (td.type()) {
      continue;
    }

    if (const auto* ttype = resolver.resolve_implicit_includes(td)) {
      td.set_type(t_type_ref::from_ptr(ttype));
      assert(td.type().resolved());
      continue;
    }

    if (use_legacy_type_ref_resolution_ && !td.resolve()) {
      success = false;
    }

    report_missing_type(diags, td);

    assert(!td.resolve());
    success = false;
  }
  return success && check_circular_typedef(diags, bundle);
}

bool sema::check_circular_typedef(
    sema_context& diags, t_program_bundle& bundle) {
  std::unordered_set<const t_type*> checked;
  for (auto& td :
       bundle.root_program()->global_scope()->placeholder_typedefs()) {
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
    type_ref_resolver(ctx, bundle).run();
  }

  t_program& root_program = *bundle.root_program();
  std::string program_prefix = root_program.name() + ".";

  result ret;
  for (t_placeholder_typedef& t :
       root_program.global_scope()->placeholder_typedefs()) {
    if (!t.resolve() && t.name().find(program_prefix) == 0) {
      report_missing_type(ctx, t);
      ret.unresolved_types = true;
    }
  }
  if (ctx.has_errors()) {
    return ret;
  }

  ret.unresolved_types = !resolve_all_types(ctx, bundle);
  if (!ret.unresolved_types) {
    for (auto& mutator : standard_mutators()) {
      mutator.mutate(ctx, bundle);
    }
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
