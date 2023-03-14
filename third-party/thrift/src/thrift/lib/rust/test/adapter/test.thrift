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

@rust.Adapter{name = "::adapters::StringAdapter"}
typedef string AdaptedString

@rust.Adapter{name = "::adapters::NonZeroI64Adapter"}
typedef i64 AdaptedI64

typedef AdaptedI64 PassThroughAdaptedI64

typedef list<PassThroughAdaptedI64> NestedPassThroughAdaptedI64

typedef list<AdaptedString> AdaptedListNewType (rust.newtype)

typedef binary (rust.type = "bytes::Bytes") IOBuf

@rust.Adapter{name = "crate::IOBufIdentityAdapter"}
typedef IOBuf AdaptedBytes

typedef AdaptedBytes WrappedAdaptedBytes (rust.newtype)

@rust.Adapter{name = "crate::WrappedAdaptedBytesIdentityAdapter"}
typedef WrappedAdaptedBytes AdaptedWrappedAdaptedBytes

typedef AdaptedWrappedAdaptedBytes PassThroughAdaptedWrappedAdaptedBytes

typedef PassThroughAdaptedWrappedAdaptedBytes WrappedAdaptedWrappedAdaptedBytes (
  rust.newtype,
)

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
  @rust.Adapter{name = "crate::FieldCheckerAdapter"}
  8: string field_checked;
  @rust.Adapter{name = "::adapters::IdentityAdapter<>"}
  9: string ident_field;
  10: AdaptedString typedef_str_val;
  11: PassThroughAdaptedI64 pass_through_adapted_i64 = 42;
  @rust.Adapter{name = "::adapters::IdentityAdapter<>"}
  12: PassThroughAdaptedI64 pass_through_adapted_and_field_i64 = 2;
  13: list<PassThroughAdaptedI64> adapted_int_list = [1, 2, 3];
  14: list<AdaptedString> adapted_string_list = ["hello", "world"];
  15: list<list<list<AdaptedString>>> nested_adapted_string_list = [
    [["hello", "world"]],
  ];
  16: map<
    string,
    list<list<NestedPassThroughAdaptedI64>>
  > nested_adapted_int_map = {
    "hello": [[[1, 2, 3], [4, 5, 6]]],
    "world": [[[7, 8, 9]]],
  };
  @rust.Adapter{name = "crate::AdaptedStringListIdentityAdapter"}
  17: list<AdaptedString> field_adapted_adapted_string_list = ["zucc"];
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
