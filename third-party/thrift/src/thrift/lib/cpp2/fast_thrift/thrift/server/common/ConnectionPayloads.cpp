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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>

namespace apache::thrift::fast_thrift::thrift {

// Out-of-line so the serialization machinery stays off ConnectionPayloads.h,
// which every translation unit that touches the inbound payload variant pulls
// in.
ThriftSetupResponsePayload::RocketFrame
ThriftSetupResponsePayload::toRocketFrame(
    rocket::server::MetadataProtocol) && noexcept {
  DCHECK(response != nullptr);
  auto metadataPush = serializeSetupResponse(std::move(*response));
  if (FOLLY_UNLIKELY(metadataPush == nullptr)) {
    // A SetupResponse is a handful of scalars with no client-supplied content,
    // so this is an allocation failure rather than anything a peer can provoke.
    XLOG(FATAL) << "failed to serialize the SETUP response";
  }
  return {
      .frameType = apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH,
      .streamId = 0,
      .metadata = std::move(metadataPush),
      .data = nullptr,
  };
}

} // namespace apache::thrift::fast_thrift::thrift
