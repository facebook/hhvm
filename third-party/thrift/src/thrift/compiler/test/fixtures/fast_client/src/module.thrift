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

package "meta.com/thrift/test/fixtures/fast_client"

namespace cpp2 cpp2.test

struct DataItem {
  1: i32 id;
  2: string name;
  3: double value;
}

enum StatusCode {
  OK = 0,
  ERROR = 1,
  PENDING = 2,
}

@cpp.FastClient
service FastClientTestService {
  void ping();
  string echo(1: string message);
  i32 add(1: i32 a, 2: i32 b);
  DataItem getData(1: i32 id);
  bool processData(1: DataItem item, 2: StatusCode status);
}
