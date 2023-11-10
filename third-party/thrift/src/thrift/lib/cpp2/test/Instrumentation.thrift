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

namespace cpp2 apache.thrift.test

include "thrift/annotation/cpp.thrift"

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf

service DebugTestService {
  string echo(1: string s);
}

service InstrumentationTestService {
  void sendRequest();
  stream<i32> sendStreamingRequest();
  IOBuf sendPayload(1: i32 id, 2: string str);
  void wait(1: i32 value, 2: bool busyWait, 3: bool setShallowRC);
  void runCallback();
  void runCallback2();
  void noop();
}
