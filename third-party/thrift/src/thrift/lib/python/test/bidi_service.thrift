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
namespace py3 thrift.python

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

struct SinkChunk {
  1: string value;
}

struct StreamChunk {
  1: string value;
}

struct FirstRequest {
  1: string value;
}

struct FirstResponse {
  1: string value;
}

exception SinkException {
  @thrift.ExceptionMessage
  1: string message;
}

exception StreamException {
  @thrift.ExceptionMessage
  1: string message;
}

exception MethodException {
  @thrift.ExceptionMessage
  1: string message;
}

service TestBidiService {
  // @lint-ignore THRIFTCHECKS new unreleased feature
  sink<string>, stream<string> echo(1: double serverDelay);

  // @lint-ignore THRIFTCHECKS new unreleased feature
  string, sink<string>, stream<string> echoWithResponse(1: string initial);

  // @lint-ignore THRIFTCHECKS new unreleased feature
  sink<string>, stream<i32> intStream(double serverDelay);

  // @lint-ignore THRIFTCHECKS new unreleased feature
  FirstResponse, sink<SinkChunk>, stream<StreamChunk> structBidi(
    1: FirstRequest request,
  );

  // Method that throws an exception:
  // if where is 0, throws on the method
  // if where is 1, throws on the stream
  // @lint-ignore THRIFTCHECKS new unreleased feature
  sink<i64 throws (1: SinkException sinkEx)>, stream<
    i64 throws (1: StreamException sinkEx)
  > canThrow(1: i32 where) throws (1: MethodException methodEx);
}
