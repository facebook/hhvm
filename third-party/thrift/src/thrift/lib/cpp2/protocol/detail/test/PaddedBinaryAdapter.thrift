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

include "thrift/annotation/cpp.thrift"

cpp_include "thrift/lib/cpp2/protocol/detail/PaddedBinaryAdapter.h"

package "apache.org/thrift/test"

@cpp.Adapter{
  name = "::apache::thrift::protocol::PaddedBinaryAdapter",
  adaptedType = "::apache::thrift::protocol::PaddedBinaryData",
}
typedef binary PaddedBinaryData

@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

struct SingleAlignedBinary {
  1: PaddedBinaryData data;
}

struct MultipleAlignedBinary {
  1: list<PaddedBinaryData> data;
}

struct OldRequest {
  1: IOBufPtr data;
  2: string checksum;
}

struct NewRequest {
  1: PaddedBinaryData data;
  2: string checksum;
}
