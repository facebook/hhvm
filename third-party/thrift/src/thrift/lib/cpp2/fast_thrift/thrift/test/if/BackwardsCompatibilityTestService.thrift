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

package "facebook.com/thrift/fast_thrift/thrift/test"

namespace cpp2 apache.thrift.fast_thrift.thrift.test

service BackwardsCompatibilityTestService {
  // Simple echo - return the input string
  string echo(1: string message);

  // Add two numbers
  i64 add(1: i64 a, 2: i64 b);

  // Return a response with the given size
  string sendResponse(1: i64 size);

  // Void response for simple connectivity test
  void ping();

  // Throws TApplicationException for error handling tests
  void throwError(1: string message);
}
