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

#include <thrift/compiler/generate/schema_populator.h>

#include <string>
#include <type_traits>
#include <utility>
#include <fmt/core.h>
#include <folly/io/IOBuf.h>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_global_scope.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_interface.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/ast/type_visitor.h>
#include <thrift/compiler/generate/const_util.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/Object.h>

namespace apache::thrift::compiler::detail {
namespace {

using apache::thrift::type::Definition;
using apache::thrift::type::DefinitionList;
using apache::thrift::type::FieldQualifier;
using apache::thrift::type::FunctionQualifier;
using apache::thrift::type::ReturnType;
using apache::thrift::type::Type;
using apache::thrift::type::TypeName;
using apache::thrift::type::TypeStruct;
using apache::thrift::type::TypeUri;
using apache::thrift::type::Void;

template <typename Enm>
auto as_underlying(Enm val) {
  return static_cast<std::underlying_type_t<Enm>>(val);
}

void set_void(auto&& ref) {
  ref.ensure() = Void::Unused;
}

Definition as_definition(auto schema) {
  Definition def;
  using Schema = decltype(schema);
  if constexpr (std::is_same_v<Schema, type::Struct>) {
    def.structDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Union>) {
    def.unionDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Exception>) {
    def.exceptionDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Enum>) {
    def.enumDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Typedef>) {
    def.typedefDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Const>) {
    def.constDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Service>) {
    def.serviceDef_ref() = std::move(schema);
  } else if constexpr (std::is_same_v<Schema, type::Interaction>) {
    def.interactionDef_ref() = std::move(schema);
  } else {
    static_assert(!sizeof(Schema), "unsupported schema definition");
  }
  return def;
}

template <typename Tag>
struct protocol_value_converter;

protocol::Value protocol_value_object(const protocol::Value& value);

template <typename T>
protocol::Value concrete_protocol_value(const T& value) {
  using ConcreteType = std::remove_cvref_t<T>;

  protocol::Value ret;
  auto& obj = ret.emplace_object();
  obj.type() = std::string{apache::thrift::uri<ConcreteType>()};

  op::for_each_field_id<ConcreteType>([&](auto field_id) {
    using Id = decltype(field_id);
    using Tag = op::get_type_tag<ConcreteType, Id>;

    if (const auto* field = op::get_value_or_null(op::get<Id>(value))) {
      obj[FieldId{folly::to_underlying(Id::value)}] =
          protocol_value_converter<Tag>::convert(*field);
    }
  });
  return ret;
}

template <typename Tag>
struct protocol_value_converter {
  template <typename T>
  static protocol::Value convert(const T& value) {
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, protocol::Value>) {
      return protocol_value_object(value);
    } else {
      return protocol::asValueStruct<Tag>(value);
    }
  }
};

template <typename T, typename Tag>
struct protocol_value_converter<type::cpp_type<T, Tag>>
    : protocol_value_converter<Tag> {};

template <typename T>
struct protocol_value_converter<type::struct_t<T>> {
  static protocol::Value convert(const T& value) {
    return concrete_protocol_value(value);
  }
};

template <typename T>
struct protocol_value_converter<type::union_t<T>> {
  static protocol::Value convert(const T& value) {
    return concrete_protocol_value(value);
  }
};

template <typename Elem>
struct protocol_value_converter<type::list<Elem>> {
  template <typename C>
  static protocol::Value convert(const C& value) {
    protocol::Value ret;
    auto& list = ret.emplace_list();
    list.reserve(value.size());
    for (const auto& item : value) {
      list.push_back(protocol_value_converter<Elem>::convert(item));
    }
    return ret;
  }
};

template <typename Elem>
struct protocol_value_converter<type::set<Elem>> {
  template <typename C>
  static protocol::Value convert(const C& value) {
    protocol::Value ret;
    auto& set = ret.emplace_set();
    set.reserve(value.size());
    for (const auto& item : value) {
      set.insert(protocol_value_converter<Elem>::convert(item));
    }
    return ret;
  }
};

template <typename Key, typename Mapped>
struct protocol_value_converter<type::map<Key, Mapped>> {
  template <typename C>
  static protocol::Value convert(const C& value) {
    protocol::Value ret;
    auto& map = ret.emplace_map();
    map.reserve(value.size());
    for (const auto& [key, mapped] : value) {
      map.emplace(
          protocol_value_converter<Key>::convert(key),
          protocol_value_converter<Mapped>::convert(mapped));
    }
    return ret;
  }
};

template <typename Tag>
struct protocol_value_union_field_converter {
  template <typename T>
  static protocol::Value convert(const protocol::Value& original, const T&) {
    return original;
  }
};

template <typename T, typename Tag>
struct protocol_value_union_field_converter<type::cpp_type<T, Tag>>
    : protocol_value_union_field_converter<Tag> {};

template <typename Elem>
struct protocol_value_union_field_converter<type::list<Elem>> {
  template <typename C>
  static protocol::Value convert(const protocol::Value&, const C& value) {
    return protocol_value_converter<type::list<Elem>>::convert(value);
  }
};

template <typename Elem>
struct protocol_value_union_field_converter<type::set<Elem>> {
  template <typename C>
  static protocol::Value convert(const protocol::Value&, const C& value) {
    return protocol_value_converter<type::set<Elem>>::convert(value);
  }
};

template <typename Key, typename Mapped>
struct protocol_value_union_field_converter<type::map<Key, Mapped>> {
  template <typename C>
  static protocol::Value convert(const protocol::Value&, const C& value) {
    return protocol_value_converter<type::map<Key, Mapped>>::convert(value);
  }
};

protocol::Value protocol_value_object(const protocol::Value& value) {
  const auto& thrift_value = value.toThrift();
  using ProtocolValue = std::remove_cvref_t<decltype(thrift_value)>;

