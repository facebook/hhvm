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

namespace py3 thrift.python

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
exception MyException {
  1: string reason;
}

exception SinkException {
  1: string reason;
}

exception FinalException {
  1: string reason;
}

service TestSinkService {
  i32 test();
  // Generate numbers between `fr` to `to`. Using `fr` because `from` is reserved in Python
  // Returns sum of received numbers
  sink<i32, i32> range_(1: i32 fr, 2: i32 to);
  sink<i32, i32> rangeThrow(1: i32 fr, 2: i32 to);
  sink<i32, bool> rangeFinalResponseThrow(1: i32 fr, 2: i32 to);
  sink<i32, i32> rangeEarlyResponse(1: i32 fr, 2: i32 to, 3: i32 early);
  sink<i32, bool> unimplemented();
  bool, sink<i32, bool> initialThrow() throws (1: MyException e);
  bool, sink<i32 throws (1: SinkException e), bool> sinkThrow();
  sink<i32, bool throws (1: FinalException e)> sinkFinalThrow();
  void purge();

  sink<i32, bool> rangeCancelAt(1: i32 fr, 2: i32 to, 3: i32 cancelAt);
  sink<i32, bool> rangeSlowFinalResponse(1: i32 fr, 2: i32 to);
}
