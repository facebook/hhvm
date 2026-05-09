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

package "facebook.com/thrift/fast_thrift/thrift/test/integration"

namespace cpp2 apache.thrift.fast_thrift.thrift.test.integration

struct EchoResponse {
  1: string message;
}

exception NotFoundException {
  1: i32 id;
  2: string message;
}

exception PermissionDeniedException {
  1: string user;
  2: string reason;
}

@cpp.FastServer
service FastThriftServer {
  // void return — exercises FastHandlerCallback<void>
  void ping();

  // primitive return + primitive args — exercises bare return type
  i64 add(1: i64 a, 2: i64 b);

  // complex return + complex args — exercises unique_ptr return + arg
  EchoResponse echo(1: string message);

  // single declared exception — exercises inner cascade with one probe
  EchoResponse lookup(1: i32 id) throws (1: NotFoundException ex);

  // multiple declared exceptions — exercises multi-probe cascade
  EchoResponse secureLookup(1: i32 id, 2: string user) throws (
    1: NotFoundException notFound,
    2: PermissionDeniedException denied,
  );
}

// Same shape as FastThriftServer but without @cpp.FastServer — generates
// apache::thrift::ServiceHandler<FastThriftChannelServer> for the
// ThriftServerChannel side of the integration benchmark.
service FastThriftChannelServer {
  void ping();
  i64 add(1: i64 a, 2: i64 b);
  EchoResponse echo(1: string message);
  EchoResponse lookup(1: i32 id) throws (1: NotFoundException ex);
  EchoResponse secureLookup(1: i32 id, 2: string user) throws (
    1: NotFoundException notFound,
    2: PermissionDeniedException denied,
  );
}
