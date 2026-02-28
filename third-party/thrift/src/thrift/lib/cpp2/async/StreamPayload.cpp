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

#include <thrift/lib/cpp2/async/StreamPayload.h>

#include <utility>

namespace apache::thrift {

FirstResponsePayload::FirstResponsePayload(
    std::unique_ptr<folly::IOBuf> p, ResponseRpcMetadata&& md)
    : payload(std::move(p)), metadata(std::move(md)) {}

StreamPayload::StreamPayload(
    std::unique_ptr<folly::IOBuf> p,
    StreamPayloadMetadata&& md,
    bool isOrderedHdr)
    : payload(std::move(p)),
      metadata(std::move(md)),
      isOrderedHeader(isOrderedHdr) {}

StreamPayload::StreamPayload(const StreamPayload& oth)
    : metadata(oth.metadata), isOrderedHeader(oth.isOrderedHeader) {
  if (oth.payload) {
    payload = oth.payload->clone();
  }
  fds.cloneToSendFromOrDfatal(oth.fds);
}

StreamPayload& StreamPayload::operator=(const StreamPayload& oth) {
  if (oth.payload) {
    payload = oth.payload->clone();
  }
  metadata = oth.metadata;
  isOrderedHeader = oth.isOrderedHeader;
  fds.cloneToSendFromOrDfatal(oth.fds);
  return *this;
}

HeadersPayload::HeadersPayload(
    HeadersPayloadContent&& p, HeadersPayloadMetadata&& md)
    : payload(std::move(p)), metadata(std::move(md)) {}

HeadersPayload::HeadersPayload(HeadersPayloadContent&& p)
    : payload(std::move(p)) {}

HeadersPayload::HeadersPayload(StreamPayloadMetadata&& sp) {
  payload.otherMetadata().copy_from(sp.otherMetadata());
  metadata.compression().copy_from(sp.compression());
}

HeadersPayload::operator StreamPayload() && {
  StreamPayloadMetadata md;
  md.otherMetadata().copy_from(payload.otherMetadata());
  md.compression().copy_from(metadata.compression());
  return StreamPayload(nullptr, std::move(md));
}

} // namespace apache::thrift
