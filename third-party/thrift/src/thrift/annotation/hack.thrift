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

package "facebook.com/thrift/annotation/hack"

namespace py thrift.annotation.hack
namespace java com.facebook.thrift.annotation.hack_deprecated
namespace android com.facebook.thrift.annotation.hack_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.hack
namespace go thrift.annotation.hack
namespace hs2 facebook.thrift.annotation.hack

// start

/// An experimental annotation that applies a Hack wrapper to fields.
/// For example:
///
///   struct User {
///     @hack.FieldWrapper{name="MyWrapper"}
///     1: i64 id;
///   }
@scope.Field
struct FieldWrapper {
  /// The name of a Hack wrapper class used to wrap the field
  1: string name;
}

/// An annotation that applies a Hack wrapper to fields, typedef or structs.
/// For example:
///
///   struct User {
///     @hack.FieldWrapper{name="MyWrapper"}
///     1: i64 id;
///   }
@scope.Typedef
@scope.Struct
@scope.Field
struct Wrapper {
  /// The name of a Hack wrapper class used to wrap the field
  1: string name;
  /// When applied directly to a typedef or struct, the IDL name of the
  /// type will refer to the adapted type in Hack and the underlying thrift struct will be
  /// generated in a nested namespace and/or with a different name. By default the type/struct
  /// will be generated in a nested 'thrift_adapted_types' namespace with the same name,
  /// but both of these can be changed by setting these fields.
  /// Empty string enables the nested namespace and uses the IDL name for the struct.
  2: string underlyingName;
  3: string extraNamespace = "thrift_adapted_types";
} (thrift.uri = "facebook.com/thrift/annotation/hack/Wrapper")

/// An annotation that applies a Hack adapter to types. For example:
/// @hack.Adapter{name="\\TimestampAdapter"}
/// typedef i64 Timestamp;
///
///   struct User {
///     1: Timestamp account_creation_time;
///   }
///
/// Here the field `account_creation_time` will have type TimestampAdapter::THackType instead of i64.
///
/// in hack:
/// ```
/// final class TimestampAdapter implements IThriftAdapter {
///   const type TThriftType = int;
///   const type THackType = Time;
///   public static function fromThrift(int $seconds)[]: Time {
///     return Time::fromEpochSeconds($seconds);
///   }
///   public static function toThrift(Time $time): int {
///     return $hack_value->asFullSecondsSinceEpoch();
///   }
/// }
/// ```
/// elsewhere in hack:
/// ```
/// function timeSinceCreated(Document $doc): Duration {
///   // $doc->created_time is of type Time
///   return Duration::between(Time::now(), $doc->created_time);
/// }
/// ```
/// This completely replaces the underlying type of a thrift for a custom implementation and uses
/// the specified adapter to convert to and from the underlying Thrift type during (de)serialization.
@scope.Typedef
@scope.Field
struct Adapter {
  /// The name of a Hack adapter class that implements IThriftAdapter
  1: string name;
}

@scope.Typedef
@scope.Field
@scope.Function
struct SkipCodegen {
  1: string reason;
}

/// This annotation is mainly used to rename symbols which can result in symbol
/// conflict errors in Hack codegen.
/// For ex: reserved keywords in Hack language, symbols with similar names from
/// other files in Hack
@scope.Definition
struct Name {
  1: string name;
  2: string reason;
}

/// This annotation is for adding Hack attributes to union enums.
@scope.Union
struct UnionEnumAttributes {
  1: list<string> attributes;
}

/// This annotation is for using a custom trait for structs.
@scope.Struct
@scope.Union
@scope.Exception
struct StructTrait {
  1: string name;
}

/// This annotation is for adding Hack attributes.
/// * Where to use: field or struct type
/// * Value: add attributes like `JSEnum` to structs or fields
/// * Example:

/// ```
/// // In thrift
/// enum MyEnum {
///   ALLOWED = 1,
///   THIS_IS_ALLOWED  =  2,
///   THIS_IS_ALLOWED_2 = 3,
/// }(
///   hack.attributes=
///     "\JSEnum(shape('name' => 'MyEnum')),
///     \GraphQLEnum('MyEnum', 'Description for my enum',)"
/// )
/// struct MyThriftStruct {
///   1: string foo (hack.attributes = "FieldAttribute");
///   2: string bar;
///   3: string baz;
/// } (hack.attributes = "ClassAttribute")
/// ```
/// ```
/// //thrift compiler will generate this for you
/// <<\JSEnum(shape('name' => 'MyEnum')),
/// \GraphQLEnum('MyEnum', 'Description for my enum',)>>
/// enum MyEnum: int {
///  ALLOWED = 1;
///  THIS_IS_ALLOWED = 2;
///  THIS_IS_ALLOWED_2 = 3;
/// }
/// <<ClassAttribute>>
/// class MyThriftStruct implements \IThriftStruct {
///  ....
///  <<FieldAttribute>>
///  public string $foo;
///  ....
/// }
/// ```
struct Attributes {
  1: list<string> attributes;
}

@scope.Struct
@scope.Union
@scope.Exception
struct StructAsTrait {}

/// This annotation is to generate an entity as internal
@scope.Struct
@scope.Union
@scope.Enum
@scope.Field
@scope.Typedef
@scope.Function
@scope.Service
struct ModuleInternal {}
