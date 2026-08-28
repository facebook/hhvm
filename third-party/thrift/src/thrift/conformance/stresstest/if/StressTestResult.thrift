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

package "meta.com/thrift/conformance/stresstest/result"

enum ConnectionMode {
  Unknown = 0,
  Plaintext = 1,
  Tls = 2,
  StopTlsV2 = 3,
}

struct ResultMetadata {
  1: string runId;
  2: string workload;
}

struct RequestCounters {
  1: i64 total;
  2: i64 succeeded;
  3: i64 failed;
}

struct ZcrxCounters {
  1: i64 eventBaseCount;
  2: set<i64> ioThreadIds;
  3: i64 copyFallbackCount;
  4: i64 copyFallbackBytes;
  5: i64 noBufferCount;
}

struct ClientResult {
  1: ResultMetadata metadata;
  2: RequestCounters requests;
}

struct ServerResult {
  1: ResultMetadata metadata;
  2: map<ConnectionMode, i64> connections;
  3: ZcrxCounters zcrx;
}
