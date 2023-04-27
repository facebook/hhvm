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

cpp_include "folly/io/IOBuf.h"

typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>", py3.iobuf) IOBufPtr
typedef binary (cpp2.type = "folly::IOBuf", py3.iobuf) IOBuf

struct Moo {
  1: i32 val;
  2: IOBufPtr ptr;
  3: IOBuf buf;
  // this behaves like a mix of a reference and a regular field, so worth
  // testing it specially
  4: optional IOBufPtr opt_ptr;
}

service IobufTestService {
  Moo getMoo();
  void sendMoo(1: Moo m);
}
