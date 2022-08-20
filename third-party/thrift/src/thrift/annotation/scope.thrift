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

package "facebook.com/thrift/annotation"

namespace java com.facebook.thrift.annotation_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.scope
namespace go thrift.annotation.scope
namespace py thrift.annotation.scope

// Annotations that indicate which IDL definition a structured annotation can
// be place on.
//
// For example:
//     include "thrift/annotation/scope.thrift"
//
//     @scope.Struct
//     struct MyStructAnnotation {...}
//
//     @MyStructAnnotation // Good.
//     struct Foo{
//       @MyStructAnnotation // Compile-time failure. MyStructAnnotation is not
//                           // allowed on fields.
//       1: i32 my_field;
//     }
struct Program {} (thrift.uri = "facebook.com/thrift/annotation/Program")
struct Struct {} (thrift.uri = "facebook.com/thrift/annotation/Struct")
struct Union {} (thrift.uri = "facebook.com/thrift/annotation/Union")
struct Exception {} (thrift.uri = "facebook.com/thrift/annotation/Exception")
struct Field {} (thrift.uri = "facebook.com/thrift/annotation/Field")
struct Typedef {} (thrift.uri = "facebook.com/thrift/annotation/Typedef")
struct Service {} (thrift.uri = "facebook.com/thrift/annotation/Service")
struct Interaction {} (
  thrift.uri = "facebook.com/thrift/annotation/Interaction",
)
struct Function {} (
  thrift.uri = "facebook.com/thrift/annotation/Function",
  hack.name = "TFunction",
)
struct EnumValue {} (thrift.uri = "facebook.com/thrift/annotation/EnumValue")
struct Const {} (
  thrift.uri = "facebook.com/thrift/annotation/Const",
  hack.name = "TConst",
)

// Indicates that a definition should be included in the runtime schema.
//
// See thrift/lib/thrift/schema.thrift
struct Schema {} (thrift.uri = "facebook.com/thrift/annotation/Schema")

// Due to cython bug, we can not use `Enum` as class name directly
// https://github.com/cython/cython/issues/2474
struct FbthriftInternalEnum {} (
  thrift.uri = "facebook.com/thrift/annotation/Enum",
)
typedef FbthriftInternalEnum Enum (thrift.uri = "")

// Indicates that the scope of sibling annotations is transitive.
//
// For example:
//
//     @scope.Struct
//     @scope.Union
//     @scope.Exception
//     @scope.Transitive
//     struct Structured {}
//
// Annotating a Thrift struct with @Structured automatically applies
// @scope.Struct, @scope.Union and @scope.Exception annotations, i.e.
//
//     @Structured
//     struct MyAnnotation {}
//
// is equivalent to
//
//     @scope.Struct
//     @scope.Union
//     @scope.Exception
//     struct MyAnnotation {}
//
struct Transitive {} (thrift.uri = "facebook.com/thrift/annotation/Transitive")

@Struct
@Union
@Exception
@Transitive
struct Structured {}

@Service
@Interaction
@Transitive
struct Interface {} (hack.name = "TInterface")

@Structured
@Interface
@Typedef
@FbthriftInternalEnum // TODO(afuller): Use the enum typedef directly.
@Const
@Transitive
struct RootDefinition {}

@RootDefinition
@Field
@Function
@EnumValue
@Transitive
struct Definition {}
