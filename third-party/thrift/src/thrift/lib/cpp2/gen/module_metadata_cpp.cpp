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

#include <thrift/lib/cpp2/gen/module_metadata_cpp.h>

#include <fmt/format.h>

namespace apache::thrift::detail::md {

ThriftConstValue cvBool(bool value) {
  ThriftConstValue ret;
  ret.cv_bool() = value;
  return ret;
}

ThriftConstValue cvInteger(int64_t value) {
  ThriftConstValue ret;
  ret.cv_integer() = value;
  return ret;
}

ThriftConstValue cvDouble(double value) {
  ThriftConstValue ret;
  ret.cv_double() = value;
  return ret;
}

ThriftConstValue cvString(const char* value) {
  ThriftConstValue ret;
  ret.cv_string() = value;
  return ret;
}

ThriftConstValue cvMap(std::vector<ThriftConstValuePair>&& value) {
  ThriftConstValue ret;
  ret.cv_map() = std::move(value);
  return ret;
}

ThriftConstValue cvList(std::initializer_list<ThriftConstValue> value) {
  ThriftConstValue ret;
  ret.cv_list().emplace().assign(
      std::make_move_iterator(value.begin()),
      std::make_move_iterator(value.end()));
  return ret;
}

ThriftConstValue cvStruct(
    const char* name, std::map<std::string, ThriftConstValue>&& fields) {
  ThriftConstValue ret;
  ThriftConstStruct s;
  s.type()->name() = name;
  s.fields() = std::move(fields);
  ret.cv_struct() = std::move(s);
  return ret;
}

ThriftConstValuePair cvPair(ThriftConstValue&& key, ThriftConstValue&& value) {
  ThriftConstValuePair pair;
  pair.key() = std::move(key);
  pair.value() = std::move(value);
  return pair;
}

std::mutex& schemaRegistryMutex() {
  static std::mutex mutex;
  return mutex;
}

template <class Node>
static std::string getName(const Node& node) {
  std::lock_guard lock(schemaRegistryMutex());
  const auto& def = node.definition();
  return fmt::format("{}.{}", def.program().name(), def.name());
}

namespace {
// Helper functions to convert
// syntax_graph::Annotation --> metadata::ThriftConstStruct
//
// Considering
//
//   struct Bar { 1: string baz; }
//   struct Foo { 1: Bar bar; }
//   @Foo{bar = Bar{baz = "123"}}
//   struct MyStruct {};
//
// In this case, SyntaxGraph::Annotation will have Type:
//
//   StructNode 'Foo'
//   ╰─ FieldNode(id = 1, presence = UNQUALIFIED, name = 'bar')
//      ╰─ type = StructNode 'Bar'
//         ╰─ FieldNode(id = 1, presence = UNQUALIFIED, name = 'baz')
//            ╰─ type = STRING
//
// With value:
//
//   {"bar": {"baz": "123"}}
//
// On the other hand, metadata::ThriftConstStruct will have value
//
//   <Struct: ThriftConstStruct (metadata.thrift)>
//   ├─ type
//   │  ╰─ <Struct: ThriftStructType (metadata.thrift)>
//   │     ╰─ name
//   │        ╰─ annotations.Foo
//   ╰─ fields
//      ╰─ <Map>
//         ├─ Key #0
//         │  ╰─ bar
//         ╰─ Value #0
//            ╰─ <Union: ThriftConstValue (metadata.thrift)>
//               ╰─ cv_struct
//                  ╰─ <Struct: ThriftConstStruct (metadata.thrift)>
//                     ├─ type
//                     │  ╰─ <Struct: ThriftStructType (metadata.thrift)>
//                     │     ╰─ name
//                     │        ╰─ annotations.Bar
//                     ╰─ fields
//                        ╰─ <Map>
//                           ├─ Key #0
//                           │  ╰─ baz
//                           ╰─ Value #0
//                              ╰─ <Union: ThriftConstValue (metadata.thrift)>
//                                 ╰─ cv_string
//                                    ╰─ 123
class AnnotationConverter {
 public:
  static ThriftConstStruct convert(const syntax_graph::Annotation&);

