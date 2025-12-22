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

namespace cpp2 apache.thrift.test
namespace py3 thrift.test

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

enum MyEnum {
  @cpp.Name{value = "REALM"}
  DOMAIN = 1,
}

struct MyStruct {
  @cpp.Name{value = "unique_name"}
  1: i64 conflicting_name;
  @cpp.Name{value = "opt_unique_name"}
  2: optional i64 opt_conflicting_name;
}

service MyService {
  @cpp.Name{value = "cppDoNothing"}
  void doNothing();
}
