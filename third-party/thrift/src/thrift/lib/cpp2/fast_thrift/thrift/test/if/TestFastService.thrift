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

package "facebook.com/thrift/fast_thrift/thrift/test"

namespace cpp2 apache.thrift.fast_thrift.thrift.test

exception TestFastException {
  1: string message;
}

@cpp.FastServer
@cpp.FastClient
service TestFastService {
  string echo(1: string message);

  // A declared exception comes back as a PAYLOAD frame carrying typed
  // ResponseRpcMetadata, so this is the failing shape that still has somewhere
  // to put response headers.
  void throwDeclared(1: string message) throws (1: TestFastException ex);
  i64 add(1: i64 a, 2: i64 b);
  string sendResponse(1: i64 size);
  void ping();

  // Paired probes: each reports whether its handler ran on the connection's
  // EventBase. Only the annotated one must stay there when a CPU pool exists.
  bool ranOnEventBase();
  @cpp.ProcessInEbThreadUnsafe
  bool ebRanOnEventBase();
}
