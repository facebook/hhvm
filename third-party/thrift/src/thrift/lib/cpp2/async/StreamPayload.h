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

#include <memory>

#include <folly/io/IOBuf.h>
#include <folly/io/async/fdsock/SocketFds.h>

#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

struct FirstResponsePayload {
  FirstResponsePayload(
      std::unique_ptr<folly::IOBuf> p, ResponseRpcMetadata&& md);

  std::unique_ptr<folly::IOBuf> payload;
  ResponseRpcMetadata metadata;
  folly::SocketFds fds;
};

struct StreamPayload {
  StreamPayload(
      std::unique_ptr<folly::IOBuf> p,
      StreamPayloadMetadata&& md,
      bool isOrderedHdr = false);

  // IMPORTANT: The copy constructor / copy assignment are only intended for
  // `StreamPayload`s that are intended to be sent.  Never copy received
  // payloads.

  StreamPayload(const StreamPayload& oth);

  StreamPayload(StreamPayload&&) = default;

  StreamPayload& operator=(const StreamPayload& oth);

  StreamPayload& operator=(StreamPayload&& oth) = default;

  std::unique_ptr<folly::IOBuf> payload;
  StreamPayloadMetadata metadata;
  // OrderedHeader is sent as a PAYLOAD frame with an empty payload
  bool isOrderedHeader;
  folly::SocketFds fds; // Sent only via `RichPayloadToSend`
};

struct HeadersPayload {
  HeadersPayload(HeadersPayloadContent&& p, HeadersPayloadMetadata&& md);
  explicit HeadersPayload(HeadersPayloadContent&& p);
  explicit HeadersPayload(StreamPayloadMetadata&& sp);

  HeadersPayloadContent payload;
  HeadersPayloadMetadata metadata;

  explicit operator StreamPayload() &&;
};

} // namespace apache::thrift
