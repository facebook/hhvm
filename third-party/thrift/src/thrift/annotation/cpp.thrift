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
namespace android com.facebook.thrift.annotation.cpp_deprecated
namespace js thrift.annotation.cpp
namespace py.asyncio facebook_thrift_asyncio.annotation.cpp
namespace go thrift.annotation.cpp
namespace py thrift.annotation.cpp
namespace hs Facebook.Thrift.Annotation.Cpp

// start

/**
 * Changes the name of the definition in generated C++ code.
 * In most cases a much better solution is to rename the problematic Thrift field itself. Only use the `@cpp.Name` annotation if such renaming is problematic,
 * e.g. when the field name appears in code as a string, particularly when using JSON serialization, and it is hard to change all usage sites.
 */
@scope.Definition
struct Name {
  1: string value;
}

/**
 * Changes the native type of a Thrift object (the C++ type used in codegen) to the value of the `name` field.
 * Container types may instead provide the `template` field, in which case template parameters will be filled in by thrift.
 * (e.g. `template = "folly::sorted_vector_set"` is equivalent to `type = "folly::sorted_vector_set<T>"` on `set<T>`)
 *
 * It is also possible to add `cpp_include` to bring in additional data structures and use them here.
 * It is required that the custom type matches the specified Thrift type even for internal container types.
 * Prefer types that can leverage `reserve(size_t)` as Thrift makes uses these optimizations.
 * *Special Case*: This annotation can be used to define a string/binary type as `IOBuf` or `unique_ptr<IOBuf>` so that you can leverage Thrift's support for zero-copy buffer manipulation through `IOBuf`.
 * During deserialization, thrift receives a buffer that is used to allocate the appropriate fields in the struct. When using smart pointers, instead of making a copy of the data, it only modifies the pointer to point to the address that is used by the buffer.
 *
 * The custom type must provide the following methods
 * * `list`: `push_back(T)`
 * * `map`: `insert(std::pair<T1, T2>)`
 * * `set`: `insert(T)`
 */
@scope.Typedef
@scope.Field
@scope.FunctionParameter
struct Type {
  1: string name;

  @Name{value = "template_"}
  2: string template;
}

/**
 * Allocates a field on the heap instead of inline.
 * This annotation is added to support recursive types. However, you can also use it to turn a field from a value to a smart pointer.
 * `@cpp.Ref` is equivalent having type`@cpp.RefType.Unique`.
 *
 * NOTE: A struct may transitively contain itself as a field only if at least one of the fields in the inclusion chain is either an optional Ref field or a container. Otherwise the struct would have infinite size.
 */
@scope.Field
struct Ref {
  1: RefType type; /// Optional, defaults to Unique
}
enum RefType {
  Unique = 0, /// `std::unique_ptr<T>`
  Shared = 1, /// `std::shared_ptr<const T> `
  SharedMutable = 2, /// `std::shared_ptr<T>`
}

/**
  Lazily deserialize large field on first access.

  ```
  FooWithLazyField foo;
  apache::thrift::CompactSerializer::deserialize(serializedData, foo);

  // large_field is lazy field, it will be deserialized on first access
  // The data will be deserialized in method call large_field_ref()
  LOG(INFO) << foo.large_field_ref()->size();

  // Result will be cached, we won't deserialize again
  LOG(INFO) << foo.large_field_ref()->size();
  ```

  Read more: /doc/fb/languages/cpp/lazy.md
*/

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

/**
* Packs isset bits into fewer bytes to save space at the cost of making access more expensive.
* Passing `atomic = false` reduces the access cost while making concurrent writes UB.
* Read more: /doc/fb/languages/cpp/isset-bitpacking.md
*/
@scope.Struct
struct PackIsset {
  1: bool atomic = true;
}

/**
  This annotation enables reordering of fields in the generated C++ struct to minimize padding.
  This is achieved by placing the fields in the order of decreasing alignments. The order of fields with the same alignment is preserved.

  ```
  @cpp.MinimizePadding
  struct Padded {
    1: byte small
    2: i64 big
    3: i16 medium
    4: i32 biggish
    5: byte tiny
  }
  ```

  For example, the C++ fields for the `Padded` Thrift struct above will be generated in the following order:

  ```
  int64_t big;
  int32_t biggish;
  int16_t medium;
  int8_t small;
  int8_t tiny;
  ```

  which gives the size of 16 bytes compared to 32 bytes if `cpp.MinimizePadding` was not specified.
*/
@scope.Struct
@scope.Exception
struct MinimizePadding {}

@scope.Union
struct ScopedEnumAsUnionType {}

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
 * 64-bit is not supported to avoid truncation since enums are sent as 32-bit integers over the wire.
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

/**
 * Causes C++ handler code to run inline on the EventBase thread.
 * Disables overload protection, use with caution.
 * Cannot be applied to individual functions in interactions.
 *
 * Causes the request to be executed on the event base thread directly instead of rescheduling onto a thread manager thread, provided the async_eb_ handler method is implemented.
 * You should only execute the request on the event base thread if it is very fast and you have measured that rescheduling is a substantial chunk of your service's CPU usage.
 * If a request executing on the event base thread blocks or takes a long time, all other requests sharing the same event base are affected and latency will increase significantly.
 * We strongly discourage the use of this annotation unless strictly necessary. You will have to implement the harder-to-use async_eb_ handler method.
 * This also disables queue timeouts, an important form of overload protection.
 */
