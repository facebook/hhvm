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

package "facebook.com/thrift/test/e2e/graceful_shutdown"

service TestGracefulShutdownService {
  // Request-response: blocks until signaled, so we can trigger shutdown
  // mid-request
  string echo(1: string message);

  // Server streaming: yields items slowly so we can trigger shutdown mid-stream
  stream<i32> range(1: i32 from, 2: i32 to);

  // Sink: accepts items slowly so we can trigger shutdown mid-sink
  sink<i32, string> consume(1: i32 count);

  // BiDi: echoes input to output so we can trigger shutdown mid-bidi
  sink<string>, stream<string> bidiEcho();
}
