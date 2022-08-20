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

namespace cpp2 facebook.thrift.benchmarks

struct TwoInts {
  1: optional i32 x;
  2: optional i32 y;
}

typedef binary (cpp2.type = "folly::IOBuf") IOBuf

struct Chunk {
  1: binary header;
  2: binary data;
}

struct Chunk2 {
  1: binary header;
  2: IOBuf data;
}
