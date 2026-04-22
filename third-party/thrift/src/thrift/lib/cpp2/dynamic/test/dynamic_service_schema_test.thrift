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

package "facebook.com/thrift/dynamic_service_schema_test"

exception TestException {
  1: string message;
}

service DynamicServiceSchemaTestService {
  i32 add(1: i32 a, 2: i32 b);
  void ping();
  string greet(1: string name) throws (1: TestException e);
  i32, stream<string> streamNames(1: i32 count);
  sink<string, i32> collectStrings();
}
