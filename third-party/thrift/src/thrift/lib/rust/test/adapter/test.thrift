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

include "thrift/annotation/rust.thrift"
include "thrift/annotation/scope.thrift"

@rust.Adapter{name = "::adapters::StringAdapter"}
typedef string AdaptedString

typedef AdaptedString WrappedAdaptedString (rust.newtype, rust.ord)

@rust.Adapter{name = "::adapters::NonZeroI64Adapter"}
typedef i64 AdaptedI64

typedef AdaptedI64 PassThroughAdaptedI64

typedef list<PassThroughAdaptedI64> NestedPassThroughAdaptedI64

typedef list<AdaptedString> AdaptedListNewType (rust.newtype)

typedef binary (rust.type = "Bytes") IOBuf

@rust.Adapter{name = "crate::types::IOBufIdentityAdapter"}
typedef IOBuf AdaptedBytes

typedef AdaptedBytes WrappedAdaptedBytes (rust.newtype)

@rust.Adapter{name = "crate::WrappedAdaptedBytesIdentityAdapter"}
typedef WrappedAdaptedBytes AdaptedWrappedAdaptedBytes

typedef AdaptedWrappedAdaptedBytes PassThroughAdaptedWrappedAdaptedBytes

typedef PassThroughAdaptedWrappedAdaptedBytes WrappedAdaptedWrappedAdaptedBytes (
  rust.newtype,
)

const AdaptedI64 adapted_int = 5;
const PassThroughAdaptedI64 pass_through_adapted_int = 6;
const WrappedAdaptedWrappedAdaptedBytes adapted_bytes_const = "some_bytes";
const AdaptedListNewType adapted_list_const = ["hello", "world"];

enum AssetType {
  UNKNOWN = 0,
  LAPTOP = 1,
  SERVER = 2,
} (rust.name = "ThriftAssetType")

@rust.Adapter{name = "crate::types::AssetAdapter"}
struct Asset {
  1: AssetType type_;
  2: i64 id;
  @rust.Adapter{name = "::adapters::NonZeroI64Adapter"}
  3: i64 id1 = 1;
  4: AdaptedI64 id2 = 2;
  @rust.Adapter{name = "::adapters::IdentityAdapter<>"}
  5: AdaptedI64 id3 = 3;
  6: string comment;
}

const Asset adapted_struct_const = {
  "type_": AssetType.SERVER,
  "id": 42,
  "comment": "foobar",
};

struct TestAnnotation {
  1: string payload;
}

@TestAnnotation{payload = "some_payload"}
struct Foo {
  1: string str_val;
  2: i64 int_val;
  @rust.Adapter{name = "::adapters::StringAdapter"}
  3: string str_val_adapted;
  @rust.Adapter{name = "::adapters::StringAdapter"}
  4: string str_val_adapted_default = "hello";
  @rust.Adapter{name = "::adapters::StringAdapter"}
  5: optional string str_val_adapted_optional;
  @rust.Adapter{name = "::adapters::NonZeroI64Adapter"}
  6: i64 validated_int_val = 1;
  @rust.Adapter{name = "::adapters::ListAdapter"}
  7: list<string> list_val = ["lorem", "ispum"];
  @rust.Adapter{name = "crate::types::FieldCheckerAdapter"}
  8: string field_checked;
  @rust.Adapter{name = "::adapters::IdentityAdapter<>"}
  9: string ident_field;
  10: AdaptedString typedef_str_val;
  11: AdaptedString typedef_str_val_default = "Hello";
  12: optional AdaptedString typedef_str_val_optional;
  13: PassThroughAdaptedI64 pass_through_adapted_i64 = 42;
  @rust.Adapter{name = "::adapters::IdentityAdapter<>"}
  14: PassThroughAdaptedI64 pass_through_adapted_and_field_i64 = 2;
  15: list<PassThroughAdaptedI64> adapted_int_list = [1, 2, 3];
  16: list<AdaptedString> adapted_string_list = ["hello", "world"];
  17: list<list<list<AdaptedString>>> nested_adapted_string_list = [
    [["hello", "world"]],
  ];
  18: map<
    string,
    list<list<NestedPassThroughAdaptedI64>>
  > nested_adapted_int_map = {
    "hello": [[[1, 2, 3], [4, 5, 6]]],
    "world": [[[7, 8, 9]]],
  };
  @rust.Adapter{name = "crate::types::AdaptedStringListIdentityAdapter"}
  19: list<AdaptedString> field_adapted_adapted_string_list = ["zucc"];
  20: Asset adapted_struct;
  @rust.Adapter{name = "crate::types::AdaptedAssetIdentityAdapter"}
  21: Asset double_adapted_struct;
  22: list<Asset> adapted_struct_list = [{"type_": AssetType.LAPTOP, "id": 10}];
  23: AdaptedListNewType adapted_list_new_type = ["hi", "there"];
  24: map<
    AdaptedString,
    WrappedAdaptedWrappedAdaptedBytes
  > map_with_adapted_key_val = {"what": "are those?"};
  25: map<
    WrappedAdaptedString,
    WrappedAdaptedWrappedAdaptedBytes
  > map_with_adapted_wrapped_key_val = {"marco": "polo"};
}

union Bar {
  @rust.Adapter{name = "::adapters::ListAdapter"}
  1: list<string> list_val;
  @rust.Adapter{name = "::adapters::StringAdapter"}
  2: string str_val;
  3: string str_val_not_adapted;
  @rust.Adapter{name = "::adapters::NonZeroI64Adapter"}
  4: i64 validated_int_val = 1;
}

struct FirstAnnotation {
  1: string uri;
}

@rust.Adapter{name = "crate::types::TransitiveTestAdapter<>"}
@FirstAnnotation{uri = "thrift/test"}
@scope.Transitive
struct TransitiveAdapterAnnotation {
  1: string payload;
}

@rust.Adapter{name = "crate::types::TransitiveAnnotationTestAdapter<>"}
@FirstAnnotation{uri = "thrift/transitive_field_test"}
@scope.Transitive
struct TransitiveFieldAdapterAnnotation {
  1: string payload;
}

@TransitiveAdapterAnnotation{payload = "hello_world"}
struct TransitiveStruct {
  @rust.Adapter{name = "crate::types::AnnotationTestAdapter"}
  1: string test_field;
  @TransitiveFieldAdapterAnnotation{payload = "foobar"}
  2: string test_field_2;
}

typedef TransitiveStruct TransitiveStructWrapper (rust.newtype)
