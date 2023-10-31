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
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/annotation/cpp"

namespace java com.facebook.thrift.annotation.cpp_deprecated
namespace js thrift.annotation.cpp
namespace py.asyncio facebook_thrift_asyncio.annotation.cpp
namespace go thrift.annotation.cpp
namespace py thrift.annotation.cpp

/**
 * Changes the native type of a Thrift object.
 * `name` and `template` correspond to `cpp.type` and `cpp.template` respecively.
 */
@scope.Typedef
@scope.Field
struct Type {
  1: string name;
  2: string template (cpp.name = "template_");
}

enum RefType {
  Unique = 0,
  Shared = 1,
  SharedMutable = 2,
}

@scope.Field
struct Ref {
  1: RefType type;
}

/**
 * Changes the name of the definition in generated C++ code.
 */
@scope.Definition
struct Name {
  1: string value;
}

@scope.Field
struct Lazy {
  // Use std::unique_ptr<folly::IOBuf> instead of folly::IOBuf to store serialized data.
  1: bool ref = false;
}

@scope.Struct
struct DisableLazyChecksum {}

/**
 * An annotation that applies a C++ adapter to typedef, field, or struct.
 *
 * For example:
 *
 *   @cpp.Adapter{name = "::ns::IdAdapter"}
 *   typedef i64 MyI64;
 *
 * Here the type `MyI64` has the C++ adapter `IdAdapter`.
 *
 *   struct User {
 *     @cpp.Adapter{name = "::ns::IdAdapter"}
 *     1: i64 id;
 *   }
 *
 * Here the field `id` has the C++ adapter `IdAdapter`.
 */
@scope.Field
@scope.Typedef
@scope.Structured
@scope.Const
struct Adapter {
  /**
   * The name of a C++ adapter type used to convert between Thrift and native
   * C++ representation.
   *
   * The adapter can be either a Type or Field adapter, providing either of the following APIs:
   *
   *     struct ThriftTypeAdapter {
   *       static AdaptedType fromThrift(ThriftType thrift);
   *       static {const ThriftType& | ThriftType} toThrift(const AdaptedType& native);
   *     };
   *
   *     struct ThriftFieldAdapter {
   *       // Context is an instantiation of apache::thrift::FieldContext
   *       template <class Context>
   *       static void construct(AdaptedType& field, Context ctx);
   *
   *       template <class Context>
   *       static AdaptedType fromThriftField(ThriftType value, Context ctx);
   *
   *       template <class Context>
   *       static {const ThriftType& | ThriftType} toThrift(const AdaptedType& adapted, Context ctx);
   *     };
   */
  1: string name;

  /**
   * It is sometimes necessary to specify AdaptedType here (in case the codegen would
   * have a circular depdenceny, which will cause the C++ build to fail).
   */
  2: string adaptedType;

  /**
   * The name and/or extra namespace to use when directly adapting a type
   * (as opposed a typedef).
   *
   * In this case, the IDL name of the type will refer to the adapted type in
   * C++ and the underlying thrift type will be generated in a nested
   * namespace and/or with a different name.
   *
   * If neither `underlyingName` or `extraNamespace` is provided, the
   * underlying type will be generated in a nested 'detail' namespace with
   * the same name.
   */
  3: string underlyingName;
  4: string extraNamespace;

  /** Must set to true when adapted type is not copyable. */
  5: bool moveOnly;
}

@scope.Struct
struct PackIsset {
  1: bool atomic = true;
}

@scope.Struct
struct MinimizePadding {}

@scope.Struct
struct TriviallyRelocatable {}

@scope.Union
struct ScopedEnumAsUnionType {}

/**
 * Indicates a typedef should be 'strong', and require an explicit cast to
 * the underlying type.
 *
 * Currently only works for integer typedefs, for example:
 *
 *     @cpp.StrongType
 *     typedef i32 MyId;
 *
 * Will cause an enum class to be used instead of a typedef in the genearte code, for example:
 *
 *     enum class MyId : ::std::int32_t {};
 *
 */
@thrift.Experimental
@scope.Typedef
struct StrongType {}

/**
 * An annotation that intercepts field access with C++ field interceptor.
 * Use with *caution* since this may introduce substantial performance overhead on each field access.
 *
 * For example:
 *
 *     struct Foo {
 *       @cpp.FieldInterceptor{name = "MyFieldInterceptor"}
 *       1: i64 id;
 *     }
 *
 * The field interceptor `MyFieldInterceptor` will intercept with `interceptThriftFieldAccess`
 * when the field `id` is accessed.
 */
@scope.Field
@thrift.Experimental
struct FieldInterceptor {
  /**
   * The name of a field interceptor.
   *
   * The field interceptor provides the following API:
   *
   *     struct ThriftFieldInterceptor {
   *       template <typename T, typename Struct, int16_t FieldId>
   *       static void interceptThriftFieldAccess(T&& field,
   *                                              apache::thrift::FieldContext<Struct, FieldId>&& ctx);
   *     };
   *
   * The field interceptor intercepts with the field value and the field context.
   * It enforces an easily searchable function name `interceptThriftFieldAccess`.
   */
  1: string name;

  /**
   * Setting to true makes compiler not inline and erase function signature for
   * the intercepting field accessor.
   */
  2: bool noinline;
}

@scope.Program
@scope.Structured
struct UseOpEncode {}

/**
 * Enum in C++ by default uses signed 32 bit integer. There is no need to specify
 * underlying type for signed 32 bit integer.
 */
enum EnumUnderlyingType {
  /** ::std::int8_t */
  I8 = 0,
  /** ::std::uint8_t */
  U8 = 1,
  /** ::std::int16_t */
  I16 = 2,
  /** ::std::uint16_t */
  U16 = 3,
  /** ::std::uint32_t */
  U32 = 4,
}

/**
 * Indicates an integer type for C++ to use as the underlying type of enum, for example:
 *
 *     @cpp.EnumType{type = cpp.EnumUnderlyingType.I8}
 *     enum Fruit {
 *       Apple = 0,
 *       Banana = 1,
 *     }
 *
 * will be generated into the following:
 *
 *     enum class Fruit : ::std::int8_t {
 *       Apple = 0,
 *       Banana = 1,
 *     };
 *
 */
@scope.Enum
struct EnumType {
  1: EnumUnderlyingType type;
}

/**
 * Indicates that frozen types should not be generated for a given struct.
 */
@scope.Structured
struct Frozen2Exclude {}

/**
 * Indicates that the container params must be complete at the time this type is instantiated.
 * Only required in rare cases where the build fails with a frozen-related assert failure.
 */
@scope.Typedef
struct Frozen2RequiresCompleteContainerParams {}
