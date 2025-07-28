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

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
package "facebook.com/thrift/type_system"

include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/id.thrift"
include "thrift/lib/thrift/type_id.thrift"
include "thrift/lib/thrift/record.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "thrift/lib/thrift/detail/TypeSystemAdapter.h"
cpp_include "folly/container/F14Map.h"

namespace cpp2 apache.thrift.type_system

@thrift.AllowLegacyTypedefUri
typedef id.FieldId FieldId
@thrift.AllowLegacyTypedefUri
typedef string FieldName

/**
 * The identifier for a field in a structured type.
 *
 * Both name and ID must be unique in a set of fields for a given type.
 */
@cpp.Adapter{
  underlyingName = "FieldIdentityStruct",
  name = "::apache::thrift::type_system::detail::FieldIdentityAdapter",
}
struct FieldIdentity {
  1: FieldId id;
  2: FieldName name;
}

enum PresenceQualifier {
  // This is an invalid state
  DEFAULT_INITIALIZED = 0,

  UNQUALIFIED = 1,
  @cpp.Name{value = "OPTIONAL_"}
  OPTIONAL = 2,
  TERSE = 3,
}

/**
 * A field that is part of a structured type (struct or union).
 */
@cpp.EnableCustomTypeOrdering
struct SerializableFieldDefinition {
  1: FieldIdentity identity;
  2: PresenceQualifier presence;
  3: type_id.TypeId type;
  4: optional record.SerializableRecord customDefaultValue;
  /**
   * The uri key must resolve to struct definition, and the record value must be
   * valid for the specified uri key.
   */
  @cpp.Type{template = "folly::F14FastMap"}
  5: map<type_id.Uri, record.SerializableRecord> annotations;
}

/**
 * A structured type that is composed of a set of fields with unique identities.
 *
 * Both the names and IDs of each field must be unique for a struct.
 */
@cpp.EnableCustomTypeOrdering
struct SerializableStructDefinition {
  2: list<SerializableFieldDefinition> fields;
  /**
   * If true, then the schema (set of fields) cannot be modified without
   * breaking backward compatibility.
   */
  3: bool isSealed;
  /**
   * The uri key must resolve to struct definition, and the record value must be
   * valid for the specified uri key.
   */
  @cpp.Type{template = "folly::F14FastMap"}
  4: map<type_id.Uri, record.SerializableRecord> annotations;
}

/**
 * A structured type that is like a struct, except that a valid value may only
 * have one field present at a time.
 *
 * All fields must have optional presence qualifiers.
 */
@cpp.EnableCustomTypeOrdering
struct SerializableUnionDefinition {
  2: list<SerializableFieldDefinition> fields;
  /**
   * If true, then the schema (set of fields) cannot be modified without
   * breaking backward compatibility.
   */
  3: bool isSealed;
  /**
   * The uri key must resolve to struct definition, and the record value must be
   * valid for the specified uri key.
   */
  @cpp.Type{template = "folly::F14FastMap"}
  4: map<type_id.Uri, record.SerializableRecord> annotations;
}

@cpp.EnableCustomTypeOrdering
struct SerializableEnumValueDefinition {
  1: string name;
  2: i32 datum;
  /**
   * The uri key must resolve to struct definition, and the record value must be
   * valid for the specified uri key.
   */
  @cpp.Type{template = "folly::F14FastMap"}
  3: map<type_id.Uri, record.SerializableRecord> annotations;
}

@cpp.EnableCustomTypeOrdering
struct SerializableEnumDefinition {
  2: list<SerializableEnumValueDefinition> values;
  /**
   * The uri key must resolve to struct definition, and the record value must be
   * valid for the specified uri key.
   */
  @cpp.Type{template = "folly::F14FastMap"}
  3: map<type_id.Uri, record.SerializableRecord> annotations;
}

/**
 * A user-defined type which allows associating a URI with the datums of
 * another type.
 */
@cpp.EnableCustomTypeOrdering
struct SerializableOpaqueAliasDefinition {
  /**
   * The target type must not be another user-defined type. That is, the typeid
   * must not be a URI.
   */
  2: type_id.TypeId targetType;
  /**
   * The uri key must resolve to struct definition, and the record value must be
   * valid for the specified uri key.
   */
  @cpp.Type{template = "folly::F14FastMap"}
  3: map<type_id.Uri, record.SerializableRecord> annotations;
}

union SerializableTypeDefinition {
  1: SerializableStructDefinition structDef;
  2: SerializableUnionDefinition unionDef;
  3: SerializableEnumDefinition enumDef;
  4: SerializableOpaqueAliasDefinition opaqueAliasDef;
}

struct SerializableThriftSourceInfo {
  /**
   * A URI of the resource (typically a file) containing the source Thrift IDL
   * for a user-defined type.
   *
   * Examples:
   *   - "file:///thrift/lib/thrift/standard.thrift"
   *   - "fbsource://xplat/thrift/lib/thrift/standard.thrift"
   */
  2: string locator;
  /**
   * The name of the definition within the source Thrift IDL pointed to by the
   * locator above
   */
  3: string name;
}

struct SerializableTypeDefinitionEntry {
  1: SerializableTypeDefinition definition;
  /**
   * If applicable, information about the Thrift IDL source code from which this
   * definition originates.
   */
  2: optional SerializableThriftSourceInfo sourceInfo;
}

/**
 * A type system is a collection of types where each type (and all types it
 * refers to) are fully defined within the same type system.
 */
struct SerializableTypeSystem {
  1: map<type_id.Uri, SerializableTypeDefinitionEntry> types;
}
