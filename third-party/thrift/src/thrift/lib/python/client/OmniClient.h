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

#include <folly/Expected.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/util/MethodMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::python::client {

using RequestChannelUnique = std::unique_ptr<
    apache::thrift::RequestChannel,
    folly::DelayedDestruction::Destructor>;

using RequestChannelShared = std::shared_ptr<apache::thrift::RequestChannel>;

using IOBufClientBufferedStream =
    apache::thrift::ClientBufferedStream<std::unique_ptr<folly::IOBuf>>;

using IOBufClientSink = apache::thrift::
    ClientSink<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>;

struct OmniClientResponseWithHeaders {
  folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper> buf;
  apache::thrift::transport::THeader::StringToStringMap headers;
  std::unique_ptr<IOBufClientBufferedStream> stream;
  std::unique_ptr<IOBufClientSink> sink;
};

/**
 * The Omniclient class can send requests to any Thrift service given
 * a RequestChannel.
 */
class OmniClient : public apache::thrift::TClientBase {
 public:
  explicit OmniClient(RequestChannelUnique channel);
  explicit OmniClient(RequestChannelShared channel);
  explicit OmniClient(OmniClient&&) noexcept;
  ~OmniClient() override;

  OmniClientResponseWithHeaders sync_send(
      const std::string& serviceName,
      const std::string& functionName,
      std::unique_ptr<folly::IOBuf> args,
      apache::thrift::MethodMetadata::Data&& metadata,
      const std::unordered_map<std::string, std::string>& headers,
      apache::thrift::RpcOptions&& rpcOptions);

  OmniClientResponseWithHeaders sync_send(
      const std::string& serviceName,
      const std::string& functionName,
      const std::string& args,
      apache::thrift::MethodMetadata::Data&& metadata,
      const std::unordered_map<std::string, std::string>& headers,
      apache::thrift::RpcOptions&& rpcOptions);

  void oneway_send(
      const std::string& serviceName,
      const std::string& functionName,
      std::unique_ptr<folly::IOBuf> args,
      apache::thrift::MethodMetadata::Data&& metadata,
      const std::unordered_map<std::string, std::string>& headers,
      apache::thrift::RpcOptions&& rpcOptions);

  void oneway_send(
      const std::string& serviceName,
      const std::string& functionName,
      const std::string& args,
      apache::thrift::MethodMetadata::Data&& metadata,
      const std::unordered_map<std::string, std::string>& headers,
      apache::thrift::RpcOptions&& rpcOptions);

  /**
   * The semifuture_send function takes in a function name and its arguments
   * encoded in channel protocol, and sends back the raw response with headers,
   * wrapped in a folly::SemiFuture.
   */
  folly::SemiFuture<OmniClientResponseWithHeaders> semifuture_send(
      const std::string& serviceName,
      const std::string& functionName,
      std::unique_ptr<folly::IOBuf> args,
      apache::thrift::MethodMetadata::Data&& metadata,
      const std::unordered_map<std::string, std::string>& headers,
      apache::thrift::RpcOptions&& rpcOptions,
      folly::Executor* executor,
      const apache::thrift::RpcKind rpcKind =
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  folly::SemiFuture<OmniClientResponseWithHeaders> semifuture_send(
      const std::string& serviceName,
      const std::string& functionName,
      const std::string& args,
      apache::thrift::MethodMetadata::Data&& metadata,
      const std::unordered_map<std::string, std::string>& headers,
      apache::thrift::RpcOptions&& rpcOptions,
      folly::Executor* executor,
      const apache::thrift::RpcKind rpcKind =
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  uint16_t getChannelProtocolId() const;

  RequestChannelShared getChannelShared() const noexcept;

  // TODO(ffrancet): get rid of in favour of calling setInteraction once we've
  // implemented client RPCOptions
  void set_interaction_factory(OmniClient* client);

 protected:
  // RpcOptions unused in base OmniClient since no interaction support
  virtual void setInteraction(apache::thrift::RpcOptions& rpcOptions);

  RequestChannelShared channel_ = nullptr;
  std::atomic<OmniClient*> factoryClient_ = nullptr;

 private:
  /**
   * Sends the request to the Thrift service through the RequestChannel.
   */
  void sendImpl(
      apache::thrift::RpcOptions&& rpcOptions,
      std::unique_ptr<folly::IOBuf> args,
      const std::string& serviceName,
      const std::string& functionName,
      std::unique_ptr<apache::thrift::RequestCallback> callback,
      const apache::thrift::RpcKind rpcKind,
      apache::thrift::MethodMetadata::Data&& metadata);
};

inline uint16_t OmniClient::getChannelProtocolId() const {
  return channel_->getProtocolId();
}

inline RequestChannelShared OmniClient::getChannelShared() const noexcept {
  return channel_;
}

inline void OmniClient::set_interaction_factory(OmniClient* client) {
  // What happens if its nullptr
  factoryClient_ = client;
}

class OmniInteractionClient : public OmniClient {
 public:
  OmniInteractionClient(
      std::shared_ptr<apache::thrift::RequestChannel> channel,
      const std::string& methodName);
  OmniInteractionClient(OmniInteractionClient&&) noexcept = default;
  OmniInteractionClient& operator=(OmniInteractionClient&&);
  ~OmniInteractionClient() override;

 protected:
  void setInteraction(apache::thrift::RpcOptions& rpcOptions) override;

 private:
  void terminate();

  std::string methodName_;
  apache::thrift::InteractionId interactionId_;

  friend class OmniClient;
};

folly::Future<std::unique_ptr<OmniInteractionClient>>
createOmniInteractionClient(
    std::shared_ptr<apache::thrift::RequestChannel> channel,
    const std::string& methodName);

} // namespace apache::thrift::python::client