  protocol::Value ret;
  auto& obj = ret.emplace_object();
  obj.type() = std::string{apache::thrift::uri<ProtocolValue>()};

  op::for_each_field_id<ProtocolValue>([&](auto field_id) {
    using Id = decltype(field_id);
    using Tag = op::get_type_tag<ProtocolValue, Id>;

    if (const auto* field = op::get_value_or_null(op::get<Id>(thrift_value))) {
      obj[FieldId{folly::to_underlying(Id::value)}] =
          protocol_value_union_field_converter<Tag>::convert(value, *field);
    }
  });
  return ret;
}

std::string type_uri_value(const TypeUri& uri) {
  if (const auto* value = op::get_value_or_null(uri.uri_ref())) {
    return {value->data(), value->size()};
  }
  if (const auto* value = op::get_value_or_null(uri.scopedName_ref())) {
    return {value->data(), value->size()};
  }
  const auto* value = op::get_value_or_null(uri.definitionKey_ref());
  assert(value != nullptr);
  return {value->data(), value->size()};
}

protocol::Value make_value(const t_const_value& const_value) {
  protocol::Value value;
  switch (const_value.kind()) {
    case t_const_value::CV_BOOL:
      value.emplace_bool(const_value.get_bool());
      break;
    case t_const_value::CV_INTEGER:
      value.emplace_i64(const_value.get_integer());
      break;
    case t_const_value::CV_DOUBLE:
      value.emplace_double(const_value.get_double());
      break;
    case t_const_value::CV_STRING:
    case t_const_value::CV_IDENTIFIER:
      value.emplace_string(const_value.get_string());
      break;
    case t_const_value::CV_MAP:
      value.emplace_map();
      break;
    case t_const_value::CV_LIST:
      value.emplace_list();
      break;
  }
  return value;
}

protocol::Value make_primitive_value(const t_type& ty) {
  protocol::Value value;
  if (const auto* primitive = ty.try_as<t_primitive_type>()) {
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_bool:
        value.emplace_bool();
        break;
      case t_primitive_type::type::t_byte:
        value.emplace_byte();
        break;
      case t_primitive_type::type::t_i16:
        value.emplace_i16();
        break;
      case t_primitive_type::type::t_i32:
        value.emplace_i32();
        break;
      case t_primitive_type::type::t_i64:
        value.emplace_i64();
        break;
      case t_primitive_type::type::t_float:
        value.emplace_float();
        break;
      case t_primitive_type::type::t_double:
        value.emplace_double();
        break;
      case t_primitive_type::type::t_string:
        value.emplace_string();
        break;
      case t_primitive_type::type::t_binary:
        value.emplace_binary();
        break;
      case t_primitive_type::type::t_void:
        break;
    }
  } else if (ty.is<t_list>()) {
    value.emplace_list();
  } else if (ty.is<t_set>()) {
    value.emplace_set();
  } else if (ty.is<t_map>()) {
    value.emplace_map();
  } else if (ty.is<t_enum>()) {
    value.emplace_i32();
  } else {
    abort();
  }
  return value;
}

} // namespace

