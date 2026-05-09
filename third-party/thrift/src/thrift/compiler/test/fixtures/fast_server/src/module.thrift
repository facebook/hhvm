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

package "meta.com/thrift/test/fixtures/fast_server"

namespace cpp2 cpp2.test

struct DataItem {
  1: i32 id;
  2: string name;
  3: double value;
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
service BasicService {
  // void return, no args
  void ping();

  // primitive args + primitive return
  i32 add(1: i32 a, 2: i32 b);

  // complex args + complex return
  DataItem buildItem(1: DataItem template_, 2: i32 id);

  // single declared exception
  DataItem lookup(1: i32 id) throws (1: NotFoundException ex);

  // multiple declared exceptions — exercises cascade with two probes
  DataItem secureLookup(1: i32 id, 2: string user) throws (
    1: NotFoundException notFound,
    2: PermissionDeniedException denied,
  );

  // oneway — generator must skip this method entirely
  // @lint-ignore THRIFTCHECKS oneway is intentional fixture input
  oneway void fireAndForget(1: string event);
}
