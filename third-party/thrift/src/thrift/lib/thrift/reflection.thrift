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

cpp_include "<unordered_map>"

namespace cpp2 apache.thrift.reflection

// A type id is a 64-bit value.  The least significant 5 bits of the type id
// are the actual type (one of the values of the Type enum, below).  The
// remaining 59 bits are a unique identifier for container or user-defined
// types, or 0 for base types (void, string, bool, byte, i16, i32, i64,
// and double)

// IMPORTANT!
// These values must be exactly the same as those defined in
// thrift/compiler/parse/t_type.h
include "thrift/annotation/cpp.thrift"

enum Type {
  TYPE_VOID = 0,
  TYPE_STRING = 1,
  TYPE_BOOL = 2,
  TYPE_BYTE = 3,
  TYPE_I16 = 4,
  TYPE_I32 = 5,
  TYPE_I64 = 6,
  TYPE_DOUBLE = 7,
  TYPE_ENUM = 8,
  TYPE_LIST = 9,
  TYPE_SET = 10,
  TYPE_MAP = 11,
  TYPE_STRUCT = 12,
  TYPE_SERVICE = 13,
  TYPE_PROGRAM = 14,
  TYPE_FLOAT = 15,
}

struct StructField {
  1: bool isRequired;
  2: i64 type;
  3: string name;
  @cpp.Type{template = "std::unordered_map"}
  4: optional map<string, string> annotations;
  5: i16 order; // lexical order of this field, 0-based
}

struct DataType {
  1: string name;
  @cpp.Type{template = "std::unordered_map"}
  2: optional map<i16, StructField> fields;
  3: optional i64 mapKeyType;
  4: optional i64 valueType;
  @cpp.Type{template = "std::unordered_map"}
  5: optional map<string, i32> enumValues;
}

struct Schema {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i64, DataType> dataTypes;
  // name to type
  @cpp.Type{template = "std::unordered_map"}
  2: map<string, i64> names;
}
