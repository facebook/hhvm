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

namespace go 'github.com/facebook/fbthrift/thrift/test/go/if/dummy'

// !!! DO NOT MODIFY !!!
//
// This schema is used to create dummy codegen for testing
// server/client implementations in 'thrift/lib/go/thrift'.
// It needs to stay simple - the way it currently is.

struct DummyStruct1 {
  1: byte field1;
  2: bool field2;
  3: i16 field3;
  4: i32 field4;
  5: i64 field5;
  6: float field6;
  7: double field7;
  8: binary field8;
  9: string field9;
}

service Dummy {
  string Echo(1: string value);
  oneway void OnewayRPC(1: string value);
  void Sleep(1: i64 milliseconds);
}
