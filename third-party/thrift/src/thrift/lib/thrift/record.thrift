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
package "facebook.com/thrift/dynamic"

include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/id.thrift"
include "thrift/lib/thrift/standard.thrift"

cpp_include "folly/container/F14Map.h"
cpp_include "folly/container/F14Set.h"
cpp_include "thrift/lib/thrift/detail/SerializableRecordAdapter.h"

namespace cpp2 apache.thrift.dynamic

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
@cpp.Adapter{
  underlyingName = "SerializableRecordUnion",
  adaptedType = "::apache::thrift::dynamic::detail::SerializableRecord",
  name = "::apache::thrift::dynamic::detail::SerializableRecordAdapter",
}
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

  // @thrift.Box does not currently work with incomplete types
  11: map<id.FieldId, SerializableRecord> fieldSetDatum;

  12: list<SerializableRecord> listDatum;

  // @thrift.Box does not currently work with incomplete types
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{
    name = "::folly::F14FastSet<::apache::thrift::dynamic::SerializableRecord, ::apache::thrift::dynamic::detail::SerializableRecordHasher>",
  }
  13: set<SerializableRecord> setDatum;

  // @thrift.Box does not currently work with incomplete types
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{
    name = "::folly::F14FastMap<::apache::thrift::dynamic::SerializableRecord, ::apache::thrift::dynamic::SerializableRecord, ::apache::thrift::dynamic::detail::SerializableRecordHasher>",
  }
  14: map<SerializableRecord, SerializableRecord> mapDatum;
  // TODO(praihan): Add anyDatum
}
