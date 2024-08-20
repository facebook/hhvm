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
include "thrift/annotation/thrift.thrift"

struct TestStruct {
  1: string s;
  2: i32 i;
}

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

struct TestStructIOBuf {
  1: IOBuf buf;
  2: i32 i;
}

struct TestStructRecursive {
  6: string tag;
  @cpp.Ref{type = cpp.RefType.Unique}
  99: optional TestStructRecursive cdr;
}

@cpp.Type{name = "uint8_t"}
typedef byte UInt8
@cpp.Type{name = "uint16_t"}
typedef i16 UInt16
@cpp.Type{name = "uint32_t"}
typedef i32 UInt32
@cpp.Type{name = "uint64_t"}
typedef i64 UInt64

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
  @cpp.ProcessInEbThreadUnsafe
  string eventBaseAsync();
  void notCalledBack();
  void voidResponse();
  i32 processHeader();
  IOBufPtr echoIOBuf(1: IOBuf buf);
  oneway void noResponseIOBuf(1: IOBuf buf);
  stream<byte> echoIOBufAsByteStream(1: IOBuf buf, 2: i32 delayMs);
  void throwsHandlerException();
  stream<i32> range(1: i32 from, 2: i32 to);
  bool, stream<i32> rangeWithResponse(1: i32 from, 2: i32 to);
  @thrift.Priority{level = thrift.RpcPriority.HIGH}
  void priorityHigh();
  @thrift.Priority{level = thrift.RpcPriority.BEST_EFFORT}
  void priorityBestEffort();
  sink<i32, i32> sumSink();
}
