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

#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>

namespace apache {
namespace thrift {

class ThriftParametersContext;

/**
 * FizzPeeker is a specialization of the default wangle Fizz peeker that returns
 * a handshake helper that includes Thrift specific behavior, such as sending
 * custom Thrift extension and implementing StopTLS.
 *
 * TODO: This can be generalized into wangle.
 */
class FizzPeeker : public wangle::DefaultToFizzPeekingCallback {
 public:
  using DefaultToFizzPeekingCallback::DefaultToFizzPeekingCallback;

  wangle::AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& /* bytes */,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo) override;

  void setThriftParametersContext(
      std::shared_ptr<apache::thrift::ThriftParametersContext> context) {
    thriftParametersContext_ = std::move(context);
  }

  std::shared_ptr<apache::thrift::ThriftParametersContext>
      thriftParametersContext_;
};
} // namespace thrift
} // namespace apache
