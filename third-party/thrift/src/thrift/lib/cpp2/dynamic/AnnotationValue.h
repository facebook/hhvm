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

#include <vector>

#include <folly/container/F14Map.h>
#include <folly/container/span.h>

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>

namespace folly {
struct dynamic;
}

namespace apache::thrift::dynamic {

// Reconstruct a structured annotation value (exposed by SyntaxGraph as a
// folly::dynamic keyed by field name) into a schema-typed DynamicValue of the
// given type. Lives in its own target so SyntaxGraph-backed descriptor builders
// can share it without pulling folly::dynamic into ServiceDescriptor or forming
// a schema/dynamic dependency cycle.
DynamicValue toDynamicValue(
    const folly::dynamic& value, const type_system::TypeRef& type);

// Serialize a dynamic value (e.g. an annotation value) into its schema-agnostic
// SerializableRecord form. The inverse of fromRecord: enums serialize as their
// i32 datum, unions as a single-entry field set.
type_system::SerializableRecord toSerializableRecord(DynamicConstRef value);

// Reconstruct a dynamic value of the given type from its SerializableRecord
// form. The inverse of toSerializableRecord.
DynamicValue fromSerializableRecord(
    const type_system::SerializableRecord& record,
    const type_system::TypeRef& type);

// Serialize annotation values into the wire annotations map, keyed by each
// value's annotation-struct URI. Annotations under
// facebook.com/thrift/annotation/ are omitted, matching the type-system
// serializer: their definitions are not bundled into the type universe, so the
// keys would not resolve.
folly::F14FastMap<std::string, type_system::SerializableRecordUnion>
serializeAnnotations(folly::span<const DynamicValue> annotations);

// Reconstruct annotation values from the wire annotations map produced by
// serializeAnnotations: each URI key is resolved against typeSystem to type its
// record, which is then rebuilt into a schema-typed DynamicValue.
std::vector<DynamicValue> deserializeAnnotations(
    const folly::F14FastMap<std::string, type_system::SerializableRecordUnion>&
        annotations,
    const type_system::TypeSystem& typeSystem);

} // namespace apache::thrift::dynamic
