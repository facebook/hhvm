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

namespace cpp2 apache.thrift.metadata
namespace java.swift com.facebook.thrift.metadata
namespace py apache.thrift.metadata
namespace py.asyncio apache_thrift_asyncio.metadata
namespace py3 apache.thrift
namespace js apache.thrift
namespace php tmeta
namespace go thrift.lib.thrift.metadata

/*
 * Keep synced with : thrift/compiler/generate/t_hack_generator.cc
 */
include "thrift/annotation/cpp.thrift"

enum ThriftPrimitiveType {
  THRIFT_BOOL_TYPE = 1,
  THRIFT_BYTE_TYPE = 2,
  THRIFT_I16_TYPE = 3,
  THRIFT_I32_TYPE = 4,
  THRIFT_I64_TYPE = 5,
  THRIFT_FLOAT_TYPE = 6,
  THRIFT_DOUBLE_TYPE = 7,
  THRIFT_BINARY_TYPE = 8,
  THRIFT_STRING_TYPE = 9,
  THRIFT_VOID_TYPE = 10,
}

struct ThriftConstValuePair {
  1: ThriftConstValue key;
  2: ThriftConstValue value;
}

union ThriftConstValue {
  1: bool cv_bool;
  2: i64 cv_integer;
  3: double cv_double;
  4: string cv_string;
  5: list<ThriftConstValuePair> cv_map;
  6: list<ThriftConstValue> cv_list;
  7: ThriftConstStruct cv_struct;
}

struct ThriftConstStruct {
  1: ThriftStructType type;
  2: map<string, ThriftConstValue> fields;
}

struct ThriftListType {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ThriftType valueType (
    rust.box,
    swift.recursive_reference = "true",
  );
}

struct ThriftSetType {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ThriftType valueType (
    rust.box,
    swift.recursive_reference = "true",
  );
}

struct ThriftMapType {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ThriftType keyType (rust.box, swift.recursive_reference = "true");
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ThriftType valueType (
    rust.box,
    swift.recursive_reference = "true",
  );
}

struct ThriftEnumType {
  1: string name;
}

struct ThriftStructType {
  1: string name;
}

struct ThriftUnionType {
  1: string name;
}

struct ThriftTypedefType {
  1: string name;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ThriftType underlyingType (
    rust.box,
    swift.recursive_reference = "true",
  );
  3: list<ThriftConstStruct> structured_annotations;
}

struct ThriftStreamType {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ThriftType elemType (
    rust.box,
    swift.recursive_reference = "true",
  );
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ThriftType initialResponseType (
    rust.box,
    swift.recursive_reference = "true",
  );
}

struct ThriftSinkType {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ThriftType elemType (
    rust.box,
    swift.recursive_reference = "true",
  );
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ThriftType finalResponseType (
    rust.box,
    swift.recursive_reference = "true",
  );
  @cpp.Ref{type = cpp.RefType.Unique}
  3: optional ThriftType initialResponseType (
    rust.box,
    swift.recursive_reference = "true",
  );
}

union ThriftType {
  1: ThriftPrimitiveType t_primitive;
  2: ThriftListType t_list;
  3: ThriftSetType t_set;
  4: ThriftMapType t_map;
  5: ThriftEnumType t_enum;
  6: ThriftStructType t_struct;
  7: ThriftUnionType t_union;
  8: ThriftTypedefType t_typedef;
  9: ThriftStreamType t_stream;
  10: ThriftSinkType t_sink;
}

struct ThriftEnum {
  1: string name;
  2: map<i32, string> elements;
  3: list<ThriftConstStruct> structured_annotations;
}

struct ThriftField {
  1: i32 id;
  2: ThriftType type;
  3: string name;
  4: bool is_optional;
  5: list<ThriftConstStruct> structured_annotations;
  6: optional map<string, string> unstructured_annotations;
}

struct ThriftStruct {
  1: string name;
  2: list<ThriftField> fields;
  3: bool is_union;
  4: list<ThriftConstStruct> structured_annotations;
}

struct ThriftException {
  1: string name;
  2: list<ThriftField> fields;
  3: list<ThriftConstStruct> structured_annotations;
}

struct ThriftFunction {
  1: string name;
  2: ThriftType return_type;
  3: list<ThriftField> arguments;
  4: list<ThriftField> exceptions;
  5: bool is_oneway;
  6: list<ThriftConstStruct> structured_annotations;
}

struct ThriftService {
  1: string name;
  2: list<ThriftFunction> functions;
  3: optional string parent;
  4: list<ThriftConstStruct> structured_annotations;
}

// ThriftModuleContext represents module-specific metadata.
struct ThriftModuleContext {
  1: string name;
}

// DEPRECATED! ThriftServiceContext represents service-specific metadata.
struct ThriftServiceContext {
  1: ThriftService service_info;
  2: ThriftModuleContext module;
}

struct ThriftServiceContextRef {
  1: string service_name;
  2: ThriftModuleContext module;
}

struct ThriftServiceMetadataResponse {
  // DEPRECATED! Use `services`.
  1: ThriftServiceContext context;
  2: ThriftMetadata metadata;
  // All service interfaces (including inherited ones) running on the server.
  3: list<ThriftServiceContextRef> services;
}

/**
 * ThriftMetadata is for looking up types/exceptions with a specific name in
 * some environments, typically used by ThriftServiceMetadataResponse to
 * help finding definitions of a service's depending types/exceptions.
 */
struct ThriftMetadata {
  // 1: string file_name;
  // ThriftMetadata is now used as a pure container for name lookup, and users
  // should use ThriftModuleContext instead if they want to get module name.
  2: map<string, ThriftEnum> enums;
  4: map<string, ThriftStruct> structs;
  5: map<string, ThriftException> exceptions;
  6: map<string, ThriftService> services;
}

service ThriftMetadataService {
  ThriftServiceMetadataResponse getThriftServiceMetadata();
}
