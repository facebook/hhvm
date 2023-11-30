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

namespace cpp2 apache.thrift.stress

include "thrift/annotation/cpp.thrift"

enum ProcessingMode {
  Default = 0, // execute inline in handler (synchronously)
  Async = 1, // execute asynchronously in handler
}

enum WorkSimulationMode {
  Default = 0, // simulate work by busy waiting
  Sleep = 1, // simulate work by sleeping
}

struct ProcessInfo {
  1: i64 processingTimeMs;
  2: ProcessingMode processingMode;
  3: WorkSimulationMode workSimulationMode;
  4: i64 responseSize;
}

struct BasicRequest {
  1: ProcessInfo processInfo;
  2: binary payload;
}

struct BasicResponse {
  1: binary payload;
}

struct StreamProcessInfo {
  1: i64 initialResponseProcessingTimeMs;
  2: i64 initialResponseSize;
  3: i64 serverChunkProcessingTimeMs;
  4: i64 clientChunkProcessingTimeMs; // client simulates work via sleep
  5: WorkSimulationMode serverWorkSimulationMode;
  6: i64 numChunks = 1; // must be >= 1
  7: i64 chunkSize;
  8: i64 finalResponseProcessingTimeMs; // only used in sinks
  9: i64 finalResponseSize; // only used in sinks
}

struct StreamRequest {
  1: StreamProcessInfo processInfo;
  3: binary payload;
}

service StressTest {
  @cpp.ProcessInEbThreadUnsafe
  void ping();
  binary echo(1: binary payload);
  @cpp.ProcessInEbThreadUnsafe
  binary echoEb(1: binary payload);

  BasicResponse requestResponseTm(1: BasicRequest req);
  @cpp.ProcessInEbThreadUnsafe
  BasicResponse requestResponseEb(1: BasicRequest req);

  BasicResponse, stream<BasicResponse> streamTm(1: StreamRequest req);
  BasicResponse, sink<BasicResponse, BasicResponse> sinkTm(
    1: StreamRequest req,
  );
}
