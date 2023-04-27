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

struct TestStruct {
  1: string s;
  2: i32 i;
}

typedef binary (cpp2.type = "folly::IOBuf") IOBuf
typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

struct TestStructIOBuf {
  1: IOBuf buf;
  2: i32 i;
}

struct TestStructRecursive {
  6: string tag;
  99: optional TestStructRecursive cdr (cpp.ref = 'true');
}

typedef byte (cpp2.type = "uint8_t") UInt8
typedef i16 (cpp2.type = "uint16_t") UInt16
typedef i32 (cpp2.type = "uint32_t") UInt32
typedef i64 (cpp2.type = "uint64_t") UInt64

struct TestUnsignedIntStruct {
  1: UInt8 u8;
  2: UInt16 u16;
  3: UInt32 u32;
  4: UInt64 u64;
}

union TestUnsignedIntUnion {
  1: UInt8 u8;
  2: UInt16 u16;
  3: UInt32 u32;
  4: UInt64 u64;
}

struct TestUnsignedInt32ListStruct {
  1: list<UInt32> l;
}

struct TestUnsignedIntMapStruct {
  1: map<UInt32, UInt64> m;
}

service TestService {
  string sendResponse(1: i64 size);
  oneway void noResponse(1: i64 size);
  string echoRequest(1: string req);
  i32 echoInt(1: i32 req);
  string serializationTest(1: bool inEventBase);
  string eventBaseAsync() (thread = 'eb');
  void notCalledBack();
  void voidResponse() (cpp.coroutine);
  i32 processHeader();
  IOBufPtr echoIOBuf(1: IOBuf buf);
  oneway void noResponseIOBuf(1: IOBuf buf);
  stream<byte> echoIOBufAsByteStream(1: IOBuf buf, 2: i32 delayMs);
  void throwsHandlerException();
  stream<i32> range(1: i32 from, 2: i32 to);
  bool, stream<i32> rangeWithResponse(1: i32 from, 2: i32 to);
  void priorityHigh() (priority = "HIGH");
  void priorityBestEffort() (priority = "BEST_EFFORT", cpp.coroutine);
  sink<i32, i32> sumSink() (cpp.coroutine);
}