TypeUri schema_populator::type_uri(const t_type& type) {
  return type_uri(type, opts().use_hash);
}

TypeUri schema_populator::type_uri(const t_named& node, const bool use_hash) {
  TypeUri ret;
  if (use_hash) {
    ret.definitionKey_ref() = schema_utils_.identify_definition(node);
  } else if (!node.uri().empty()) {
    ret.uri_ref() = node.uri();
  } else if (node.program()) {
    ret.scopedName_ref() = node.program()->scoped_name(node);
  } else {
    ret.scopedName_ref() = node.name();
  }
  return ret;
}

type::DefinitionAttrs schema_populator::gen_attrs(
    const t_named& node, const t_program* program) {
  type::DefinitionAttrs attrs;
  attrs.name() = node.name();
  if (!node.uri().empty()) {
    attrs.uri() = node.uri();
  } else if (auto program_node = dynamic_cast<const t_program*>(&node)) {
    if (!program_node->package().empty()) {
      attrs.uri() = program_node->package().name();
    }
  }

  if (auto structured = node.structured_annotations();
      !structured.empty() && opts().include_annotations) {
    for (const auto& item : structured) {
      type::Annotation annot;
      const auto ty_wrapper = protocol_value_builder{*item.type()};

      if (!item.value()->is_empty()) {
        for (const auto& [key, value] : item.value()->get_map()) {
          annot.fields()[key->get_string()] =
              ty_wrapper.property(*key).wrap(*value);
        }
      }

      // Double write to deprecated externed path. (T161963504)
      if (opts().double_writes) {
        type::StructuredAnnotation structured_annotation;
        structured_annotation.type() = type_uri(*item.type(), opts().use_hash);
        structured_annotation.fields() = *annot.fields();
        auto id = intern_value_(
            concrete_protocol_value(structured_annotation),
            const_cast<t_program*>(program));
        attrs.structuredAnnotations()->insert(static_cast<type::ValueId>(id));
      }

      auto unhashed_uri = type_uri(*item.type(), false /*use_hash*/);
      // We're not hashing here, as annotations are stored by string identifier.
      attrs.annotations()[type_uri_value(unhashed_uri)] = annot;

      auto hashed_uri = type_uri(*item.type(), true /*use_hash*/);
      attrs.annotationsByKey()[type_uri_value(hashed_uri)] = std::move(annot);
    }
  }

  if (auto unstructured = node.unstructured_annotations();
      !unstructured.empty() && opts().include_annotations) {
    for (const auto& pair : unstructured) {
      attrs.unstructuredAnnotations()[pair.first] = pair.second.value;
    }
  }

  if (node.has_doc() && opts().include_docs) {
    attrs.docs()->contents() = node.doc();
  }

  return attrs;
}

