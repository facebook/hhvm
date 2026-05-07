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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test/e2e/bidi"

exception BiDiSinkException {
  @thrift.ExceptionMessage
  1: string message;
}

exception BiDiStreamException {
  @thrift.ExceptionMessage
  1: string message;
}

exception BiDiMethodException {
  @thrift.ExceptionMessage
  1: string message;
}

service TestBiDiService {
  sink<string>, stream<string> echo();

  string, sink<string>, stream<string> echoWithResponse(1: string initial);

  sink<i64>, stream<i64> intStream();

  sink<i64 throws (1: BiDiSinkException sinkEx)>, stream<
    i64 throws (1: BiDiStreamException sinkEx)
  > canThrow() throws (1: BiDiMethodException methodEx);

  // EB-mode variants: force the handler to run inline on the IO/EventBase
  // thread. Used to exercise the code path in ServerBiDiStreamFactory where
  // serverExecutor captured by the factory is the EventBase, not a CPU
  // thread pool. These mirror the non-EB methods above one-for-one.
  @cpp.ProcessInEbThreadUnsafe
  sink<string>, stream<string> echoEb();

  @cpp.ProcessInEbThreadUnsafe
  string, sink<string>, stream<string> echoWithResponseEb(1: string initial);

  @cpp.ProcessInEbThreadUnsafe
  sink<i64>, stream<i64> intStreamEb();

  @cpp.ProcessInEbThreadUnsafe
  sink<i64 throws (1: BiDiSinkException sinkEx)>, stream<
    i64 throws (1: BiDiStreamException sinkEx)
  > canThrowEb() throws (1: BiDiMethodException methodEx);
}
