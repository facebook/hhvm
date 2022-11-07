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

package "test.dev/thrift/lib/java/test/rocket"

namespace java.swift com.facebook.thrift.test.rocket

struct TestRequest {
  1: i32 int_field;
  2: optional string str_field;
}

struct TestRequest2 {
  10: i16 short_field;
  11: optional string str_field;
}

struct TestResponse {
  3: i32 int_field;
  4: optional string str_field;
}

struct InitialTestResponse {
  7: i32 int_field;
}

exception TestException {
  1: string msg;
}

exception TestFunctionException {
  9: string msg;
}

service TestService {
  void requestResponseVoid(1: TestRequest request);
  TestResponse requestResponse(1: TestRequest request);

  stream<TestResponse> streamResponse(1: TestRequest request);
  InitialTestResponse, stream<TestResponse> streamInitialResponse(
    1: TestRequest request,
  );
  stream<TestResponse> streamResponseNull(1: TestRequest request);
  stream<TestResponse> streamResponseEmpty(1: TestRequest request);

  stream<TestResponse throws (1: TestException e)> streamDeclaredException(
    1: TestRequest request,
  );
  stream<TestResponse throws (2: TestException e)> streamDeclaredException2(
    1: TestRequest request,
  );
  stream<
    TestResponse throws (3: TestException e)
  > streamDeclaredAndFunctionException(1: TestRequest request) throws (
    6: TestFunctionException e1,
  );
  stream<
    TestResponse throws (7: TestException e)
  > streamDeclaredExceptionFluxError(1: TestRequest request);

  stream<TestResponse throws (5: TestException e)> streamUndeclaredException(
    1: TestRequest request,
  );
  stream<TestResponse throws (6: TestException e)> streamUndeclaredException2(
    1: TestRequest request,
  );
  stream<
    TestResponse throws (8: TestException e)
  > streamUndeclaredAndFunctionException(1: TestRequest request) throws (
    3: TestFunctionException e1,
  );

  stream<
    TestResponse throws (10: TestException e)
  > streamDeclaredExceptionMultiInput(
    2: TestRequest request,
    5: TestRequest2 request2,
  );

  InitialTestResponse, stream<
    TestResponse throws (11: TestException e)
  > streamInitialResponseDeclaredException(1: TestRequest request);
  InitialTestResponse, stream<
    TestResponse throws (12: TestException e)
  > streamInitialResponseDeclaredAndFunctionException(
    1: TestRequest request,
  ) throws (13: TestFunctionException e1);
}
