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

namespace cpp apache.thrift.test

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr
@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf

exception Xception {
  1: i32 errorCode;
  2: string message;
}

service FutureService {
  string sendResponse(1: i64 size);
  oneway void noResponse(1: i64 size);
  @cpp.GenerateDeprecatedHeaderClientMethods
  string echoRequest(1: string req);
  string echoRequestSlow(1: string req, 2: i64 sleepMs);
  i32 throwing() throws (1: Xception err1);
  void voidThrowing() throws (1: Xception err1);

  void buftest(1: IOBufPtr data);
}
