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

// start

/**
 * Annotations that indicate which IDL definition a structured annotation can
 * be placed on.
 *
 * For example:
 *   include "thrift/annotation/scope.thrift"
 *
 *   @scope.Struct
 *   struct MyStructAnnotation {...}
 *
 *   @MyStructAnnotation // Good.
 *   struct Foo{
 *     @MyStructAnnotation // Compile-time failure. MyStructAnnotation is not
 *                         // allowed on fields.
 *     1: i32 my_field;
 *   }
 */
package "facebook.com/thrift/annotation"

namespace java com.facebook.thrift.annotation_deprecated
namespace android com.facebook.thrift.annotation_deprecated
namespace js thrift.annotation.scope
namespace py.asyncio facebook_thrift_asyncio.annotation.scope
namespace go thrift.annotation.scope
namespace py thrift.annotation.scope

/**
 * Indicates that the scope of sibling annotations is transitive.
 *
 * For example:
 *
 *     @scope.Struct
 *     @scope.Union
 *     @scope.Exception
 *     @scope.Transitive
 *     struct Structured {}
 *
 * Annotating a Thrift struct with @Structured automatically applies
 * @scope.Struct, @scope.Union and @scope.Exception annotations, i.e.
 *
 *     @Structured
 *     struct MyAnnotation {}
 *
 * is equivalent to
 *
 *     @scope.Struct
 *     @scope.Union
 *     @scope.Exception
 *     struct MyAnnotation {}
 *
 */
struct Transitive {}

/**
 * The Program scope.
 *
 * This allows annotations on the `package` definition, which implies the
 * annotaiton applies to the entire program.
 */
struct Program {}

/** The `struct` definition scope. */
struct Struct {}

/** The `union` definition scope. */
struct Union {}

/** The `exception` definition scope. */
struct Exception {}

/** Field declartaions, for example in `struct` or `function` declartions. */
struct Field {}

/** The `typedef` definition scope. */
struct Typedef {}

/** The `service` definition scope. */
struct Service {}

/** The `interaction` definition scope. */
struct Interaction {}

/** The `function` definition scope. */
struct Function {} (hack.name = "TFunction", js.name = "TFunction")

/** The Enum value definition scope. */
struct EnumValue {}

/** The `const` definition scope. */
struct Const {} (hack.name = "TConst")

// Due to cython bug, we can not use `Enum` as class name directly
// https://github.com/cython/cython/issues/2474
struct Enum {} (thrift.uri = "facebook.com/thrift/annotation/Enum", py3.hidden)

/** A scope that includes all 'structured' definitions. */
@Struct
@Union
@Exception
@Transitive
struct Structured {}

/** A scope that includes all 'interface' definitions. */
@Service
@Interaction
@Transitive
struct Interface {} (hack.name = "TInterface")

/** A scope that includes all program-scoped definition. */
@Structured
@Interface
@Typedef
@Enum
@Const
@Transitive
struct RootDefinition {}

/** A scope that includes all definitions. */
@RootDefinition
@Field
@Function
@EnumValue
@Transitive
struct Definition {}
