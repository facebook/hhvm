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

#include <thrift/lib/cpp2/dynamic/Path.h>

#include <fmt/format.h>
#include <folly/Overload.h>
#include <folly/lang/Exception.h>

namespace apache::thrift::dynamic {

namespace detail {

std::string typeDisplayName(const type_system::TypeRef& type) {
  auto uriToName = [](std::string_view uri) {
    auto lastSlash = uri.find_last_of('/');
    if (lastSlash == std::string_view::npos) {
      return std::string(uri);
    } else {
      return std::string(uri.substr(lastSlash + 1));
    }
  };

  switch (type.kind()) {
    case type_system::TypeRef::Kind::BOOL:
      return "bool";
    case type_system::TypeRef::Kind::BYTE:
      return "byte";
    case type_system::TypeRef::Kind::I16:
      return "i16";
    case type_system::TypeRef::Kind::I32:
      return "i32";
    case type_system::TypeRef::Kind::I64:
      return "i64";
    case type_system::TypeRef::Kind::FLOAT:
      return "float";
    case type_system::TypeRef::Kind::DOUBLE:
      return "double";
    case type_system::TypeRef::Kind::STRING:
      return "string";
    case type_system::TypeRef::Kind::BINARY:
      return "binary";
    case type_system::TypeRef::Kind::ANY:
      return "any";
    case type_system::TypeRef::Kind::LIST:
      return fmt::format(
          "list<{}>", typeDisplayName(type.asList().elementType()));
    case type_system::TypeRef::Kind::SET:
      return fmt::format(
          "set<{}>", typeDisplayName(type.asSet().elementType()));
    case type_system::TypeRef::Kind::MAP:
      return fmt::format(
          "map<{}, {}>",
          typeDisplayName(type.asMap().keyType()),
          typeDisplayName(type.asMap().valueType()));
    case type_system::TypeRef::Kind::STRUCT:
      return uriToName(type.asStruct().uri());
    case type_system::TypeRef::Kind::UNION:
      return uriToName(type.asUnion().uri());
    case type_system::TypeRef::Kind::ENUM:
      return uriToName(type.asEnum().uri());
    case type_system::TypeRef::Kind::OPAQUE_ALIAS:
      return uriToName(type.asOpaqueAlias().uri());
  }
  return "unknown";
}

} // namespace detail

// Path implementation

Path::Path(std::string rootTypeName) : rootTypeName_(std::move(rootTypeName)) {}

void Path::push(Component component) {
  components_.push_back(std::move(component));
}

void Path::pop() {
  components_.pop_back();
}

std::string Path::toString() const {
  std::string result = rootTypeName_;

  for (const auto& component : components_) {
    folly::variant_match(
        component,
        [&](const FieldAccess& f) {
          result += fmt::format(".{}", f.fieldName);
        },
        [&](const ListElement& l) { result += fmt::format("[{}]", l.index); },
        [&](const SetElement& s) { result += fmt::format("{{{}}}", s.value); },
        [&](const MapKey& m) { result += fmt::format("{{{}}}", m.key); },
        [&](const MapValue& m) { result += fmt::format("[{}]", m.key); },
        [&](const AnyType& a) { result += fmt::format("[{}]", a.typeId); });
  }

  return result;
}

// PathBuilder implementation

PathBuilder::ScopeGuard::ScopeGuard(PathBuilder* builder) : builder_(builder) {}

PathBuilder::ScopeGuard::~ScopeGuard() {
  if (builder_) {
    builder_->pop();
  }
}

PathBuilder::PathBuilder(type_system::TypeRef rootType)
    : path_(detail::typeDisplayName(rootType)) {
  typeStack_.push_back(rootType);
}

namespace {
template <typename T>
decltype(auto) printable(T t) {
  if constexpr (requires { t.ordinal; }) {
    return t.ordinal;
  } else {
    return t;
  }
}
} // namespace

template <typename T>
PathBuilder::ScopeGuard PathBuilder::enterFieldImpl(T id) {
  const auto& current = currentType();

  if (!current.isStructured()) {
    folly::throw_exception<InvalidPathAccessError>(fmt::format(
        "cannot access field '{}' on non-structured type '{}'",
        printable(id),
        detail::typeDisplayName(current)));
  }

  const auto& structured = current.asStructured();
  auto handle = [&] {
    if constexpr (std::is_same_v<T, type_system::FastFieldHandle>) {
      return id;
    } else {
      return structured.fieldHandleFor(id);
    }
  }();

  if (!handle.valid()) {
    folly::throw_exception<InvalidPathAccessError>(fmt::format(
        "field '{}' does not exist on type '{}'",
        printable(id),
        detail::typeDisplayName(current)));
  }

  const auto& field = structured.at(handle);
  typeStack_.push_back(field.type());
  path_.push(Path::FieldAccess{field.identity().name()});

  return ScopeGuard(this);
}

PathBuilder::ScopeGuard PathBuilder::enterField(std::string_view handle) {
  return enterFieldImpl(handle);
}

PathBuilder::ScopeGuard PathBuilder::enterField(type::FieldId handle) {
  return enterFieldImpl(handle);
}

PathBuilder::ScopeGuard PathBuilder::enterField(
    type_system::FastFieldHandle handle) {
  return enterFieldImpl(handle);
}

PathBuilder::ScopeGuard PathBuilder::enterListElement(std::size_t index) {
  const auto& current = currentType();

  if (!current.isList()) {
    folly::throw_exception<InvalidPathAccessError>(fmt::format(
        "cannot access list element on non-list type '{}'",
        detail::typeDisplayName(current)));
  }

  typeStack_.push_back(current.asList().elementType());
  path_.push(Path::ListElement{index});

  return ScopeGuard(this);
}

PathBuilder::ScopeGuard PathBuilder::enterMapValueImpl(std::string key) {
  const auto& current = currentType();

  if (!current.isMap()) {
    folly::throw_exception<InvalidPathAccessError>(fmt::format(
        "cannot access map value on non-map type '{}'",
        detail::typeDisplayName(current)));
  }

  typeStack_.push_back(current.asMap().valueType());
  path_.push(Path::MapValue{std::move(key)});

  return ScopeGuard(this);
}

PathBuilder::ScopeGuard PathBuilder::enterAnyType(
    type_system::TypeRef knownType) {
  const auto& current = currentType();

  if (!current.isAny()) {
    folly::throw_exception<InvalidPathAccessError>(fmt::format(
        "cannot access any type on non-any type '{}'",
        detail::typeDisplayName(current)));
  }

  typeStack_.push_back(knownType);
  path_.push(Path::AnyType{knownType.id().name()});

  return ScopeGuard(this);
}

void PathBuilder::pop() {
  path_.pop();
  typeStack_.pop_back();
}

} // namespace apache::thrift::dynamic
