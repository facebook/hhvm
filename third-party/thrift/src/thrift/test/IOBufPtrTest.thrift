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

namespace cpp2 thrift.test.iobufptr

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr
@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBufBinary

struct Request {
  1: IOBufPtr one;
  2: IOBufPtr two = "hello";
  3: IOBufBinary three;
}

union Union {
  1: IOBufPtr foo;
}

struct Containers {
  1: optional list<IOBufBinary> bufList;
  2: optional list<IOBufPtr> bufPtrList;
  3: optional list<Union> bufPtrUnionList;
  4: optional set<Union> bufPtrUnionSet;
  5: optional map<i32, Union> bufPtrUnionMap;
}

service IOBufPtrTestService {
  IOBufPtr combine(1: Request req);
}
