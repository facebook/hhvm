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

namespace cpp2 apache.thrift.detail.test

include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test/e2e/sink"

exception SinkItemException {
  @thrift.ExceptionMessage
  1: string message;
}

exception SinkFinalException {
  @thrift.ExceptionMessage
  1: string message;
}

exception SinkMethodException {
  @thrift.ExceptionMessage
  1: string message;
}

service TestSinkE2EService {
  sink<i32, string> echo(1: i32 count);
  string, sink<i32, string> echoWithResponse(1: i32 count);
  sink<
    i32 throws (1: SinkItemException sinkEx),
    string throws (1: SinkFinalException finalEx)
  > canThrow() throws (1: SinkMethodException methodEx);
}