@scope.Function
@scope.Interaction
struct ProcessInEbThreadUnsafe {}

/**
 * Causes uses of the given structured type to be replaced with `CursorSerializationWrapper` to allow use of cursor-based serialization.
 * Must add `cpp_include "thrift/lib/cpp2/protocol/CursorBasedSerializer.h"` to files that use this annotation.
 * See documentation for this class in CursorBasedSerializer.h
 * Can only be applied to top-level structs (used as return type or sole argument to an RPC or serialized directly), not to types used as struct fields or container elements.
 */
@scope.Struct
@scope.Union
@scope.Typedef
@scope.Transitive
@Adapter{name = "::apache::thrift::CursorSerializationAdapter"}
struct UseCursorSerialization {}

/**
 * Given either of the following Thrift service definitions:
 *
 *     @cpp.GenerateDeprecatedHeaderClientMethods
 *     service Foo {
 *       void bar();
 *     }
 *
 *     service Foo {
 *       @cpp.GenerateDeprecatedHeaderClientMethods
 *       void bar();
 *     }
 *
 * This annotation instructs the compiler to generate the following (now deprecated) client method variants:
 *   - apache::thrift::Client<Foo>::header_future_bar
 *   - apache::thrift::Client<Foo>::header_semifuture_bar
 */
@scope.Service
@scope.Function
struct GenerateDeprecatedHeaderClientMethods {}

/**
 * Allows the field to be annotated @cpp.Ref (or cpp[2].ref[_type]) even if it
 * is not optional (or in a union, which is effectively optional).
 *
 * This annotation is provided for a limited time, to exempt pre-existing fields
 * while rolling out a stricter enforcement of the condition above.
 *
 * Reminder: reference fields should be optional because the corresponding smart
 * pointers (std::unique_ptr, std::shared_ptr) can always be reset or set to
 * nullptr by the clients. If the field is not optional, this leads to a
 * confusing (or non-sensical) situation, wherein a field that should always
 * have a value has nullptr instead.
 */
@scope.Field
struct AllowLegacyNonOptionalRef {}

/**
 * Changes the serialization behavior of the field to skip serialization if its value is equal to standard default
 * for bool, integers, enums, and floating points, and intrinsic default for string, binary, and containers.
 * If an unqualified field with structured type has @cpp.Ref, the serilaization skips the field if the pointer is empty.
 *
 * This annotation is only applicable to an unqualified field with bool, integers, enums, floating points, string, binary, and containers
 * but not for structs, unions, and exceptions.
 *
 * Note, this is a deprecated feature. Use @thrift.TerseWrite instead.
 */
@scope.Field
struct DeprecatedTerseWrite {}

/**
 * Allows the field to be annotated @cpp.Ref (or cpp[2].ref[_type]) even if it
 * is deprecated_terse_writes field.
 *
 * This annotation is provided for a limited time, to exempt pre-existing fields
 * while rolling out a stricter enforcement of the condition above.
 */
@scope.Field
struct AllowLegacyDeprecatedTerseWritesRef {}

/**
 * If there are custom types in thrift structure (e.g., `std::unordered_map` field),
 * We won't define `operator<` automatically (unless URI exists, but that's about
 * to change). Note that `operator<` is always declared, unless `@cpp.NonOrderable` is used.
 * This annotation ensures the `operator<` is always defined. For types that
 * don't have `operator<`, such as `std::unordered_map`, we will convert it to
 * a sorted `std::vector<pair<K*, V*>>` to do the comparison.
 */
@scope.Program
@scope.Structured
struct EnableCustomTypeOrdering {}

/**
 * When applied to a service, this annotation will cause the thrift compiler
 * to generate the method decorator interface for the class.
 */
@scope.Service
struct GenerateServiceMethodDecorator {}

/**
 * Marks a structured type as non-orderable, marking `operator<` as deleted.
 * This is useful when types should never be ordered. By default, `operator<` is
 * always declared, but depending on whether or not the type's shape is considered
 * orderable, it may or may not be defined. See `EnableCustomTypeOrdering` for more
 * details.
 * Note: The unstructured `cpp_noncomparable` annotation has priority over this one &
 * will cause `operator==` && `operator<` to *not* be declared or defined.
 */
@scope.Structured
struct NonOrderable {}

/**
 * Declares a specialization of `std::hash` for the struct or type.
 * This is equivalent to the unstructured `cpp.declare_hash` annotation.
 * The generated code will include a forward declaration of the hash specialization in the std namespace.
 */
@scope.Structured
@scope.Typedef
struct DeclareHashSpecialization {}

/**
 * Declares a specialization of `std::equal_to` for the struct or type.
 * This is equivalent to the unstructured `cpp.declare_equal_to` annotation.
 * The generated code will include a forward declaration of the equal_to specialization in the std namespace.
 */
@scope.Structured
@scope.Typedef
struct DeclareEqualToSpecialization {}
