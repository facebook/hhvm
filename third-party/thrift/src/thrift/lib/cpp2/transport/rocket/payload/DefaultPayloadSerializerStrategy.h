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

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializerStrategy.h>

namespace apache::thrift::rocket {

// TODO rroeser - right now this is a no-op, but will be used for adding support
// for checksum and compression, and other features that don't delegate to the
// PayloadUtils.h header free functions.
class DefaultPayloadSerializerStrategy final
    : public PayloadSerializerStrategy<DefaultPayloadSerializerStrategy> {
 public:
  DefaultPayloadSerializerStrategy() : PayloadSerializerStrategy(*this) {}

  template <class T>
  folly::Try<T> unpackAsCompressed(rocket::Payload&& payload, bool useBinary);

  template <class T>
  folly::Try<T> unpack(rocket::Payload&& payload, bool useBinary);

  template <typename Metadata>
  rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      folly::AsyncTransport* transport);

  template <typename T>
  size_t unpackCompact(T&, const folly::IOBuf*) {
    throw std::runtime_error("not implemented");
  }

  template <class PayloadType>
  Payload pack(PayloadType&& payload, folly::AsyncTransport* transport);
};

} // namespace apache::thrift::rocket
