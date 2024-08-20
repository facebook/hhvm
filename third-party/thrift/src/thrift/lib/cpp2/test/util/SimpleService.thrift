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

namespace cpp apache.thrift.util

include "thrift/annotation/cpp.thrift"

service SimpleService {
  i64 add(1: i64 a, 2: i64 b);

  string echoSlow(1: string message, 2: i64 sleepMs);

  oneway void lob();

  stream<i64> emptyStreamSlow(1: i64 sleepMs);

  sink<i64, bool> slowReturnSink(1: i64 sleepMs);

  void largeRequest(1: binary_9092 request);
}

service OtherService {
}

// The following were automatically generated and may benefit from renaming.
@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary binary_9092
