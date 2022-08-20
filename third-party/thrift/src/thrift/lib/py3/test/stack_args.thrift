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

struct simple {
  1: i32 val;
}

service StackService {
  list<i32> add_to(1: list<i32> lst, 2: i32 value);
  simple get_simple();
  simple get_simple_no_sa() (cpp.stack_arguments = "0");
  void take_simple(1: simple smpl);
  IOBuf get_iobuf();
  void take_iobuf(1: IOBuf val);
  // currently unsupported by the cpp backend:
  // IOBufPtr get_iobuf_ptr()
  void take_iobuf_ptr(1: IOBufPtr val);
}
