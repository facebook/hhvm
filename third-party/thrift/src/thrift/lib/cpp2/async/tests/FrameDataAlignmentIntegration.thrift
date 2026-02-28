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
include "thrift/lib/cpp2/transport/rocket/framing/test/FrameDataFirstFieldAlignment.thrift"

cpp_include "thrift/lib/cpp2/protocol/detail/PaddedBinaryAdapter.h"

package "apache.org/thrift/test"

@cpp.Adapter{
  name = "::apache::thrift::protocol::PaddedBinaryAdapter",
  adaptedType = "::apache::thrift::protocol::PaddedBinaryData",
}
typedef binary PaddedBinaryData

struct AlignedDataWrongEntryRequest {
  1: binary firstEntry;
  2: PaddedBinaryData data;
}

struct AlignedDataResponse {
  1: binary data;
  2: i32 padding;
}

service FrameDataAlignmentIntegrationTestService {
  AlignedDataResponse sendRecvAligned(
    1: FrameDataFirstFieldAlignment.AlignedDataRequest request,
  );
  AlignedDataResponse sendRecvWrongParam(
    1: binary firstParam,
    2: FrameDataFirstFieldAlignment.AlignedDataRequest request,
  );
  AlignedDataResponse sendRecvWrongEntry(
    1: AlignedDataWrongEntryRequest request,
  );
  binary sendRecvBinary(
    1: FrameDataFirstFieldAlignment.BinaryDataRequest request,
  );
}