 private:
  static metadata::detail::LimitedVector<ThriftConstValue> convertListOrSet(
      const syntax_graph::TypeRef& element, const folly::dynamic&);
  static std::vector<ThriftConstValuePair> convertMap(
      const syntax_graph::Map&, const folly::dynamic&);
  static ThriftConstStruct convertStructured(
      const syntax_graph::StructuredNode&, const folly::dynamic&);

  static ThriftConstValue convertValue(
      const syntax_graph::TypeRef&, const folly::dynamic&);
  static ThriftConstValue convertMapKey(
      const syntax_graph::TypeRef&, std::string_view);
};

ThriftConstStruct AnnotationConverter::convert(
    const syntax_graph::Annotation& annotation) {
  return convertStructured(
      annotation.type().trueType().asStructured(), annotation.value());
}

ThriftConstStruct AnnotationConverter::convertStructured(
    const syntax_graph::StructuredNode& node, const folly::dynamic& dynamic) {
  ThriftConstStruct ret;
  ret.type().ensure().name() = getName(node);

  if (dynamic.empty()) {
    return ret;
  }

  for (const auto& field : node.fields()) {
    auto iter = dynamic.find(field.name());
    if (iter != dynamic.items().end()) {
      ret.fields()[std::string(field.name())] =
          convertValue(field.type(), iter->second);
    }
  }
  return ret;
}
ThriftConstValue AnnotationConverter::convertValue(
    const syntax_graph::TypeRef& refInput, const folly::dynamic& dynamic) {
  const auto& ref = refInput.trueType();
  ThriftConstValue ret;

  if (dynamic.isBool()) {
    ret.cv_bool() = dynamic.asBool();
    return ret;
  }

  if (dynamic.isInt()) {
    ret.cv_integer() = dynamic.asInt();
    return ret;
  }

  if (dynamic.isDouble()) {
    ret.cv_double() = dynamic.asDouble();
    return ret;
  }

  if (dynamic.isString()) {
    ret.cv_string() = dynamic.asString();
    return ret;
  }

  if (ref.isList()) {
    ret.cv_list() = convertListOrSet(ref.asList().elementType(), dynamic);
    return ret;
  }

  if (ref.isSet()) {
    ret.cv_list() = convertListOrSet(ref.asSet().elementType(), dynamic);
    return ret;
  }

  if (ref.isMap()) {
    ret.cv_map() = convertMap(ref.asMap(), dynamic);
    return ret;
  }

  if (ref.isStructured()) {
    ret.cv_struct() = convertStructured(ref.asStructured(), dynamic);
    return ret;
  }

  folly::throw_exception_fmt_format<std::logic_error>(
      "Mismatched annotation type and value");
}

metadata::detail::LimitedVector<ThriftConstValue>
AnnotationConverter::convertListOrSet(
    const syntax_graph::TypeRef& element, const folly::dynamic& dynamic) {
  metadata::detail::LimitedVector<ThriftConstValue> ret;
  for (const auto& i : dynamic) {
    ret.push_back(convertValue(element, i));
  }
  return ret;
}
std::vector<ThriftConstValuePair> AnnotationConverter::convertMap(
    const syntax_graph::Map& map, const folly::dynamic& dynamic) {
  std::vector<ThriftConstValuePair> ret;
  for (const auto& [k, v] : dynamic.items()) {
    ThriftConstValuePair pair;
    pair.key() = convertMapKey(map.keyType().trueType(), k.asString());
    pair.value() = convertValue(map.valueType().trueType(), v);
    ret.push_back(std::move(pair));
  }
  return ret;
}
ThriftConstValue AnnotationConverter::convertMapKey(
    const syntax_graph::TypeRef& ref, std::string_view s) {
  ThriftConstValue ret;
  if (ref.isEnum()) {
    // Enum is encoded as i32.
    ret.cv_integer() = folly::to<std::int64_t>(s);
    return ret;
  }
  switch (ref.asPrimitive()) {
    case syntax_graph::Primitive::BOOL:
      ret.cv_bool() = folly::to<bool>(s);
      break;
    case syntax_graph::Primitive::BYTE:
    case syntax_graph::Primitive::I16:
    case syntax_graph::Primitive::I32:
    case syntax_graph::Primitive::I64:
      ret.cv_integer() = folly::to<std::int64_t>(s);
      break;
    case syntax_graph::Primitive::FLOAT:
    case syntax_graph::Primitive::DOUBLE:
      ret.cv_double() = folly::to<double>(s);
      break;
    case syntax_graph::Primitive::STRING:
    case syntax_graph::Primitive::BINARY:
      ret.cv_string() = s;
      break;
  }
  return ret;
}

metadata::detail::LimitedVector<ThriftConstStruct> genStructuredAnnotations(
    folly::span<const syntax_graph::Annotation> annotations) {
  metadata::detail::LimitedVector<ThriftConstStruct> ret;
  for (const auto& i : annotations) {
    ret.push_back(AnnotationConverter::convert(i));
  }
  return ret;
}
} // namespace

GenMetadataResult<metadata::ThriftEnum> genEnumMetadata(
    metadata::ThriftMetadata& md,
    const syntax_graph::EnumNode& node,
    Options options) {
  auto name = getName(node);
  auto res = md.enums()->try_emplace(name);
  GenMetadataResult<metadata::ThriftEnum> ret{!res.second, res.first->second};
  if (ret.preExists) {
    return ret;
  }
  ret.metadata.name() = std::move(name);
  for (const auto& value : node.values()) {
    ret.metadata.elements()[value.i32()] = value.name();
  }
  if (options.genAnnotations) {
    ret.metadata.structured_annotations() =
        genStructuredAnnotations(node.definition().annotations());
  }
  return ret;
}

namespace {
metadata::ThriftPrimitiveType toThriftPrimitiveType(
    syntax_graph::Primitive type) {
  switch (type) {
    case syntax_graph::Primitive::BOOL:
      return metadata::ThriftPrimitiveType::THRIFT_BOOL_TYPE;
    case syntax_graph::Primitive::BYTE:
      return metadata::ThriftPrimitiveType::THRIFT_BYTE_TYPE;
    case syntax_graph::Primitive::I16:
      return metadata::ThriftPrimitiveType::THRIFT_I16_TYPE;
    case syntax_graph::Primitive::I32:
      return metadata::ThriftPrimitiveType::THRIFT_I32_TYPE;
    case syntax_graph::Primitive::I64:
      return metadata::ThriftPrimitiveType::THRIFT_I64_TYPE;
    case syntax_graph::Primitive::FLOAT:
      return metadata::ThriftPrimitiveType::THRIFT_FLOAT_TYPE;
    case syntax_graph::Primitive::DOUBLE:
      return metadata::ThriftPrimitiveType::THRIFT_DOUBLE_TYPE;
    case syntax_graph::Primitive::STRING:
      return metadata::ThriftPrimitiveType::THRIFT_STRING_TYPE;
    case syntax_graph::Primitive::BINARY:
      return metadata::ThriftPrimitiveType::THRIFT_BINARY_TYPE;
  }
  throw std::logic_error(
      fmt::format("unknown primitive type: {}", folly::to_underlying(type)));
}

// For struct/union/exception/services, we need to generate metadata for nested
// types.
ThriftType genType(
    metadata::ThriftMetadata& md, const syntax_graph::TypeRef& type) {
  ThriftType ret;
  Options options = {.genAnnotations = true, .genNestedTypes = true};
  switch (type.kind()) {
    case syntax_graph::TypeRef::Kind::PRIMITIVE: {
      ret.t_primitive() = toThriftPrimitiveType(type.asPrimitive());
      return ret;
    }
    case syntax_graph::TypeRef::Kind::STRUCT: {
      auto res = genStructMetadata(md, type.asStructured(), options);
      ret.t_struct().emplace().name() = *res.metadata.name();
      return ret;
    }
    case syntax_graph::TypeRef::Kind::UNION: {
      auto res = genStructMetadata(md, type.asStructured(), options);
      ret.t_union().emplace().name() = *res.metadata.name();
      return ret;
    }
    case syntax_graph::TypeRef::Kind::EXCEPTION: {
      // Note: exception is considered t_struct in metadata's Type system.
      // There is no t_exception in metadata::ThriftType.
      auto res = genStructMetadata(md, type.asStructured(), options);
      ret.t_struct().emplace().name() = *res.metadata.name();
      return ret;
    }
    case syntax_graph::TypeRef::Kind::ENUM: {
      auto res = genEnumMetadata(md, type.asEnum(), options);
      ret.t_enum().emplace().name() = *res.metadata.name();
      return ret;
    }
    case syntax_graph::TypeRef::Kind::TYPEDEF: {
      const auto& node = type.asTypedef();
      ret.t_typedef().emplace();
      ret.t_typedef()->name() = getName(node);
      ret.t_typedef()->underlyingType() =
          std::make_unique<ThriftType>(genType(md, node.targetType()));
      ret.t_typedef()->structured_annotations() =
          genStructuredAnnotations(node.definition().annotations());
      return ret;
    }
    case syntax_graph::TypeRef::Kind::LIST: {
      ret.t_list().emplace().valueType() = std::make_unique<ThriftType>(
          genType(md, type.asList().elementType()));
      return ret;
    }
    case syntax_graph::TypeRef::Kind::SET: {
      ret.t_set().emplace().valueType() =
          std::make_unique<ThriftType>(genType(md, type.asSet().elementType()));
      return ret;
    }
    case syntax_graph::TypeRef::Kind::MAP: {
      ret.t_map().emplace();
      ret.t_map()->keyType() =
          std::make_unique<ThriftType>(genType(md, type.asMap().keyType()));
      ret.t_map()->valueType() =
          std::make_unique<ThriftType>(genType(md, type.asMap().valueType()));
      return ret;
    }
  }

  throw std::logic_error(
      fmt::format("unknown type kind: {}", folly::to_underlying(type.kind())));
}
} // namespace

template <class Metadata>
static auto genStructuredInMetadataMap(
    metadata::ThriftMetadata& md,
    std::map<std::string, Metadata>& metadataMap,
    const syntax_graph::StructuredNode& node,
    Options options) {
  auto name = getName(node);
  auto res = metadataMap.try_emplace(name);
  GenMetadataResult<Metadata> ret{!res.second, res.first->second};
  if (ret.preExists) {
    return ret;
  }
  ret.metadata.name() = std::move(name);
  if constexpr (!std::is_same_v<Metadata, metadata::ThriftException>) {
    ret.metadata.is_union() = node.definition().isUnion();
  }
  ret.metadata.fields()->reserve(node.fields().size());
  for (auto& field : node.fields()) {
    auto& f = ret.metadata.fields()->emplace_back();
    f.id() = folly::to_underlying(field.id());
    f.name() = field.name();
    f.is_optional() =
        (field.presence() == syntax_graph::FieldPresenceQualifier::OPTIONAL_);
    if (options.genAnnotations) {
      f.structured_annotations() =
          genStructuredAnnotations(field.annotations());
    }
    if (options.genNestedTypes) {
      f.type() = genType(md, field.type());
    }
  }
  if (options.genAnnotations) {
    ret.metadata.structured_annotations() =
        genStructuredAnnotations(node.definition().annotations());
  }
  return ret;
}

GenMetadataResult<metadata::ThriftStruct> genStructMetadata(
    metadata::ThriftMetadata& md,
    const syntax_graph::StructuredNode& node,
    Options options) {
  return genStructuredInMetadataMap(md, *md.structs(), node, options);
}

void genStructFieldMetadata(
    const syntax_graph::StructuredNode& node,
    metadata::ThriftField& field,
    const EncodedThriftField& f,
    size_t index) {
  DCHECK_EQ(*field.id(), f.id);
  DCHECK_EQ(*field.name(), f.name);
  DCHECK_EQ(*field.is_optional(), f.is_optional);

  auto newAnnotations = std::move(*field.structured_annotations());
  field.structured_annotations().emplace().assign(
      f.structured_annotations.begin(), f.structured_annotations.end());

  DCHECK(structuredAnnotationsEquality(
      *field.structured_annotations(),
      newAnnotations,
      getFieldAnnotationTypes(node, index, static_cast<std::int16_t>(f.id))));
}

GenMetadataResult<metadata::ThriftException> genExceptionMetadata(
    metadata::ThriftMetadata& md,
    const syntax_graph::ExceptionNode& node,
    Options options) {
  return genStructuredInMetadataMap(md, *md.exceptions(), node, options);
}

metadata::ThriftService genServiceMetadata(
    const syntax_graph::ServiceNode& node,
    metadata::ThriftMetadata& md,
    Options options) {
  metadata::ThriftService ret;
  ret.name() = getName(node);
  ret.uri() = node.uri();
  if (const auto* p = node.baseService()) {
    ret.parent() = getName(*p);
  }
  for (const auto& func : node.functions()) {
    if (func.isPerforms()) {
      continue;
    }
    ret.functions()->emplace_back().name() = func.name();
    if (options.genNestedTypes) {
      if (auto stream = func.response().stream()) {
        ret.functions()->back().return_type()->t_stream().emplace();
        ret.functions()->back().return_type()->t_stream()->elemType() =
            std::make_unique<ThriftType>(genType(md, stream->payloadType()));
        if (auto retType = func.response().type()) {
          ret.functions()
              ->back()
              .return_type()
              ->t_stream()
              ->initialResponseType() =
              std::make_unique<ThriftType>(genType(md, *retType));
        }
      } else if (auto sink = func.response().sink()) {
        ret.functions()->back().return_type()->t_sink().emplace();
        ret.functions()->back().return_type()->t_sink()->elemType() =
            std::make_unique<ThriftType>(genType(md, sink->payloadType()));
        ret.functions()->back().return_type()->t_sink()->finalResponseType() =
            std::make_unique<ThriftType>(
                genType(md, sink->finalResponseType()));
        if (auto retType = func.response().type()) {
          ret.functions()
              ->back()
              .return_type()
              ->t_sink()
              ->initialResponseType() =
              std::make_unique<ThriftType>(genType(md, *retType));
        }
      } else if (auto retType = func.response().type()) {
        ret.functions()->back().return_type() = genType(md, *retType);
      } else {
        ret.functions()->back().return_type()->t_primitive() =
            metadata::ThriftPrimitiveType::THRIFT_VOID_TYPE;
      }
    }
    ret.functions()->back().is_oneway() =
        func.qualifier() == type::FunctionQualifier::OneWay;
    for (const auto& param : func.params()) {
      auto& i = ret.functions()->back().arguments()->emplace_back();
      i.id() = static_cast<std::int16_t>(param.id());
      i.name() = param.name();
      i.is_optional() = false;
      if (options.genAnnotations) {
        i.structured_annotations() =
            genStructuredAnnotations(param.annotations());
      }
      if (options.genNestedTypes) {
        i.type() = genType(md, param.type());
      }
    }
    for (const auto& exception : func.exceptions()) {
      auto& i = ret.functions()->back().exceptions()->emplace_back();
      i.id() = static_cast<std::int16_t>(exception.id());
      i.name() = exception.name();
      i.is_optional() = false;
      if (options.genAnnotations) {
        i.structured_annotations() =
            genStructuredAnnotations(exception.annotations());
      }
      if (options.genNestedTypes) {
        i.type() = genType(md, exception.type());
        if (exception.type().isStructured()) {
          // Mimicking the existing logic: we add all types in throw clause
          // into `exceptions` field as long as it's structured.
          // https://github.com/facebook/fbthrift/blob/v2025.11.03.00/thrift/compiler/generate/templates/cpp2/module_metadata.cpp.mustache#L153-L157
          genStructuredInMetadataMap(
              md, *md.exceptions(), exception.type().asStructured(), options);
        }
      }
    }
    if (options.genAnnotations) {
      ret.functions()->back().structured_annotations() =
          genStructuredAnnotations(func.annotations());
    }
  }
  if (options.genAnnotations) {
    ret.structured_annotations() =
        genStructuredAnnotations(node.definition().annotations());
  }
  return ret;
}

std::vector<syntax_graph::TypeRef> getAnnotationTypes(
    folly::span<const syntax_graph::Annotation> annotations) {
  std::vector<syntax_graph::TypeRef> ret;
  ret.reserve(annotations.size());
  for (auto& annotation : annotations) {
    ret.push_back(annotation.type());
  }
  return ret;
}

std::vector<syntax_graph::TypeRef> getFieldAnnotationTypes(
    const syntax_graph::StructuredNode& node,
    size_t position,
    std::int16_t id) {
  DCHECK_LT(position, node.fields().size());
  const auto& field = node.fields()[position];
  DCHECK_EQ(static_cast<std::int16_t>(field.id()), id);
  return getAnnotationTypes(field.annotations());
}

namespace {
// In ThriftConstValue, `set`/`map` are stored as list.
// This function sort `set`/`map` so that we can do equality comparison.
void normalizeThriftConstValue(
    ThriftConstValue& t, const syntax_graph::TypeRef& type);

void normalizeThriftConstStruct(
    ThriftConstStruct& t, const syntax_graph::TypeRef& type) {
  std::unordered_map<std::string, syntax_graph::TypeRef> fieldType;
  for (auto& field : type.trueType().asStructured().fields()) {
    fieldType.emplace(field.name(), field.type());
  }
  for (auto& [name, value] : *t.fields()) {
    normalizeThriftConstValue(value, fieldType.at(name));
  }
}
void normalizeThriftConstValue(
    ThriftConstValue& t, const syntax_graph::TypeRef& ref) {
  const auto& type = ref.trueType();
  if (type.isList()) {
    for (auto& i : *t.cv_list()) {
      normalizeThriftConstValue(i, type.asList().elementType());
    }
  }

  if (type.isSet()) {
    for (auto& i : *t.cv_list()) {
      normalizeThriftConstValue(i, type.asSet().elementType());
    }
    std::sort(t.cv_list()->begin(), t.cv_list()->end());
  }

  if (type.isMap()) {
    auto keyType = type.asMap().keyType();
    auto valueType = type.asMap().valueType();
    for (auto& i : *t.cv_map()) {
      normalizeThriftConstValue(*i.key(), keyType);
      normalizeThriftConstValue(*i.value(), valueType);
    }
    std::sort(t.cv_map()->begin(), t.cv_map()->end());
  }

  if (type.isStructured()) {
    normalizeThriftConstStruct(*t.cv_struct(), type);
  }
}

// This function will sort structured annotations, as well as sorting
// `set`/`map` inside annotations so that we can do equality comparison.
std::vector<ThriftConstStruct> normalizeStructuredAnnotations(
    std::vector<ThriftConstStruct> annotations,
    const std::unordered_map<std::string, syntax_graph::TypeRef>& nameToType) {
  for (auto& i : annotations) {
    normalizeThriftConstStruct(i, nameToType.at(*i.type()->name()));
  }
  std::sort(annotations.begin(), annotations.end());
  return annotations;
}
} // namespace

bool structuredAnnotationsEquality(
    std::vector<ThriftConstStruct> lhsAnnotations,
    std::vector<ThriftConstStruct> rhsAnnotations,
    const std::vector<syntax_graph::TypeRef>& annotationTypes) {
  std::unordered_map<std::string, syntax_graph::TypeRef> nameToType;
  for (const auto& i : annotationTypes) {
    nameToType.emplace(getName(i.trueType().asStructured()), i);
  }
  return normalizeStructuredAnnotations(
             std::move(lhsAnnotations), nameToType) ==
      normalizeStructuredAnnotations(std::move(rhsAnnotations), nameToType);
}

const ThriftServiceContextRef* genServiceMetadataRecurse(
    const syntax_graph::ServiceNode& node,
    ThriftMetadata& metadata,
    std::vector<ThriftServiceContextRef>& services) {
  Options options = {.genAnnotations = true, .genNestedTypes = true};
  auto serviceMetadata = genServiceMetadata(node, metadata, options);
  // We need to keep the index around because a reference or iterator could be
  // invalidated.
  auto selfIndex = services.size();
  services.emplace_back();
  if (auto base = node.baseService()) {
    genServiceMetadataRecurse(*base, metadata, services);
  }
  ThriftServiceContextRef& context = services[selfIndex];
  auto name = *serviceMetadata.name();
  metadata.services()->emplace(name, std::move(serviceMetadata));
  context.service_name() = std::move(name);
  metadata::ThriftModuleContext module;
  module.name() = node.definition().program().name();
  context.module() = std::move(module);
  return &context;
}

void genServiceMetadataResponse(
    const syntax_graph::ServiceNode& node,
    metadata::ThriftServiceMetadataResponse& response) {
  const ::apache::thrift::metadata::ThriftServiceContextRef* self =
      genServiceMetadataRecurse(
          node, *response.metadata(), *response.services());
  metadata::ThriftServiceContext context;
  // TODO(praihan): Remove ThriftServiceContext from response. But in the
  // meantime, we need to fill the field with the result of looking up in
  // ThriftMetadata.
  context.module() = *self->module();
  context.service_info() =
      response.metadata()->services()->at(*self->service_name());
  response.context() = std::move(context);
}
} // namespace apache::thrift::detail::md