Type schema_populator::gen_type(
    schema_populator* generator,
    const t_program* program,
    DefinitionList* defns_schema,
    const t_type& type) {
  TypeStruct schema;
  TypeName type_name;

  if (type.is<t_typedef>()) {
    type_name.typedefType_ref() = type_uri(type);
    schema.name() = std::move(type_name);
    return Type{std::move(schema)};
  }

  auto* resolved_type = &type;
  resolved_type->visit(
      [&](const t_primitive_type& primitive) {
        switch (primitive.primitive_type()) {
          case t_primitive_type::type::t_void:
            break;
          case t_primitive_type::type::t_bool:
            set_void(type_name.boolType_ref());
            break;
          case t_primitive_type::type::t_byte:
            set_void(type_name.byteType_ref());
            break;
          case t_primitive_type::type::t_i16:
            set_void(type_name.i16Type_ref());
            break;
          case t_primitive_type::type::t_i32:
            set_void(type_name.i32Type_ref());
            break;
          case t_primitive_type::type::t_i64:
            set_void(type_name.i64Type_ref());
            break;
          case t_primitive_type::type::t_double:
            set_void(type_name.doubleType_ref());
            break;
          case t_primitive_type::type::t_float:
            set_void(type_name.floatType_ref());
            break;
          case t_primitive_type::type::t_string:
            set_void(type_name.stringType_ref());
            break;
          case t_primitive_type::type::t_binary:
            set_void(type_name.binaryType_ref());
            break;
        }
      },
      [&](const t_list& list) {
        set_void(type_name.listType_ref());
        schema.params()->push_back(
            gen_type(generator, program, defns_schema, *list.elem_type())
                .toThrift());
      },
      [&](const t_set& set) {
        set_void(type_name.setType_ref());
        schema.params()->push_back(
            gen_type(generator, program, defns_schema, *set.elem_type())
                .toThrift());
      },
      [&](const t_map& map) {
        set_void(type_name.mapType_ref());
        schema.params()->push_back(
            gen_type(generator, program, defns_schema, *map.key_type())
                .toThrift());
        schema.params()->push_back(
            gen_type(generator, program, defns_schema, *map.val_type())
                .toThrift());
      },
      [&](const t_enum& enum_type) {
        if (defns_schema && generator) {
          defns_schema->push_back(
              as_definition(generator->gen_schema(enum_type)));
        }
        type_name.enumType_ref() = type_uri(*resolved_type);
      },
      [&](const t_struct& struct_type) {
        if (defns_schema && generator) {
          defns_schema->push_back(
              as_definition(generator->gen_schema(struct_type)));
        }
        type_name.structType_ref() = type_uri(*resolved_type);
      },
      [&](const t_union& union_type) {
        if (defns_schema && generator) {
          defns_schema->push_back(
              as_definition(generator->gen_schema(union_type)));
        }
        type_name.unionType_ref() = type_uri(*resolved_type);
      },
      [&](const t_exception& exception_type) {
        if (defns_schema && generator) {
          defns_schema->push_back(
              as_definition(generator->gen_schema(exception_type)));
        }
        type_name.exceptionType_ref() = type_uri(*resolved_type);
      },
      [&](const t_interaction&) { assert(false); },
      [&](const t_service&) { assert(false); },
      [&](const t_typedef&) {
        // This should not happen since we resolve typedefs above
        assert(false);
      });
  schema.name() = std::move(type_name);
  return Type{std::move(schema)};
}

void schematize_recursively(
    schema_populator* generator,
    const t_program* program,
    DefinitionList* defns_schema,
    const t_type& type) {
  if (type.is<t_typedef>()) {
    return;
  }

  type.visit(
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
        defns_schema->push_back(
            as_definition(generator->gen_schema(enum_type)));
      },
      [&](const t_struct& struct_type) {
        defns_schema->push_back(
            as_definition(generator->gen_schema(struct_type)));
      },
      [&](const t_union& union_type) {
        defns_schema->push_back(
            as_definition(generator->gen_schema(union_type)));
      },
      [&](const t_exception& exception_type) {
        defns_schema->push_back(
            as_definition(generator->gen_schema(exception_type)));
      },
      [&](const t_interaction&) { assert(false); },
      [&](const t_service&) { assert(false); },
      [&](const t_typedef&) {
        // This should not happen since we resolve typedefs above
        assert(false);
      });
}

