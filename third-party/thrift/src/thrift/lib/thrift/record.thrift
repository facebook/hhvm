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
include "thrift/lib/thrift/standard.thrift"

namespace cpp2 apache.thrift.type_system
namespace py3 apache.thrift.type_system

/**
 * A Thrift record is a concrete, machine-readable, description of every datum
 * that is possible to represent in Thrift.
 *
 * A record captures the structure of datums without association to a Thrift
 * type. This structure derives from de-facto standard constructs in the
 * computing ecosystem. Most (if not all) useful programming languages and
 * computer hardware can efficiently support these structures.
 *
 * A SerializableRecord is a description of a record as a Thrift struct, so that
 * it may be persisted and transmitted in a language-agnostic manner.
 *
 * SerializableRecord is not intended to be an optimized representation to
 * operate on Thrift data itself. Instead, a separate implementation should be
 * used for data manipulation and converted to/from SerializableRecord when
 * persisting/transmitting.
 */
@cpp.Name{value = "SerializableRecordUnion"}
union SerializableRecord {
  1: bool boolDatum;
  2: byte int8Datum;
  3: i16 int16Datum;
  5: i32 int32Datum;
  6: i64 int64Datum;
  7: float float32Datum;
  8: double float64Datum;
  9: string textDatum;

  @cpp.Ref{type = cpp.RefType.Unique}
  10: standard.ByteBuffer byteArrayDatum;

  11: map<id.FieldId, SerializableRecord> fieldSetDatum;

  12: list<SerializableRecord> listDatum;

  /**
   * This list MUST not contain two elements that compare equal to each other.
   *
   * At present, Thrift implementations have poor support for structured types
   * as the element of set types. Therefore, we use a list to instead.
   */
  13: list<SerializableRecord> setDatum;

  /**
   * This list MUST not contain two entries whose keys compare equal to each other.
   *
   * At present, Thrift implementations have poor support for structured types
   * as the element of set types. Therefore, we use a list to instead.
   */
  14: list<SerializableRecordMapEntry> mapDatum;
  // Thrift's object model defines "Any" as a record kind. But we are choosing
  // to omit the ability to represent such records in the serializable data format.
  //
  // The rationale for the omissions include:
  //   - SerializableRecord is primarily used in type_system.thrift to encode
  //     the definition of Thrift schema, specifically custom default values
  //     for fields, and structured annotations. These are not expected to be
  //     "Any" records.
  //   - The existing any.Any deviates from the strict object model definition
  //     in some small ways. It is unclear whether any.Any should be forked
  //     *just* for SerializableRecord.
  //   - A future revision may relax this restriction since adding a field
  //     to a union is a common-field-preserving change. There is no reason to
  //     rush the inclusion of "Any" here.
  //
  // 15: ??? anyDatum;
}

struct SerializableRecordMapEntry {
  1: SerializableRecord key;
  2: SerializableRecord value;
}
