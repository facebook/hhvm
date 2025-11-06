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

#pragma once

#define THRIFT_LIB_CPP2_TRANSPORT_ROCKET_FRAMING_FRAMEDATAFIRSTFIELDALIGNER_H_

#include <cstdint>
#include <memory>

#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>

namespace apache::thrift::rocket {

/**
 * NOTE: This feature is for a very specific use case, and should not be used
 * without consulting the Thrift team.
 *
 * This trieds to be align the first entry of the first param of a Thrift RPC
 * Call to the provided value, relative to the start of the frame. There are
 * several conditions that must be true for this to be serviced:
 * 1. Users must request for a valid alignment (power of 2)
 * 2. The field to be aligned must be the first entry of the first param of the
 * Thrift RPC Call.
 * 3. The field to be aligned must be a binary field.
 * 4. The field to be aligned must be using PaddedBinaryAdapter.
 * 5. The field to be aligned must be serialized using BinaryProtocol.
 * 6. The field to be aligned must be padded to the size of the alignment that
 * is requested.
 * 7. The IOBuf chain created during serialization must be retained, and not
 * coalesced. If any of these are not true, then the requested alignment would
 * not be provided.
 */
template <class Frame>
class FrameDataFirstFieldAligner {
 public:
  explicit FrameDataFirstFieldAligner(Frame& frame);

  void align();

 private:
  bool validateAlignmentRequest();
  void extractPayloadComponents();
  void reconstructPayload();
  bool findFieldToAlign();
  bool validatePadding();
  void trimPadding();
  uint32_t computePaddingNeeded();

  Frame& frame_;
  uint32_t fieldAlignment_{0};
  uint32_t payloadMetadataSize_{0};
  std::unique_ptr<folly::IOBuf> payloadBuf_;
  uint32_t offsetOfFieldToAlignInPayload_{0};
  uint32_t updatedDataSize_{0};
  uint32_t updatedPadding_{0};
  uint32_t paddingToTrim_{0};
  folly::io::Cursor readCursor_{nullptr};
  folly::IOBuf* paddingIOBuf_{nullptr};

  static constexpr uint32_t kDataSizeFieldLength = sizeof(uint32_t);
};

} // namespace apache::thrift::rocket
