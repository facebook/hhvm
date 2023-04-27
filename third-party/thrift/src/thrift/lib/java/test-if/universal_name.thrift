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

package "test.dev/thrift/lib/java/test/universalname"

namespace java.swift com.facebook.thrift.test.universalname

struct TestRequest {
  1: bool aBool;
  3: i64 aLong;
  5: string aString;
} (thrift.uri = "test.dev/thrift/lib/java/my_request")

struct TestResponse {
  1: list<string> aList;
  2: string aString;
}

union TestUnion {
  1: i32 aInt;
  2: string aString;
  3: TestRequest aStruct;
}

exception TestException {
  1: string message;
} (thrift.uri = "test.dev/thrift/lib/java/my_exp")

service UNService {
  TestResponse test(1: TestRequest request) throws (1: TestException ex);
}
