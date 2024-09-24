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

#include <folly/Try.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>

namespace apache::thrift::rocket {

/**
 * Defines strategy contract for serializing and deserializing payloads.
 */
template <typename Child>
class PayloadSerializerStrategy {
 public:
  template <class T>
  FOLLY_ERASE folly::Try<T> unpackAsCompressed(Payload&& payload) {
    return child_.unpackAsCompressed(std::move(payload));
  }

  template <class T>
  FOLLY_ERASE folly::Try<T> unpack(Payload&& payload) {
    return child_.unpack(std::move(payload));
  }

  template <typename T>
  FOLLY_ERASE std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
    return child_.packCompact(std::forward<T>(data));
  }

  template <typename Metadata>
  FOLLY_ERASE rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      folly::AsyncTransport* transport) {
    return child_.packWithFds(
        metadata, std::move(payload), std::move(fds), transport);
  }

  template <class PayloadType>
  FOLLY_ERASE Payload
  pack(PayloadType&& payload, folly::AsyncTransport* transport) {
    return child_.pack(std::forward<PayloadType>(payload), transport);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    return child_.unpackCompact(output, buffer);
  }

  virtual ~PayloadSerializerStrategy() = default;

 protected:
  explicit PayloadSerializerStrategy(Child& child) : child_(child) {}

 private:
  Child& child_;
};

} // namespace apache::thrift::rocket