type::Fields schema_populator::gen_fields(
    schema_populator* generator,
    const t_program* program,
    DefinitionList* defns_schema,
    node_list_view<const t_field> fields) {
  type::Fields fields_schema;

  for (const auto& field : fields) {
    auto& field_schema = fields_schema.emplace_back();
    field_schema.attrs() = gen_attrs(field, program);
    field_schema.id() = type::FieldId{field.id()};

    switch (field.qualifier()) {
      case t_field_qualifier::none:
        field_schema.qualifier() = FieldQualifier::Default;
        break;
      case t_field_qualifier::required:
        field_schema.qualifier() = FieldQualifier::Fill;
        break;
      case t_field_qualifier::optional:
        field_schema.qualifier() = FieldQualifier::Optional;
        break;
      case t_field_qualifier::terse:
        field_schema.qualifier() = FieldQualifier::Terse;
        break;
    }

    field_schema.type() =
        gen_type(generator, program, defns_schema, *field.type());
    if (auto deflt = field.default_value()) {
      assert(program);
      field_schema.customDefault() = static_cast<type::ValueId>(intern_value_(
          const_to_value(*deflt, field.type()->get_true_type()),
          const_cast<t_program*>(program)));
    }
  }

  return fields_schema;
}

type::Struct schema_populator::gen_schema(const t_struct& node) {
  type::Struct schema;
  schema.attrs() = gen_attrs(node, node.program());
  schema.fields() = gen_fields(this, node.program(), nullptr, node.fields());
  return schema;
}

type::Union schema_populator::gen_schema(const t_union& node) {
  type::Union schema;
  schema.attrs() = gen_attrs(node, node.program());
  schema.fields() = gen_fields(this, node.program(), nullptr, node.fields());
  return schema;
}

type::Exception schema_populator::gen_schema(const t_exception& node) {
  type::Exception schema;
  schema.attrs() = gen_attrs(node, node.program());
  schema.fields() = gen_fields(this, node.program(), nullptr, node.fields());
  schema.safety() =
      static_cast<type::ErrorSafety>(as_underlying(node.safety()));
  schema.kind() = static_cast<type::ErrorKind>(as_underlying(node.kind()));
  schema.blame() = static_cast<type::ErrorBlame>(as_underlying(node.blame()));
  return schema;
}

type::Schema schema_populator::gen_full_schema(const t_service& node) {
  type::Schema schema;
  auto& defns_schema = *schema.definitions();

  defns_schema.push_back(as_definition(gen_schema(node)));

  for (const auto& func : node.functions()) {
    const t_type_ref& ret = func.return_type();
    // TODO: Handle sink, stream, interactions
    if (!func.sink_or_stream()) {
      schematize_recursively(
          this, node.program(), &defns_schema, *ret->get_true_type());
    }

    for (const auto& field : func.params().fields()) {
      schematize_recursively(
          this, node.program(), &defns_schema, *field.type());
    }

    if (func.exceptions()) {
      for (const auto& field : func.exceptions()->fields()) {
        schematize_recursively(
            this, node.program(), &defns_schema, *field.type());
      }
    }
  }

  if (auto parent = node.extends()) {
    defns_schema.push_back(as_definition(gen_schema(*parent)));
  }

  return schema;
}

