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

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/type.thrift"
cpp_include "folly/container/F14Map.h"
cpp_include "<thrift/lib/thrift/TypeToMaskAdapter.h>"

@thrift.Experimental
package "facebook.com/thrift/protocol"

namespace cpp2 apache.thrift.protocol
namespace py3 apache.thrift.protocol
namespace php apache_thrift_protocol
namespace java.swift com.facebook.thrift.protocol_swift
namespace py.asyncio apache_thrift_asyncio.field_mask
namespace go thrift.lib.thrift.field_mask
namespace py thrift.lib.thrift.field_mask

@thrift.AllowLegacyTypedefUri
@cpp.Type{template = "::folly::F14FastMap"}
typedef map<i16, Mask> FieldIdToMask
@thrift.AllowLegacyTypedefUri
@cpp.Type{template = "::folly::F14FastMap"}
typedef map<i64, Mask> MapIdToMask
@thrift.AllowLegacyTypedefUri
@cpp.Type{template = "::folly::F14FastMap"}
typedef map<string, Mask> MapStringToMask

struct TypeAndMaskEntry {
  1: type.Type type;
  2: Mask mask;
}

/**
 * Ideally this would've been a map from type to Mask
 * However, structured keys are not well supported (eg. in Hack),
 * so we use a list (and adapters in supported languages) as a workaround
 *
 * Contract (for unadapted usage): There must be at most one entry for a
 * given type within the list
 */
@thrift.AllowLegacyTypedefUri
@cpp.Adapter{
  name = "::apache::thrift::protocol::detail::TypeToMaskAdapter<
      ::apache::thrift::protocol::TypeAndMaskEntry,
      ::apache::thrift::protocol::Mask
    >",
}
typedef list<TypeAndMaskEntry> MapTypeToMask

/**
 *  Overview
 *  --------
 *  Field Mask is a data structure that represents a subset of fields and
 *  nested fields of a thrift struct. The purpose is to provide utilities
 *  to manipulate these fields such as copying certain fields and nested fields
 *  from one struct instance to another.
 *
 *  The concept is similar to GraphQL’s field selectors, JSON:API’s Sparse
 *  Fieldsets and Protobuf’s FieldMask.
 *
 *  Implementation
 *  --------------
 *  It is a union data structure, which can represent the map of exclusive
 *  fields or the map of inclusive fields. The map maps from field id to the
 *  mask for the nested thrift struct. If it is set to excludes, the mask
 *  works by excluding all fields in the map. If the field is another thrift
 *  struct, it excludes the nested fields that are included in the nested mask.
 *  Includes works similarly by including the fields specified by the map.
 *
 *  Usage
 *  --------------
 *  // In thrift file
 *  struct Nested {
 *    1: i32 field_1;
 *    2: i32 field_2;
 *  }
 *
 *  struct Foo {
 *    10: Nested nested;
 *    20: i32 field_3;
 *  }
 *
 *  // Masks field_2 and field_3 of Foo in C++
 *  Mask mask1;
 *  mask1.includes_ref().ensure()[10].includes_ref().ensure()[2] = allMask;
 *  mask1.includes_ref().ensure()[20] = allMask;
 *
 *  // Alternatively we can exclude field_1 in Foo
 *  Mask mask2;
 *  mask2.excludes_ref().ensure()[10].includes_ref().ensure()[1] = allMask;
 *
 */
// Inclusive fields should always be an even number.
@cpp.EnableCustomTypeOrdering
union Mask {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: FieldIdToMask excludes; // Fields that will be excluded.
  @cpp.Ref{type = cpp.RefType.Unique}
  2: FieldIdToMask includes; // Fields that will be included.

  @cpp.Ref{type = cpp.RefType.Unique}
  3: MapIdToMask excludes_map; // Map fields that will be excluded.
  @cpp.Ref{type = cpp.RefType.Unique}
  4: MapIdToMask includes_map; // Map fields that will be included.

  @cpp.Ref{type = cpp.RefType.Unique}
  5: MapStringToMask excludes_string_map; // String map fields that will be excluded.
  @cpp.Ref{type = cpp.RefType.Unique}
  6: MapStringToMask includes_string_map; // String map fields that will be included.

  7: MapTypeToMask excludes_type; // types to be excluded for thrift.Any
  8: MapTypeToMask includes_type; // types to be included for thrift.Any
}

const Mask allMask = Mask{excludes = {}}; // Masks all fields.
const Mask noneMask = Mask{includes = {}}; // Masks no fields.
