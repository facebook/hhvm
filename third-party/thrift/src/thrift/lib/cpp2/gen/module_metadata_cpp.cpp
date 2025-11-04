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

GenMetadataResult<metadata::ThriftEnum> genEnumMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::EnumNode& node) {
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
  return ret;
}

template <class Metadata>
static auto genStructuredInMetadataMap(
    const syntax_graph::StructuredNode& node,
    std::map<std::string, Metadata>& metadataMap) {
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
  }
  // TODO: add other information
  return ret;
}

GenMetadataResult<metadata::ThriftStruct> genStructMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::StructuredNode& node) {
  return genStructuredInMetadataMap(node, *md.structs());
}

GenMetadataResult<metadata::ThriftException> genExceptionMetadata(
    metadata::ThriftMetadata& md, const syntax_graph::ExceptionNode& node) {
  return genStructuredInMetadataMap(node, *md.exceptions());
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