type::Functions schema_populator::gen_functions(const t_interface& node) {
  type::Functions functions_schema;

  for (const auto& func : node.functions()) {
    auto& func_schema = functions_schema.emplace_back();
    func_schema.attrs() = gen_attrs(func, node.program());

    switch (func.qualifier()) {
      case t_function_qualifier::none:
        func_schema.qualifier() = FunctionQualifier::Unspecified;
        break;
      case t_function_qualifier::oneway:
        func_schema.qualifier() = FunctionQualifier::OneWay;
        break;
      case t_function_qualifier::idempotent:
        func_schema.qualifier() = FunctionQualifier::Idempotent;
        break;
      case t_function_qualifier::readonly:
        func_schema.qualifier() = FunctionQualifier::ReadOnly;
        break;
    }

    if (const auto& interaction = func.interaction()) {
      func_schema.interactionType()->uri() = type_uri(*interaction);
    }
    if (func.is_interaction_constructor()) {
      func_schema.isPerforms() = true;
    }
    if (!func.has_void_initial_response() ||
        (!func.sink_or_stream() && !func.interaction())) {
      const t_type* type = &func.return_type().deref();
      func_schema.returnType() = gen_type(*type, node.program());
      // Double write of return type for backwards compatibility (T161963504).
      if (opts().double_writes) {
        auto& schema = func_schema.returnTypes()->emplace_back();
        schema.thriftType_ref() = gen_type(*type, node.program());
      }
    }

    if (func.is_bidirectional_stream()) {
      type::BidirectionalStream bidi_schema;
      auto bidi_stream = func.stream();
      auto bidi_sink = func.sink();
      bidi_schema.streamPayload() =
          gen_type(*bidi_stream->elem_type(), node.program());
      bidi_schema.sinkPayload() =
          gen_type(*bidi_sink->elem_type(), node.program());
      if (auto throws = bidi_stream->exceptions()) {
        bidi_schema.streamExceptions() =
            gen_fields(this, node.program(), nullptr, throws->fields());
      }
      if (auto throws = bidi_sink->sink_exceptions()) {
        bidi_schema.sinkExceptions() =
            gen_fields(this, node.program(), nullptr, throws->fields());
      }
      func_schema.streamOrSink()->bidirectionalStream_ref() =
          std::move(bidi_schema);
    } else if (auto stream = func.stream()) {
      type::Stream stream_schema;
      stream_schema.payload() = gen_type(*stream->elem_type(), node.program());
      if (auto throws = stream->exceptions()) {
        stream_schema.exceptions() =
            gen_fields(this, node.program(), nullptr, throws->fields());
      }
      func_schema.streamOrSink()->streamType_ref() = std::move(stream_schema);
    } else if (auto sink = func.sink()) {
      type::Sink sink_schema;
      sink_schema.payload() = gen_type(*sink->elem_type(), node.program());
      if (auto throws = sink->sink_exceptions()) {
        sink_schema.clientExceptions() =
            gen_fields(this, node.program(), nullptr, throws->fields());
      }
      if (sink->final_response_type()) {
        sink_schema.finalResponse() =
            gen_type(*sink->final_response_type(), node.program());
      }
      if (auto throws = sink->final_response_exceptions()) {
        sink_schema.serverExceptions() =
            gen_fields(this, node.program(), nullptr, throws->fields());
      }
      func_schema.streamOrSink()->sinkType_ref() = std::move(sink_schema);
    }

    func_schema.paramlist()->fields() =
        gen_fields(this, node.program(), nullptr, func.params().fields());

    if (func.exceptions()) {
      func_schema.exceptions() = gen_fields(
          this, node.program(), nullptr, func.exceptions()->fields());
    }
  }

  return functions_schema;
}

type::Interaction schema_populator::gen_schema(const t_interaction& node) {
  type::Interaction schema;
  schema.attrs() = gen_attrs(node, node.program());
  schema.functions() = gen_functions(node);
  return schema;
}

type::Service schema_populator::gen_schema(const t_service& node) {
  type::Service schema;
  schema.attrs() = gen_attrs(node, node.program());
  schema.functions() = gen_functions(node);
  if (auto parent = node.extends()) {
    schema.baseService()->uri() = type_uri(*parent);
  }
  return schema;
}

type::Const schema_populator::gen_schema(const t_const& node) {
  const auto* program = node.program();
  assert(program);

  type::Const schema;
  schema.attrs() = gen_attrs(node, program);
  schema.type() = gen_type(*node.type(), program);

  schema.value() = static_cast<type::ValueId>(intern_value_(
      const_to_value(*node.value(), node.type()->get_true_type()),
      const_cast<t_program*>(program)));

  return schema;
}

type::Enum schema_populator::gen_schema(const t_enum& node) {
  type::Enum schema;
  schema.attrs() = gen_attrs(node, node.program());

  for (const auto& value : node.values()) {
    auto& value_schema = schema.values()->emplace_back();
    value_schema.attrs() = gen_attrs(value, node.program());
    value_schema.value() = value.get_value();
  }

  return schema;
}

type::Program schema_populator::gen_schema(const t_program& node) {
  type::Program schema;
  schema.attrs() = gen_attrs(node, &node);
  schema.path() = node.path();

  for (const auto& [lang, incs] : node.language_includes()) {
    auto& includes = schema.languageIncludes()[lang];
    includes.insert(includes.end(), incs.begin(), incs.end());
  }

  for (const auto& [lang, langNamespace] : node.namespaces()) {
    schema.namespaces()[lang] = langNamespace->ns();
  }

  // The remaining fields are intern IDs and have to be stiched in by the
  // caller.

  return schema;
}

