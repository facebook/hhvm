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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 apache.thrift.python.test
namespace py thrift.py.test
namespace py3 thrift.python

exception EmptyException {}

exception ArithmeticException {
  1: string msg;
}

service TestService {
  i32 add(1: i32 num1, 2: i32 num2);
  double divide(1: double dividend, 2: double divisor) throws (
    1: ArithmeticException ae,
  );
  void noop();
  void oops() throws (1: EmptyException ee);
  oneway void oneway();
  void surprise();
  string readHeader(1: string key);
  stream<SimpleResponse throws (1: ArithmeticException e)> nums(
    1: i32 f,
    2: i32 t,
  ) throws (1: ArithmeticException ee);
  i64, stream<SimpleResponse> sumAndNums(1: i32 f, 2: i32 t) throws (
    1: ArithmeticException ee,
  );
  SimpleResponse, sink<EmptyChunk, SimpleResponse> dumbSink(1: string hi);

  // Note the Python sink client serializes payloads, and the Python server
  // deserializes them before giving them to the handler. But in this test,
  // the OmniClient is serializing, but the C++ server doesn't have the deserialize
  // logic, so we have to manually simulate the deserialization inside the handler.
  sink<PyBuffer, PyBuffer> countSinkPyBuf(1: i32 from, 2: i32 to) throws (
    1: ArithmeticException e,
  );
}

service EchoService extends TestService {
  string echo(1: string input);
}

@cpp.Type{name = "folly::IOBuf"}
typedef binary PyBuffer

//
// The following structures are defined to mimic the anonymous argument structs
// for the related service functions. These request structures are used to test
// the `OmniClient` so that we can easily construct the arguments.
//
struct AddRequest {
  1: i32 num1;
  2: i32 num2;
}

struct EmptyRequest {
  1: string hi;
}

struct SimpleResponse {
  1: string value;
}

struct EmptyChunk {}

struct StreamChunk {
  1: i32 value;
}

struct ReadHeaderRequest {
  1: string key;
}

struct NumsRequest {
  1: i32 f;
  2: i32 t;
}
