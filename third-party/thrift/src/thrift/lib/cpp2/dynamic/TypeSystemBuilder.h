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

#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

#include <folly/container/F14Map.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::type_system {

/**
 * TypeSystemBuilder provides an ergonomic API to declaratively build a
 * TypeSystem implementation.
 *
 * Example:
 *
 *     using def = TypeSystemBuilder::DefinitionHelper;
 *     TypeSystemBuilder builder;
 *
 *     builder.addType(
 *         "meta.com/foo/OuterStruct",
 *         def::Struct({
 *             def::Field(
 *                 def::Identity(1, "innerUnion"),
 *                 def::AlwaysPresent,
 *                 TypeIds::uri("meta.com/foo/innerUnion")),
 *         }),
 *         def::sourceInfo("path/to/foo.thrift", "OuterStruct"));
 *
 *     builder.addType(
 *         "meta.com/foo/innerUnion",
 *         def::Union({
 *             def::Field(
 *                 def::Identity(1, "field1"), def::Optional, TypeIds::I32),
 *         }),
 *         def::SourceInfo("path/to/foo.thrift", "InnerUnion"));
 *
 *     std::unique_ptr<TypeSystem> typeSystem = std::move(builder).build();
 *
 * This roughly corresponds to the following Thrift IDL definition:
 *
 *     // path/to/foo.thrift
 *     package "meta.com/foo"
 *
 *     struct OuterStruct {
 *       1: InnerUnion innerUnion;
 *     }
 *
 *     union InnerUnion {
 *       1: i32 field1;
 *     }
 */
class TypeSystemBuilder {
 public:
  TypeSystemBuilder() = default;

  /**
   * Defines a Thrift struct. See DefinitionHelper::Struct for more details.
   */
  void addType(
      Uri,
      const SerializableStructDefinition&,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  /**
   * Defines a Thrift union. See DefinitionHelper::Union for more details.
   */
  void addType(
      Uri,
      const SerializableUnionDefinition&,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  /**
   * Defines a Thrift enum. See DefinitionHelper::Enum for more details.
   */
  void addType(
      Uri,
      const SerializableEnumDefinition&,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  /**
   * Defines a Thrift opaque alias. See DefinitionHelper::OpaqueAlias for more
   * details.
   */
  void addType(
      Uri,
      const SerializableOpaqueAliasDefinition&,
      std::optional<SerializableThriftSourceInfo> = std::nullopt);

  void addType(Uri, const SerializableTypeDefinitionEntry&);
  void addTypes(SerializableTypeSystem);

  /**
   * Builds a standalone TypeSystem from the added types.
   */
  std::unique_ptr<TypeSystem> build() &&;

  /**
   * Builds a TypeSystem that derives from an existing base TypeSystem.
   *
   * The definitions added to this builder form an "overlay" on top of the
   * provided base. If a type is not defined in the overlay, then the derivation
   * will fall back to the base.
   *
   * In short:
   *
   *     Derivation (result) = Overlay (this) ∪ Base (argument)
   *
   * ──────────────────────────────────────────────────────────────────────────
   * Derivation Semantics
   * ──────────────────────────────────────────────────────────────────────────
   * Types in the overlay may reference types defined in the base. For example,
   * an newly defined struct may have a field whose type is an existing struct
   * defined in the base.
   *
   * Derivation produces a single TypeSystem instance. This instance will
   * transparently return results from either the overlay or the base as
   * necessary.
   *
   * Deriviation is purely additive. The overlay cannot:
   *   - Change the schema of existing types in the base.
   *   - Hide types in the base (shadow an existing URI).
   * This eliminates schema compatibility concerns, by construction.
   *
   * ──────────────────────────────────────────────────────────────────────────
   * Validation
   * ──────────────────────────────────────────────────────────────────────────
   * URI uniqueness:
   *   No type added to this builder may have the same URI as a type in base.
   *
   * Source identifier uniqueness:
   *   No type added to this builder may have the same source identifier
   *   (location + name) as a type in base.
   *
   * Throws:
   *   - InvalidTypeError if an added type's URI conflicts with a type in base.
   *   - InvalidTypeError if an added type's source identifier conflicts with a
   *     type in base.
   *   - InvalidTypeError if a TypeId referenced by an added type cannot be
   *     resolved.
   */
  std::unique_ptr<TypeSystem> buildDerivedFrom(
      std::shared_ptr<const TypeSystem> base) &&;

  /**
   * A helper class that provides a more declarative experience when defining a
   * TypeSystem through TypeSystemBuilder.
   *
   * This class is not instantiable. It provides a set of static methods that
   * produce intermediate data structures for use with TypeSystemBuilder.
   */
  struct DefinitionHelper {
    DefinitionHelper() = delete;

    static constexpr inline PresenceQualifier Optional =
        PresenceQualifier::OPTIONAL_;
    static constexpr inline PresenceQualifier AlwaysPresent =
        PresenceQualifier::UNQUALIFIED;

    /**
     * Creates the identity a field for use in a struct or union definition.
     *
     * A field include must include both the ID and name of the field.
     */
    static FieldIdentity Identity(std::int16_t id, std::string_view name);

    /**
     * Creates a FieldDefinition for use in a struct or union definition.
     */
    static SerializableFieldDefinition Field(
        FieldIdentity identity,
        PresenceQualifier presence,
        TypeId type,
        std::optional<SerializableRecord> customDefault = std::nullopt,
        const AnnotationsMap& annotations = {});

    /**
     * Defines a Thrift struct, which is a collection of fields.
     */
    static SerializableStructDefinition Struct(
        std::vector<SerializableFieldDefinition> fields,
        bool isSealed = false,
        const AnnotationsMap& annotations = {});

    /**
     * Defines a Thrift union, which is a collection of fields. At most one
     * field can be present. All fields must be optional.
     */
    static SerializableUnionDefinition Union(
        std::vector<SerializableFieldDefinition> fields,
        bool isSealed = false,
        const AnnotationsMap& annotations = {});

    struct EnumValue {
      std::string name;
      std::int32_t value;
      AnnotationsMap annotations;

      EnumValue(std::string n, std::int32_t v) : name{std::move(n)}, value{v} {}
      EnumValue(std::string n, std::int32_t v, AnnotationsMap m)
          : name{std::move(n)}, value{v}, annotations(std::move(m)) {}
    };

    /**
     * Defines a Thrift enum, which is a mapping of unique names to unique
     * 32-bit integer values.
     */
    static SerializableEnumDefinition Enum(
        const std::vector<EnumValue>& values,
        const AnnotationsMap& annotations = {});

    /**
     * Defines a Thrift opaque alias, which is a distinct user-defined type
     * (with a URI) with the schema of an underlying built-in type.
     *
     * The target type must not be a user-defined type.
     */
    static SerializableOpaqueAliasDefinition OpaqueAlias(
        TypeId targetType, const AnnotationsMap& annotations = {});

    /**
     * Optional information about the source IDL files from which a definition
     * originates.
     */
    static SerializableThriftSourceInfo SourceInfo(
        std::string_view location, std::string_view name);
  };

 private:
  struct DefinitionEntry {
    SerializableTypeDefinition definition;
    std::optional<SerializableThriftSourceInfo> sourceInfo;
  };
  folly::F14FastMap<Uri, DefinitionEntry> definitions_;

  void tryEmplace(Uri, DefinitionEntry&&);
};

} // namespace apache::thrift::type_system