type::Typedef schema_populator::gen_schema(const t_typedef& node) {
  type::Typedef schema;
  schema.attrs() = gen_attrs(node, node.program());
  schema.type() = gen_type(*node.type(), node.program());
  return schema;
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
        return protocol_value_builder{*field->type()};
      },
      [&](const t_union& union_) {
        assert(
            key.kind() == t_const_value::CV_STRING &&
            "A union only has named fields");
        const auto* field = union_.get_field_by_name(key.get_string());
        return protocol_value_builder{*field->type()};
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

protocol::Value protocol_value_builder::to_labeled_value(
    const t_const_value& protocol_value) const {
  if (ty_ == nullptr) {
    return make_value(protocol_value);
  }

  auto value = make_primitive_value(*ty_);
  auto raise_exception = [&] {
    throw std::runtime_error(
        fmt::format(
            "Could not match value of kind {} to type {}",
            t_const_value::kind_to_string(protocol_value.kind()),
            ty_->get_full_name()));
  };

  switch (protocol_value.kind()) {
    case t_const_value::CV_INTEGER:
      if (!(ty_->is_any_int() || ty_->is<t_enum>() ||
            ty_->is_floating_point())) {
        raise_exception();
      }
      if (value.is_byte()) {
        value.as_byte() = protocol_value.get_integer();
      } else if (value.is_i16()) {
        value.as_i16() = protocol_value.get_integer();
      } else if (value.is_i32()) {
        value.as_i32() = protocol_value.get_integer();
      } else if (value.is_i64()) {
        value.as_i64() = protocol_value.get_integer();
      } else if (value.is_float()) {
        value.as_float() = protocol_value.get_integer();
      } else if (value.is_double()) {
        value.as_double() = protocol_value.get_integer();
      }
      break;
    case t_const_value::CV_DOUBLE:
      if (!ty_->is_floating_point()) {
        raise_exception();
      }
      if (value.is_float()) {
        value.as_float() = protocol_value.get_double();
      } else {
        value.as_double() = protocol_value.get_double();
      }
      break;
    case t_const_value::CV_STRING:
      if (!ty_->is_string_or_binary()) {
        raise_exception();
      }
      if (value.is_string()) {
        value.as_string() = protocol_value.get_string();
      } else {
        value.as_binary() = folly::IOBuf(
            folly::IOBuf::CopyBufferOp{}, protocol_value.get_string());
      }
      break;
    case t_const_value::CV_BOOL:
      value.as_bool() = protocol_value.get_bool();
      break;
    case t_const_value::CV_MAP:
    case t_const_value::CV_LIST:
    case t_const_value::CV_IDENTIFIER:
      break;
  }
  return value;
}

protocol::Value protocol_value_builder::wrap(
    const t_const_value& protocol_value) const {
  switch (protocol_value.kind()) {
    case t_const_value::CV_BOOL:
    case t_const_value::CV_INTEGER:
    case t_const_value::CV_DOUBLE:
    case t_const_value::CV_STRING:
      return to_labeled_value(protocol_value);
    case t_const_value::CV_MAP: {
      protocol::Value ret;
      auto& map = ret.emplace_map();
      for (const auto& [k, v] : protocol_value.get_map()) {
        map.emplace(key(*k).wrap(*k), property(*k).wrap(*v));
      }
      return ret;
    }
    case t_const_value::CV_LIST: {
      protocol::Value ret;
      auto list_ty_resolver = container_element(protocol_value);
      if (ty_ != nullptr && ty_->is<t_set>()) {
        auto& set = ret.emplace_set();
        for (const auto& list_elem : protocol_value.get_list()) {
          set.insert(list_ty_resolver.wrap(*list_elem));
        }
      } else {
        auto& list = ret.emplace_list();
        for (const auto& list_elem : protocol_value.get_list()) {
          list.push_back(list_ty_resolver.wrap(*list_elem));
        }
      }
      return ret;
    }
    case t_const_value::CV_IDENTIFIER:
      return to_labeled_value(protocol_value);
  }
  abort();
}

} // namespace apache::thrift::compiler::detail
