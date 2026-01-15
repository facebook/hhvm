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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

#include <fmt/core.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Exception.h>

#include <deque>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace apache::thrift::dynamic {

namespace detail {

/**
 * Serialize a value to a SimpleJSON string.
 */
template <typename T>
std::string toSimpleJSON(const T& value) {
  folly::IOBufQueue queue;
  SimpleJSONProtocolWriter writer;
  writer.setOutput(&queue);

  if constexpr (std::convertible_to<T, std::string_view>) {
    op::encode<type::string_t>(writer, std::string_view(value));
  } else {
    op::encode<type::infer_tag<T>>(writer, value);
  }

  return queue.move()->to<std::string>();
}

/**
 * Get a display name for a TypeRef.
 */
std::string typeDisplayName(const type_system::TypeRef& type);

} // namespace detail

/**
 * Exception thrown when a path access is invalid for the current type.
 */
class InvalidPathAccessError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

/**
 * Represents a path through a Thrift structure as a sequence of access
 * operations.
 *
 * Path format when converted to string:
 *   <root><component>*
 *
 * Where each component is one of:
 *   .fieldName     - struct/union field access
 *   [index]        - list element access
 *   {<value>}      - set element or map key
 *   [<value>]      - map value access by key or any type access
 *
 * The <value> is serialized using Thrift's SimpleJSON protocol.
 *
 * Example paths:
 *   MyStruct.users["alice"].scores[0]
 *   MyStruct.tags{"important"}
 *   MyStruct.userData[meta.com/ads/TargetingFeatures].age
 */
class Path {
 public:
  /**
   * Returns the current path as a string.
   */
  std::string toString() const;

 private:
  /**
   * Access types for path components.
   */
  struct FieldAccess {
    std::string fieldName;

    bool operator==(const FieldAccess&) const = default;
  };

  struct ListElement {
    std::size_t index;

    bool operator==(const ListElement&) const = default;
  };

  struct SetElement {
    // Serialized value - may replace with DynamicValue so shouldn't expose.
    std::string value;

    bool operator==(const SetElement&) const = default;
  };

  struct MapKey {
    // Serialized key - may replace with DynamicValue so shouldn't expose.
    std::string key;

    bool operator==(const MapKey&) const = default;
  };

  struct MapValue {
    // Serialized key - may replace with DynamicValue so shouldn't expose.
    std::string key;

    bool operator==(const MapValue&) const = default;
  };

  struct AnyType {
    // URI/typeId - may replace with TypeRef so shouldn't expose.
    std::string typeId;

    bool operator==(const AnyType&) const = default;
  };

  using Component = std::
      variant<FieldAccess, ListElement, SetElement, MapKey, MapValue, AnyType>;

  /**
   * Returns the root type name.
   */
  const std::string& rootTypeName() const { return rootTypeName_; }

  /**
   * Returns the path components.
   */
  std::span<const Component> components() const { return components_; }

  /**
   * Construct a Path with a root type name.
   */
  explicit Path(std::string rootTypeName);

  /**
   * Add a component to the path.
   */
  void push(Component component);

  /**
   * Remove the last component from the path.
   */
  void pop();

  std::string rootTypeName_;
  std::vector<Component> components_;

  friend class PathBuilder;
};

/**
 * A builder for creating Path objects with type validation.
 *
 * Provides scope guards for entering fields, list elements, map keys/values,
 * etc., which automatically pop the path component when destroyed.
 *
 * This class validates that all accesses are valid for the current type.
 * If an invalid access is attempted, an InvalidPathAccessError is thrown.
 */
class PathBuilder {
 public:
  class ScopeGuard {
   public:
    ~ScopeGuard();

    ScopeGuard(ScopeGuard&& other) noexcept = delete;
    ScopeGuard& operator=(ScopeGuard&& other) noexcept = delete;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

   private:
    friend class PathBuilder;
    explicit ScopeGuard(PathBuilder* builder);

    PathBuilder* builder_;
  };

  /**
   * Construct a PathBuilder with a root type.
   */
  explicit PathBuilder(type_system::TypeRef rootType);

  /**
   * Enter a struct field with the given name.
   * Returns a scope guard that pops this path component on destruction.
   *
   * Throws InvalidPathAccessError if the current type is not a struct/union
   * or does not have a field with the given name.
   */
  [[nodiscard]] ScopeGuard enterField(std::string_view fieldName);
  [[nodiscard]] ScopeGuard enterField(type::FieldId fieldId);
  [[nodiscard]] ScopeGuard enterField(type_system::FastFieldHandle fieldHandle);

  /**
   * Enter a list element at the given index.
   * Returns a scope guard that pops this path component on destruction.
   *
   * Throws InvalidPathAccessError if the current type is not a list.
   */
  [[nodiscard]] ScopeGuard enterListElement(std::size_t index);

  /**
   * Enter a set element with the given value.
   * Returns a scope guard that pops this path component on destruction.
   *
   * Throws InvalidPathAccessError if the current type is not a set.
   */
  template <typename T>
  [[nodiscard]] ScopeGuard enterSetElement(const T& value) {
    const auto& current = currentType();
    if (!current.isSet()) {
      folly::throw_exception<InvalidPathAccessError>(fmt::format(
          "cannot access set element on non-set type '{}'",
          detail::typeDisplayName(current)));
    }
    typeStack_.push_back(current.asSet().elementType());
    path_.push(Path::SetElement{detail::toSimpleJSON(value)});
    return ScopeGuard(this);
  }

  /**
   * Enter the given map key.
   * Returns a scope guard that pops this path component on destruction.
   *
   * Throws InvalidPathAccessError if the current type is not a map.
   */
  template <typename T>
  [[nodiscard]] ScopeGuard enterMapKey(const T& key) {
    const auto& current = currentType();
    if (!current.isMap()) {
      folly::throw_exception<InvalidPathAccessError>(fmt::format(
          "cannot access map key on non-map type '{}'",
          detail::typeDisplayName(current)));
    }
    typeStack_.push_back(current.asMap().keyType());
    path_.push(Path::MapKey{detail::toSimpleJSON(key)});
    return ScopeGuard(this);
  }

  /**
   * Enter a map value with the given key.
   * Returns a scope guard that pops this path component on destruction.
   *
   * Throws InvalidPathAccessError if the current type is not a map.
   */
  template <typename T>
  [[nodiscard]] ScopeGuard enterMapValue(const T& key) {
    return enterMapValueImpl(detail::toSimpleJSON(key));
  }

  /**
   * Enter an any type with the given known inner type.
   * Returns a scope guard that pops this path component on destruction.
   *
   * Throws InvalidPathAccessError if the current type is not an any type.
   */
  [[nodiscard]] ScopeGuard enterAnyType(type_system::TypeRef knownType);

  /**
   * Returns the current path as a string.
   */
  std::string toString() const { return path_.toString(); }

  /**
   * Returns a copy of the current path.
   */
  Path path() const& { return path_; }
  Path path() && { return std::move(path_); }

  /**
   * Returns the current type at this path location.
   */
  const type_system::TypeRef& currentType() const { return typeStack_.back(); }

 private:
  [[nodiscard]] ScopeGuard enterMapValueImpl(std::string key);
  void pop();

  template <typename Handle>
  [[nodiscard]] ScopeGuard enterFieldImpl(Handle handle);

  Path path_;
  std::deque<type_system::TypeRef> typeStack_;
};

} // namespace apache::thrift::dynamic
