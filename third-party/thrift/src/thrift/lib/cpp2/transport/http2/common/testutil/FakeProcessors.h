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

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace apache {
namespace thrift {

// A set of processors useful for testing.

/**
 * EchoProcessor returns what it receives after adding one extra
 * header and appending a string to the end of the payload (so we know
 * that the processor has been invoked).
 */
class EchoProcessor : public ThriftProcessor {
 public:
  // The arguments to the constructor are the additional header and
  // payload data that is sent back, as well as the event base in
  // which to schedule the callbacks.
  EchoProcessor(
      const std::string& key,
      const std::string& value,
      const std::string& trailer,
      folly::EventBase* evb)
      : ThriftProcessor(server_),
        key_(key),
        value_(value),
        trailer_(trailer),
        evb_(evb) {}

  ~EchoProcessor() override = default;

  void onThriftRequest(
      RequestRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload,
      std::shared_ptr<ThriftChannelIf> channel,
      std::unique_ptr<Cpp2ConnContext> connContext) noexcept override;

 private:
  // We override onThriftRequest so this is unused. This exists simply to
  // appease the constructor of ThriftProcessor.
  ThriftServer server_;

  // The new entry to add to the header.
  std::string key_;
  std::string value_;

  // The new entry to add to the payload.
  std::string trailer_;

  // The event base for callbacks.
  folly::EventBase* evb_;

  void onThriftRequestHelper(
      RequestRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload,
      std::shared_ptr<ThriftChannelIf> channel) noexcept;
};

} // namespace thrift
} // namespace apache
