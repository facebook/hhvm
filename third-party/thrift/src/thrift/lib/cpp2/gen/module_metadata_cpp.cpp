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
    pair.key() = convertMapKey(map.keyType(), k.asString());
    pair.value() = convertValue(map.valueType(), v);
    ret.push_back(std::move(pair));
  }
  return ret;
}
ThriftConstValue AnnotationConverter::convertMapKey(
    const syntax_graph::TypeRef& ref, std::string_view s) {
  ThriftConstValue ret;
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
    bool genAnnotations) {
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
  if (genAnnotations) {
    ret.metadata.structured_annotations() =
        genStructuredAnnotations(node.definition().annotations());
  }
  return ret;
}

template <class Metadata>
static auto genStructuredInMetadataMap(
    const syntax_graph::StructuredNode& node,
    std::map<std::string, Metadata>& metadataMap,
    bool genAnnotations) {
  auto name = getName(node);
  auto res = metadataMap.try_emplace(name);
  GenMetadataResult<Metadata> ret{!res.second, res.first->second};
  if (ret.preExists) {
    return ret;
  }
  ret.metadata.name() = std::move(name);
  ret.metadata.fields()->reserve(node.fields().size());
  for (auto& field : node.fields()) {
    auto& f = ret.metadata.fields()->emplace_back();
    f.id() = folly::to_underlying(field.id());
    f.name() = field.name();
    f.is_optional() =
        (field.presence() == syntax_graph::FieldPresenceQualifier::OPTIONAL_);
    if (genAnnotations) {
      f.structured_annotations() =
          genStructuredAnnotations(field.annotations());
    }
  }
  // TODO: add other information
  return ret;
}

GenMetadataResult<metadata::ThriftStruct> genStructMetadata(
    metadata::ThriftMetadata& md,
    const syntax_graph::StructuredNode& node,
    bool genAnnotations) {
  return genStructuredInMetadataMap(node, *md.structs(), genAnnotations);
}

GenMetadataResult<metadata::ThriftException> genExceptionMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::ExceptionNode& node) {
  return genStructuredInMetadataMap(node, *md.exceptions(), false);
}

metadata::ThriftService genServiceMetadata(
    const syntax_graph::ServiceNode& node) {
  metadata::ThriftService ret;
  ret.name() = getName(node);
  ret.uri() = node.uri();
  if (const auto* p = node.baseService()) {
    ret.parent() = getName(*p);
  }
  // TODO: add other information
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
} // namespace apache::thrift::detail::md
