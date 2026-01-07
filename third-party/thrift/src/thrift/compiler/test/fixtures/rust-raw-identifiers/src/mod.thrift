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
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct ThereAreNoPascalCaseKeywords {
  1: bool return;
  @rust.Name{name = "super_"}
  2: bool super;
}

service Foo {
  void return(1: ThereAreNoPascalCaseKeywords bar);
  @rust.Name{name = "super_"}
  void super(1: ThereAreNoPascalCaseKeywords bar);
}
