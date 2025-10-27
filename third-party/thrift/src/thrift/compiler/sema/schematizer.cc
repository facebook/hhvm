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

#include <thrift/compiler/sema/schematizer.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <fmt/core.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_global_scope.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/ast/type_visitor.h>

namespace apache::thrift::compiler::detail {
namespace {

template <typename... Args>
std::unique_ptr<t_const_value> val(Args&&... args) {
  return std::make_unique<t_const_value>(std::forward<Args>(args)...);
}
template <typename Enm, typename = std::enable_if_t<std::is_enum_v<Enm>>>
std::unique_ptr<t_const_value> val(Enm val) {
  return std::make_unique<t_const_value>(
      static_cast<std::underlying_type_t<Enm>>(val));
}
std::unique_ptr<t_const_value> val(std::string_view s) {
  return val(std::string{s});
}

const char* kBoolTypeName = "boolValue";
const char* kBinaryTypeName = "binaryValue";
const char* kByteTypeName = "byteValue";
const char* kDoubleTypeName = "doubleValue";
const char* kFloatTypeName = "floatValue";
const char* kI16TypeName = "i16Value";
const char* kI32TypeName = "i32Value";
const char* kI64TypeName = "i64Value";
const char* kStringTypeName = "stringValue";
const char* kListTypeName = "listValue";
const char* kMapTypeName = "mapValue";
const char* kSetTypeName = "setValue";

const char* const_value_type_name(t_const_value::t_const_value_kind kind) {
  switch (kind) {
    case t_const_value::t_const_value_kind::CV_BOOL:
      return kBoolTypeName;
    case t_const_value::t_const_value_kind::CV_INTEGER:
      return kI64TypeName;
    case t_const_value::t_const_value_kind::CV_DOUBLE:
      return kDoubleTypeName;
    case t_const_value::t_const_value_kind::CV_STRING:
      return kStringTypeName;
    case t_const_value::t_const_value_kind::CV_MAP:
      return kMapTypeName;
    case t_const_value::t_const_value_kind::CV_LIST:
      return kListTypeName;
    case t_const_value::t_const_value_kind::CV_IDENTIFIER:
      return kStringTypeName;
  }
  abort();
}
} // namespace

t_type_ref schematizer::std_type(std::string_view uri) {
  return t_type_ref::from_req_ptr(
      static_cast<const t_type*>(global_scope_.find_by_uri(uri)));
}

// Returns the `TypeUri` type & the corresponding Uri value for the given node
schematizer::resolved_uri schematizer::calculate_uri(
    const t_named& node, const bool use_hash) {
  if (use_hash) {
    return {"definitionKey", identify_definition(node)};
  }
  if (!node.uri().empty()) {
    return {"uri", node.uri()};
  }
  if (node.program()) {
    return {"scopedName", node.program()->scoped_name(node)};
  }
  return {"scopedName", node.name()};
}

std::unique_ptr<t_const_value> schematizer::type_uri(const t_type& type) {
  auto ret = t_const_value::make_map();
  auto uri = calculate_uri(type, opts_.use_hash);
  ret->add_map(val(uri.uri_type), val(std::move(uri.value)));
  ret->set_ttype(std_type("facebook.com/thrift/type/TypeUri"));
  return ret;
}

void schematizer::add_definition(
    t_const_value& schema,
    const t_named& node,
    const t_program* program,
    intern_func& intern_value) {
  auto definition = t_const_value::make_map();
  definition->add_map(val("name"), val(node.name()));
  if (!node.uri().empty()) {
    definition->add_map(val("uri"), val(node.uri()));
  } else if (auto program_from_node_2 = dynamic_cast<const t_program*>(&node)) {
    if (!program_from_node_2->package().empty()) {
      definition->add_map(
          val("uri"), val(program_from_node_2->package().name()));
    }
  }

  if (auto structured = node.structured_annotations();
      !structured.empty() && opts_.include_annotations) {
    auto annots = t_const_value::make_map();
    auto structured_annots = t_const_value::make_list();
    auto annots_by_key = t_const_value::make_map();

    for (const auto& item : structured) {
      auto annot = t_const_value::make_map();

      const auto ty_wrapper = protocol_value_builder{*item.type()};

      if (!item.value()->is_empty()) {
        auto protocol_value_ttype =
            std_type("facebook.com/thrift/protocol/Value");
        auto fields = t_const_value::make_map();
        for (const auto& [key, value] : item.value()->get_map()) {
          fields->add_map(
              key->clone(),
              ty_wrapper.property(*key).wrap(*value, protocol_value_ttype));
        }
        annot->add_map(val("fields"), std::move(fields));
      }

      // Double write to deprecated externed path. (T161963504)
      if (opts_.double_writes) {
        auto structured_annot = annot->clone();
        structured_annot->set_ttype(
            std_type("facebook.com/thrift/type/StructuredAnnotation"));
        structured_annot->add_map(val("type"), type_uri(*item.type()));

        auto id = intern_value(
            std::move(structured_annot), const_cast<t_program*>(program));
        structured_annots->add_list(val(id));
      }

      annot->set_ttype(std_type("facebook.com/thrift/type/Annotation"));
      auto unhashed_uri = calculate_uri(*item.type(), false /*use_hash*/);
      // We're not hashing & ignoring the UriType here, as annotations are
      // stored as map<string, Annotation>.
      annots->add_map(val(std::move(unhashed_uri.value)), annot->clone());

      auto hashed_uri = calculate_uri(*item.type(), true /*use_hash*/);
      annots_by_key->add_map(
          val(std::move(hashed_uri.value)), std::move(annot));
    }

    // Double write to deprecated externed path (T161963504).
    if (opts_.double_writes) {
      definition->add_map(
          val("structuredAnnotations"), std::move(structured_annots));
    }

    definition->add_map(val("annotations"), std::move(annots));
    definition->add_map(val("annotationsByKey"), std::move(annots_by_key));
  }

  if (auto unstructured = node.unstructured_annotations();
      !unstructured.empty() && opts_.include_annotations) {
    auto annots = t_const_value::make_map();

    for (const auto& pair : unstructured) {
      annots->add_map(val(pair.first), val(pair.second.value));
    }

    definition->add_map(val("unstructuredAnnotations"), std::move(annots));
  }

  if (node.has_doc() && opts_.include_docs) {
    auto docs = t_const_value::make_map();
    docs->add_map(val("contents"), val(node.doc()));
    definition->add_map(val("docs"), std::move(docs));
  }

  schema.add_map(val("attrs"), std::move(definition));
}

void add_as_definition(
    t_const_value& defns_schema,
    const std::string& defn_field,
    std::unique_ptr<t_const_value> schema) {
  auto defn_schema = t_const_value::make_map();
  defn_schema->add_map(val(defn_field), std::move(schema));

  defns_schema.add_list(std::move(defn_schema));
}

/// Returns a constant of type type_rep.TypeStruct,
/// resolving placeholder typedefs if needed.
std::unique_ptr<t_const_value> schematizer::gen_type(
    schematizer* generator,
    const t_program* program,
    t_const_value* defns_schema,
    const t_type& type) {
  auto schema = t_const_value::make_map();
  auto type_name = t_const_value::make_map();
  std::unique_ptr<t_const_value> params;

  auto* resolved_type = &type;
  while (auto* typedf = dynamic_cast<const t_typedef*>(resolved_type)) {
    if (typedf->typedef_kind() != t_typedef::kind::defined) {
      resolved_type = &*typedf->type();
      continue;
    }

    type_name->add_map(val("typedefType"), type_uri(*resolved_type));
    schema->add_map(val("name"), std::move(type_name));
    return schema;
  }

  resolved_type->visit(
      [&](const t_primitive_type& primitive) {
        switch (primitive.primitive_type()) {
          case t_primitive_type::type::t_void:
            break;
          case t_primitive_type::type::t_bool:
            type_name->add_map(val("boolType"), val(0));
            break;
          case t_primitive_type::type::t_byte:
            type_name->add_map(val("byteType"), val(0));
            break;
          case t_primitive_type::type::t_i16:
            type_name->add_map(val("i16Type"), val(0));
            break;
          case t_primitive_type::type::t_i32:
            type_name->add_map(val("i32Type"), val(0));
            break;
          case t_primitive_type::type::t_i64:
            type_name->add_map(val("i64Type"), val(0));
            break;
          case t_primitive_type::type::t_double:
            type_name->add_map(val("doubleType"), val(0));
            break;
          case t_primitive_type::type::t_float:
            type_name->add_map(val("floatType"), val(0));
            break;
          case t_primitive_type::type::t_string:
            type_name->add_map(val("stringType"), val(0));
            break;
          case t_primitive_type::type::t_binary:
            type_name->add_map(val("binaryType"), val(0));
            break;
        }
      },
      [&](const t_list& list) {
        type_name->add_map(val("listType"), val(0));
        params = t_const_value::make_list();
        params->add_list(
            gen_type(generator, program, defns_schema, *list.elem_type()));
      },
      [&](const t_set& set) {
        type_name->add_map(val("setType"), val(0));
        params = t_const_value::make_list();
        params->add_list(
            gen_type(generator, program, defns_schema, *set.elem_type()));
      },
      [&](const t_map& map) {
        type_name->add_map(val("mapType"), val(0));
        params = t_const_value::make_list();
        params->add_list(
            gen_type(generator, program, defns_schema, *map.key_type()));
        params->add_list(
            gen_type(generator, program, defns_schema, *map.val_type()));
      },
      [&](const t_enum& enum_type) {
        if (defns_schema && generator) {
          auto enum_schema = generator->gen_schema(enum_type);
          add_as_definition(*defns_schema, "enumDef", std::move(enum_schema));
        }
        type_name->add_map(val("enumType"), type_uri(*resolved_type));
      },
      [&](const t_struct& struct_type) {
        if (defns_schema && generator) {
          auto struct_schema = generator->gen_schema(struct_type);
          add_as_definition(
              *defns_schema, "structDef", std::move(struct_schema));
        }
        type_name->add_map(val("structType"), type_uri(*resolved_type));
      },
      [&](const t_union& union_type) {
        if (defns_schema && generator) {
          auto union_schema = generator->gen_schema(union_type);
          add_as_definition(*defns_schema, "unionDef", std::move(union_schema));
        }
        type_name->add_map(val("unionType"), type_uri(*resolved_type));
      },
      [&](const t_exception& exception_type) {
        if (defns_schema && generator) {
          auto ex_schema = generator->gen_schema(exception_type);
          add_as_definition(
              *defns_schema, "exceptionDef", std::move(ex_schema));
        }
        type_name->add_map(val("exceptionType"), type_uri(*resolved_type));
      },
      [&](const t_service&) { assert(false); },
      [&](const t_typedef&) {
        // This should not happen since we resolve typedefs above
        assert(false);
      });
  schema->add_map(val("name"), std::move(type_name));
  if (params) {
    schema->add_map(val("params"), std::move(params));
  }
  return schema;
}

void schematize_recursively(
    schematizer* generator,
    const t_program* program,
    t_const_value* defns_schema,
    const t_type& type) {
  auto* resolved_type = &type;
  while (auto* typedf = dynamic_cast<const t_typedef*>(resolved_type)) {
    if (typedf->typedef_kind() != t_typedef::kind::defined) {
      resolved_type = &*typedf->type();
      continue;
    }
    return;
  }

  resolved_type->visit(
      [&](const t_primitive_type&) {
        // No action needed for primitive types
      },
      [&](const t_list& list) {
        schematize_recursively(
            generator, program, defns_schema, *list.elem_type());
      },
      [&](const t_set& set) {
        schematize_recursively(
            generator, program, defns_schema, *set.elem_type());
      },
      [&](const t_map& map) {
        schematize_recursively(
            generator, program, defns_schema, *map.key_type());
        schematize_recursively(
            generator, program, defns_schema, *map.val_type());
      },
      [&](const t_enum& enum_type) {
        auto enum_schema = generator->gen_schema(enum_type);
        add_as_definition(*defns_schema, "enumDef", std::move(enum_schema));
      },
      [&](const t_struct& struct_type) {
        auto struct_schema = generator->gen_schema(struct_type);
        add_as_definition(*defns_schema, "structDef", std::move(struct_schema));
      },
      [&](const t_union& union_type) {
        auto union_schema = generator->gen_schema(union_type);
        add_as_definition(*defns_schema, "unionDef", std::move(union_schema));
      },
      [&](const t_exception& exception_type) {
        auto exception_schema = generator->gen_schema(exception_type);
        add_as_definition(
            *defns_schema, "exceptionDef", std::move(exception_schema));
      },
      [&](const t_service&) { assert(false); },
      [&](const t_typedef&) {
        // This should not happen since we resolve typedefs above
        assert(false);
      });
}

const t_enum* find_enum(const t_program* program, const std::string& enum_uri) {
  // May be null in unit tests.
  return program ? dynamic_cast<const t_enum*>(
                       program->global_scope()->find_by_uri(enum_uri))
                 : nullptr;
}

void add_qualifier(const t_enum* t_enum, t_const_value& schema, int enum_val) {
  auto qualifier_schema = val(enum_val);
  if (t_enum) {
    qualifier_schema->set_is_enum();
    qualifier_schema->set_enum(t_enum);
    qualifier_schema->set_enum_value(t_enum->find_value(enum_val));
  }
  schema.add_map(val("qualifier"), std::move(qualifier_schema));
}

void schematizer::add_fields(
    schematizer* generator,
    const t_program* program,
    t_const_value* defns_schema,
    t_const_value& schema,
    const std::string& fields_name,
    node_list_view<const t_field> fields,
    intern_func& intern_value) {
  auto fields_schema = t_const_value::make_list();

  const auto* field_qualifier_enum =
      find_enum(program, "facebook.com/thrift/type/FieldQualifier");

  for (const auto& field : fields) {
    auto field_schema = t_const_value::make_map();
    add_definition(*field_schema, field, program, intern_value);
    field_schema->add_map(val("id"), val(field.id()));

    add_qualifier(field_qualifier_enum, *field_schema, [&] {
      switch (field.qualifier()) {
        case t_field_qualifier::none:
          return 0; // Default
        case t_field_qualifier::required:
          return 3; // Fill
        case t_field_qualifier::optional:
          return 1; // Optional
        case t_field_qualifier::terse:
          return 2; // Terse
      }
      assert(false);
      return 0; // Default
    }());

    field_schema->add_map(
        val("type"), gen_type(generator, program, defns_schema, *field.type()));
    if (auto deflt = field.default_value()) {
      assert(program);
      auto clone = deflt->clone();
      clone->set_ttype(field.type());
      auto id = intern_value(std::move(clone), const_cast<t_program*>(program));
      field_schema->add_map(val("customDefault"), val(id));
    }
    fields_schema->add_list(std::move(field_schema));
  }

  schema.add_map(val(fields_name), std::move(fields_schema));
}

std::unique_ptr<t_const_value> schematizer::gen_schema(
    const t_structured& node) {
  auto schema = t_const_value::make_map();
  add_definition(*schema, node, node.program(), opts_.intern_value);
  add_fields(
      this,
      node.program(),
      nullptr,
      *schema,
      "fields",
      node.fields(),
      opts_.intern_value);

  if (node.is<t_exception>()) {
    const auto& ex = static_cast<const t_exception&>(node);
    schema->add_map(val("safety"), val(ex.safety()));
    schema->add_map(val("kind"), val(ex.kind()));
    schema->add_map(val("blame"), val(ex.blame()));
  }

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_full_schema(
    const t_service& node) {
  auto schema = t_const_value::make_map();
  auto dfns_schema = t_const_value::make_list();

  auto svc_schema = gen_schema(node);
  add_as_definition(*dfns_schema, "serviceDef", std::move(svc_schema));

  for (const auto& func : node.functions()) {
    const t_type_ref& ret = func.return_type();
    // TODO: Handle sink, stream, interactions
    if (!func.sink_or_stream()) {
      schematize_recursively(
          this, node.program(), dfns_schema.get(), *ret->get_true_type());
    }

    for (const auto& field : func.params().fields()) {
      schematize_recursively(
          this, node.program(), dfns_schema.get(), *field.type());
    }

    if (func.exceptions()) {
      for (const auto& field : func.exceptions()->fields()) {
        schematize_recursively(
            this, node.program(), dfns_schema.get(), *field.type());
      }
    }
  }

  if (auto parent = node.extends()) {
    add_as_definition(*dfns_schema, "serviceDef", gen_schema(*parent));
  }

  schema->add_map(val("definitions"), std::move(dfns_schema));
  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_service& node) {
  auto svc_schema = t_const_value::make_map();
  add_definition(*svc_schema, node, node.program(), opts_.intern_value);

  auto functions_schema = t_const_value::make_list();

  const auto* func_qualifier_enum =
      find_enum(node.program(), "facebook.com/thrift/type/FunctionQualifier");

  for (const auto& func : node.functions()) {
    auto func_schema = t_const_value::make_map();
    add_definition(*func_schema, func, node.program(), opts_.intern_value);

    add_qualifier(func_qualifier_enum, *func_schema, [&] {
      switch (func.qualifier()) {
        case t_function_qualifier::none:
          return 0;
        case t_function_qualifier::oneway:
          return 1;
        case t_function_qualifier::idempotent:
          return 2;
        case t_function_qualifier::readonly:
          return 3;
      }
      assert(false);
      return 0; // Default
    }());

    if (const auto& interaction = func.interaction()) {
      auto ref = t_const_value::make_map();
      ref->add_map(val("uri"), type_uri(*interaction));
      func_schema->add_map(val("interactionType"), std::move(ref));
    }
    if (!func.has_void_initial_response() ||
        (!func.sink_or_stream() && !func.interaction())) {
      const t_type* type = func.return_type().get_type();
      func_schema->add_map(val("returnType"), gen_type(*type, node.program()));
      // Double write of return type for backwards compatibility (T161963504).
      if (opts_.double_writes) {
        auto return_types_schema = t_const_value::make_list();
        auto schema = t_const_value::make_map();
        schema->add_map(val("thriftType"), gen_type(*type, node.program()));
        return_types_schema->add_list(std::move(schema));
        func_schema->add_map(
            val("returnTypes"), std::move(return_types_schema));
      }
    }

    if (auto stream = func.stream()) {
      auto stream_schema = t_const_value::make_map();
      stream_schema->add_map(
          val("payload"), gen_type(*stream->elem_type(), node.program()));
      if (auto throws = stream->exceptions()) {
        add_fields(
            this,
            node.program(),
            nullptr,
            *stream_schema,
            "exceptions",
            throws->fields(),
            opts_.intern_value);
      }
      auto return_type = t_const_value::make_map();
      return_type->add_map(val("streamType"), std::move(stream_schema));
      func_schema->add_map(val("streamOrSink"), std::move(return_type));
    } else if (auto sink = func.sink()) {
      auto sink_schema = t_const_value::make_map();
      sink_schema->add_map(
          val("payload"), gen_type(*sink->elem_type(), node.program()));
      if (auto throws = sink->sink_exceptions()) {
        add_fields(
            this,
            node.program(),
            nullptr,
            *sink_schema,
            "clientExceptions",
            throws->fields(),
            opts_.intern_value);
      }
      if (sink->final_response_type()) {
        sink_schema->add_map(
            val("finalResponse"),
            gen_type(*sink->final_response_type(), node.program()));
      }
      if (auto throws = sink->final_response_exceptions()) {
        add_fields(
            this,
            node.program(),
            nullptr,
            *sink_schema,
            "serverExceptions",
            throws->fields(),
            opts_.intern_value);
      }
      auto return_type = t_const_value::make_map();
      return_type->add_map(val("sinkType"), std::move(sink_schema));
      func_schema->add_map(val("streamOrSink"), std::move(return_type));
    }

    auto param_list_schema = t_const_value::make_map();
    add_fields(
        this,
        node.program(),
        nullptr,
        *param_list_schema,
        "fields",
        func.params().fields(),
        opts_.intern_value);
    func_schema->add_map(val("paramlist"), std::move(param_list_schema));

    if (func.exceptions()) {
      add_fields(
          this,
          node.program(),
          nullptr,
          *func_schema,
          "exceptions",
          func.exceptions()->fields(),
          opts_.intern_value);
    }

    functions_schema->add_list(std::move(func_schema));
  }
  svc_schema->add_map(val("functions"), std::move(functions_schema));

  if (auto parent = node.extends()) {
    auto ref = t_const_value::make_map();
    ref->add_map(val("uri"), type_uri(*parent));
    svc_schema->add_map(val("baseService"), std::move(ref));
  }

  return svc_schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_const& node) {
  const auto* program = node.program();
  assert(program);

  auto schema = t_const_value::make_map();
  add_definition(*schema, node, program, opts_.intern_value);

  schema->add_map(val("type"), gen_type(*node.type(), program));

  std::unique_ptr<t_const_value> clone = node.value()->clone();
  clone->set_ttype(node.type_ref());
  auto id =
      opts_.intern_value(std::move(clone), const_cast<t_program*>(program));
  schema->add_map(val("value"), val(id));

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_enum& node) {
  auto schema = t_const_value::make_map();
  add_definition(*schema, node, node.program(), opts_.intern_value);

  auto values = t_const_value::make_list();

  for (const auto& value : node.values()) {
    auto value_schema = t_const_value::make_map();
    add_definition(*value_schema, value, node.program(), opts_.intern_value);
    value_schema->add_map(val("value"), val(value.get_value()));
    values->add_list(std::move(value_schema));
  }

  schema->add_map(val("values"), std::move(values));

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_program& node) {
  auto schema = t_const_value::make_map();
  add_definition(*schema, node, &node, opts_.intern_value);

  schema->add_map(val("path"), val(node.path()));

  if (!node.language_includes().empty()) {
    auto langs = t_const_value::make_map();
    for (const auto& [lang, incs] : node.language_includes()) {
      auto includes = t_const_value::make_list();
      for (const auto& inc : incs) {
        includes->add_list(val(inc));
      }
      langs->add_map(val(lang), std::move(includes));
    }
    schema->add_map(val("languageIncludes"), std::move(langs));
  }

  if (!node.namespaces().empty()) {
    auto langs = t_const_value::make_map();
    for (const auto& [lang, langNamespace] : node.namespaces()) {
      langs->add_map(val(lang), val(langNamespace));
    }
    schema->add_map(val("namespaces"), std::move(langs));
  }

  // The remaining fields are intern IDs and have to be stiched in by the
  // caller.

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_typedef& node) {
  auto schema = t_const_value::make_map();
  add_definition(*schema, node, node.program(), opts_.intern_value);
  schema->add_map(val("type"), gen_type(*node.type(), node.program()));
  return schema;
}

const char* protocol_value_type_name(const t_type& ty) {
  if (const auto* primitive = ty.try_as<t_primitive_type>()) {
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_bool:
        return kBoolTypeName;
      case t_primitive_type::type::t_byte:
        return kByteTypeName;
      case t_primitive_type::type::t_i16:
        return kI16TypeName;
      case t_primitive_type::type::t_i32:
        return kI32TypeName;
      case t_primitive_type::type::t_i64:
        return kI64TypeName;
      case t_primitive_type::type::t_float:
        return kFloatTypeName;
      case t_primitive_type::type::t_double:
        return kDoubleTypeName;
      case t_primitive_type::type::t_string:
        return kStringTypeName;
      case t_primitive_type::type::t_binary:
        return kBinaryTypeName;
      default:
        break;
    }
  }

  if (const auto* container = ty.try_as<t_container>()) {
    switch (container->container_type()) {
      case t_container::type::t_list:
        return kListTypeName;
      case t_container::type::t_set:
        return kSetTypeName;
      case t_container::type::t_map:
        return kMapTypeName;
    }
  }

  if (ty.is<t_enum>()) {
    return kI32TypeName;
  }

  abort();
}

protocol_value_builder::protocol_value_builder(const t_type& struct_ty)
    : ty_{struct_ty.get_true_type()} {}

protocol_value_builder::protocol_value_builder() : ty_{nullptr} {}

[[nodiscard]] protocol_value_builder protocol_value_builder::as_value_type() {
  return protocol_value_builder{};
}

[[nodiscard]] protocol_value_builder protocol_value_builder::property(
    const t_const_value& key) const {
  if (ty_ == nullptr) {
    return as_value_type();
  }

  return ty_->visit(
      [&](const t_map& map) {
        // Maps are not restricted to string-based field keys, so we should
        // extend the look-up to any sealed key type.
        return protocol_value_builder{*map.val_type()};
      },
      [&](const t_struct& strct) {
        assert(
            key.kind() == t_const_value::CV_STRING &&
            "A struct only has named fields");
        const auto* field = strct.get_field_by_name(key.get_string());
        return protocol_value_builder{*field->type().get_type()};
      },
      [&](const t_union& union_) {
        assert(
            key.kind() == t_const_value::CV_STRING &&
            "A union only has named fields");
        const auto* field = union_.get_field_by_name(key.get_string());
        return protocol_value_builder{*field->type().get_type()};
      },
      [&](auto&&) -> protocol_value_builder {
        throw std::logic_error(
            fmt::format(
                "Invalid name={} for property look-up", ty_->get_full_name()));
      });
}

[[nodiscard]] protocol_value_builder protocol_value_builder::key(
    [[maybe_unused]] const t_const_value& key) const {
  if (ty_ == nullptr) {
    return as_value_type();
  }

  return ty_->visit(
      [&](const t_struct&) {
        assert(
            key.kind() == t_const_value::CV_STRING &&
            "A struct only has named fields");
        return as_value_type();
      },
      [&](const t_union&) {
        assert(
            key.kind() == t_const_value::CV_STRING &&
            "A union only has named fields");
        return as_value_type();
      },
      [&](const t_map& map) {
        return protocol_value_builder{map.key_type().deref()};
      },
      [&](auto&&) -> protocol_value_builder {
        throw std::logic_error(
            fmt::format(
                "Invalid name={} for key look-up", ty_->get_full_name()));
      });
}

[[nodiscard]] protocol_value_builder protocol_value_builder::container_element(
    const t_const_value& container) const {
  assert(container.kind() == t_const_value::CV_LIST);
  if (container.get_list().empty()) {
    // If the list is empty, we don't care what the type resolves to
    return as_value_type();
  }

  return ty_->visit(
      [&](const t_list& list) {
        return protocol_value_builder{*list.elem_type()};
      },
      [&](const t_set& set) {
        return protocol_value_builder{*set.elem_type()};
      },
      [&](const t_map&) -> protocol_value_builder {
        throw std::logic_error(
            "Maps should not deduce a single container element");
      },
      [&](auto&&) -> protocol_value_builder {
        throw std::logic_error(
            fmt::format(
                "Invalid name={} for container element look-up",
                ty_->get_full_name()));
      });
}

std::pair<std::unique_ptr<t_const_value>, std::unique_ptr<t_const_value>>
protocol_value_builder::to_labeled_value(
    const t_const_value& protocol_value) const {
  if (ty_ == nullptr) {
    auto label = val(const_value_type_name(protocol_value.kind()));
    return std::make_pair(std::move(label), protocol_value.clone());
  }

  std::unique_ptr<t_const_value> value;
  auto raise_exception = [&] {
    throw std::runtime_error(
        fmt::format(
            "Could not match value of kind {} to type {}",
            t_const_value::kind_to_string(protocol_value.kind()),
            ty_->get_full_name()));
  };

  // Verify that the field is a valid type for a protocol value key
  switch (protocol_value.kind()) {
    case t_const_value::CV_INTEGER:
      if (!(ty_->is_any_int() || ty_->is<t_enum>() ||
            ty_->is_floating_point())) {
        raise_exception();
      }
      if (ty_->is_floating_point()) {
        // Coalesce to double
        value = std::make_unique<t_const_value>();
        value->set_double(protocol_value.get_integer());
      } else {
        value = protocol_value.clone();
      }
      break;
    case t_const_value::CV_DOUBLE:
      if (!ty_->is_floating_point()) {
        raise_exception();
      }
      value = protocol_value.clone();
      break;
    case t_const_value::CV_STRING:
      if (!ty_->is_string_or_binary()) {
        raise_exception();
      }
      value = protocol_value.clone();
      break;
    case t_const_value::CV_BOOL:
    case t_const_value::CV_MAP:
    case t_const_value::CV_LIST:
    case t_const_value::CV_IDENTIFIER:
      // These all have unequivocal keys
      value = protocol_value.clone();
      break;
  }

  // Generate a protocol value label for a given `t_const_value`, based on the
  // type of the field it represents. e.g. `i64Value` for `i64`.
  auto label = val(protocol_value_type_name(*ty_));
  return std::make_pair(std::move(label), std::move(value));
}

std::unique_ptr<t_const_value> protocol_value_builder::wrap(
    const t_const_value& protocol_value, t_type_ref ttype) const {
  auto ret = t_const_value::make_map();
  ret->set_ttype(ttype);
  switch (protocol_value.kind()) {
    case t_const_value::CV_BOOL:
    case t_const_value::CV_INTEGER:
    case t_const_value::CV_DOUBLE:
    case t_const_value::CV_STRING: {
      auto [value_label, value] = to_labeled_value(protocol_value);
      ret->add_map(std::move(value_label), std::move(value));
      break;
    }
    case t_const_value::CV_MAP: {
      auto map = t_const_value::make_map();
      for (const auto& [k, v] : protocol_value.get_map()) {
        map->add_map(key(*k).wrap(*k, ttype), property(*k).wrap(*v, ttype));
      }
      ret->add_map(val("mapValue"), std::move(map));
      break;
    }
    case t_const_value::CV_LIST: {
      auto list = t_const_value::make_list();
      auto list_ty_resolver = container_element(protocol_value);
      for (const auto& list_elem : protocol_value.get_list()) {
        list->add_list(list_ty_resolver.wrap(*list_elem, ttype));
      }
      ret->add_map(val(protocol_value_type_name(*ty_)), std::move(list));
      break;
    }
    case t_const_value::CV_IDENTIFIER:
      break;
  }
  return ret;
}

std::string_view schematizer::program_checksum(const t_program& program) {
  if (auto it = program_checksums_.find(&program);
      it != program_checksums_.end()) {
    return it->second;
  }
  // @lint-ignore CLANGTIDY facebook-hte-CArray
  unsigned char hash[MD5_DIGEST_LENGTH];
  auto val = sm_.get_file(program.path())->text;
  EVP_Digest(val.data(), val.size(), hash, nullptr, EVP_md5(), nullptr);
  return (
      program_checksums_[&program] =
          std::string(reinterpret_cast<const char*>(hash), sizeof(hash)));
}

size_t schematizer::definition_identifier_length() {
  return 16;
}

std::string schematizer::identify_definition(const t_named& node) {
  // @lint-ignore CLANGTIDY facebook-hte-CArray
  unsigned char hash[SHA256_DIGEST_LENGTH];
  auto val = fmt::format(
      "{}{}{}",
      program_checksum(*node.program()),
      node.program()->path(),
      node.name());
  SHA256(reinterpret_cast<const unsigned char*>(val.c_str()), val.size(), hash);
  return std::string(
      reinterpret_cast<const char*>(hash), definition_identifier_length());
}

int64_t schematizer::identify_program(const t_program& node) {
  auto checksum = program_checksum(node);
  auto path =
      std::filesystem::path(node.path()).lexically_normal().generic_string();
  // @lint-ignore CLANGTIDY facebook-hte-CArray
  unsigned char hash[SHA256_DIGEST_LENGTH];
  auto val = fmt::format("{}{}", checksum, path);
  SHA256(reinterpret_cast<const unsigned char*>(val.c_str()), val.size(), hash);
  int64_t ret;
  memcpy(&ret, hash, sizeof(ret));
  return ret;
}

std::string schematizer::name_schema(
    source_manager& sm, const t_program& node) {
  schematizer s(*node.global_scope(), sm, {});
  return fmt::format(
      "_fbthrift_schema_{:x}", static_cast<uint64_t>(s.identify_program(node)));
}

} // namespace apache::thrift::compiler::detail
