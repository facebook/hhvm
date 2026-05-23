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

// Two test services used by ThriftServerCompositeE2ETest. Disjoint method
// names so the composite routes inbound requests to the correct child
// codegen adapter. Each service is annotated for both FastServer (server-
// side codegen) and FastClient (client-side codegen) so the E2E test can
// stand up a real server pipeline with composite-wrapped children and
// drive it from typed FastClients over real sockets.

include "thrift/annotation/cpp.thrift"

package "facebook.com/thrift/fast_thrift/thrift/test/composite_e2e"

namespace cpp2 apache.thrift.fast_thrift.thrift.test.composite_e2e

@cpp.FastServer
@cpp.FastClient
service CompositeE2EPrimaryService {
  void primaryPing();
  string primaryEcho(1: string message);
  i64 primaryAdd(1: i64 a, 2: i64 b);
}

@cpp.FastServer
@cpp.FastClient
service CompositeE2ESecondaryService {
  void secondaryPing();
  string secondaryGreet(1: string name);
  i64 secondaryMultiply(1: i64 a, 2: i64 b);
}
