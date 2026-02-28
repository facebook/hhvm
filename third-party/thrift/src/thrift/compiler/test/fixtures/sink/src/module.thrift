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

namespace java.swift test.fixtures.sink

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct InitialResponse {
  1: string content;
}

struct FinalResponse {
  1: string content;
}

struct SinkPayload {
  1: string content;
}

struct CompatibleWithKeywordSink {
  // sink keyword only has effect in service scope
  1: string sink;
}

exception InitialException {
  1: string reason;
}

exception SinkException1 {
  1: string reason;
}

exception SinkException2 {
  1: i64 reason;
}

service SinkService {
  sink<SinkPayload, FinalResponse> method();
  InitialResponse, sink<SinkPayload, FinalResponse> methodAndReponse();
  sink<SinkPayload, FinalResponse> methodThrow() throws (
    1: InitialException ex,
  );
  sink<
    SinkPayload throws (1: SinkException1 ex),
    FinalResponse
  > methodSinkThrow();
  sink<
    SinkPayload,
    FinalResponse throws (1: SinkException2 ex)
  > methodFinalThrow();
  sink<
    SinkPayload throws (1: SinkException1 ex),
    FinalResponse throws (1: SinkException2 ex)
  > methodBothThrow();
  @cpp.ProcessInEbThreadUnsafe
  sink<SinkPayload, FinalResponse> methodFast();
}
