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
