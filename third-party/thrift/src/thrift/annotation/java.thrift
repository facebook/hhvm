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

include "thrift/annotation/scope.thrift"

package "facebook.com/thrift/annotation/java"

namespace java com.facebook.thrift.annotation.java_deprecated
namespace android com.facebook.thrift.annotation.java_deprecated
namespace js thrift.annotation.java
namespace py.asyncio facebook_thrift_asyncio.annotation.java
namespace go thrift.annotation.java
namespace py thrift.annotation.java

// The thrift java compiler generate immutable structs and corresponding Builder by default.
// When this annotation is applied to a struct, thrift compiler will generate getters and setters for the struct.
@scope.Struct
@scope.Exception
struct Mutable {}

// When this annotation is applied, thrift compiler will annotate corresponding java entity the given java annotation.
@scope.Field
@scope.Struct
@scope.Union
@scope.Exception
struct Annotation {
  1: string java_annotation;
}

@scope.Typedef
struct BinaryString {}

// An annotation that is applied to a Typedef or field that maps it a Java type Adapter.
// For example:
// @java.Adapter{adapterClassName="com.facebook.thrift.TimestampAdapter", typeClassName="java.time.Instant"}
// typedef i64 Timestamp
//
@scope.Field
@scope.Typedef
@scope.Structured
struct Adapter {
  // Fully qualified name to a class that implements com.facebook.thrift.adaptor.TypeAdapter
  1: string adapterClassName;

  // Fully qualified name the above implementation adapts to
  2: string typeClassName;
} (thrift.uri = "facebook.com/thrift/annotation/java/Adapter")

@scope.Field
struct Wrapper {
  // Fully qualified name to a class that implements com.facebook.thrift.adaptor.Wrapper
  1: string wrapperClassName;

  // Fully qualified name the above implementation wraps to
  2: string typeClassName;
} (thrift.uri = "facebook.com/thrift/annotation/java/Wrapper")

@scope.Field
struct Recursive {}

// Note, previous Java codegenerator contains a bug where we use mangled rather than
// unmangled name for Field ID in unions and exceptions. Fixing it will be a breaking change.
// To unblock customer, We provide this annotation to guarantee correct behavior. This annotation
// is only applied in union field or struct field and is ignored if not in a struct field, where
// behavior is correct to begin with.
@scope.Field
struct FieldUseUnmangledName {}
