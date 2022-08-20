/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr
typedef binary (cpp2.type = "folly::IOBuf") IOBufBinary

struct Request {
  1: IOBufPtr one;
  2: IOBufPtr two = "hello";
  3: IOBufBinary three;
}

service IOBufPtrTestService {
  IOBufPtr combine(1: Request req);
}
