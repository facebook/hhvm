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

include "thrift/annotation/hack.thrift"

// Program-level prefix with apply_on_getName=false and skip_services=true.
// Non-service generated class names are prefixed, but getName() returns the
// unprefixed base name. Service-generated types are not prefixed at all.
@hack.NamePrefix{prefix = "TMyPrefix_", apply_on_getName = false}
@hack.LegacyOmitPrefixInNameString
package "facebook.com/thrift/test/fixtures/hack_name_prefix_omit_from_name_string"

namespace hack test.fixtures.hack_name_prefix_omit_from_name_string

enum Status {
  Unknown = 0,
  Active = 1,
}

struct MyStruct {
  1: string str_value;
  2: i32 int_value;
}

exception MyException {
  1: string message;
}

service MyService {
  MyStruct getStruct(1: i32 id) throws (1: MyException ex);
  void ping();
}
