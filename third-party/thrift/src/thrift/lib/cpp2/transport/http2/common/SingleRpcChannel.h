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

#include <string>

#include <folly/FixedString.h>
#include <folly/Function.h>

#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/transport/http2/common/H2Channel.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    void,
    handleFrameworkMetadataHTTPHeader,
    const transport::THeader::StringToStringMap&,
    RequestRpcMetadata*);
}

class Cpp2Worker;

class SingleRpcChannel : public H2Channel {
 public:
  SingleRpcChannel(
      proxygen::HTTPTransaction* toHttp2,
      ThriftProcessor* processor,
      std::shared_ptr<Cpp2Worker> worker);

  SingleRpcChannel(
      folly::EventBase& evb,
      folly::Function<proxygen::HTTPTransaction*(SingleRpcChannel*)>
          transactionFactory);

  ~SingleRpcChannel() override;

  void sendThriftResponse(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload) noexcept override;

  void sendThriftRequest(
      RequestMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload,
      std::unique_ptr<ThriftClientCallback> callback) noexcept override;

  folly::EventBase* getEventBase() noexcept override;

  void onH2StreamBegin(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onH2BodyFrame(std::unique_ptr<folly::IOBuf> contents) noexcept override;

  void onH2StreamEnd() noexcept override;

  void onH2StreamClosed(
      proxygen::ProxygenError error,
      std::string errorDescription) noexcept override;

  void onMessageFlushed() noexcept override;

  void setNotYetStable() noexcept;

 private:
  // The server side handling code for onH2StreamEnd().
  virtual void onThriftRequest() noexcept;

  // The client side handling code for onH2StreamEnd().
  void onThriftResponse() noexcept;

  void extractHeaderInfo(RequestRpcMetadata* metadata) noexcept;

  // Called from onThriftRequest() to send an error response.
  void sendThriftErrorResponse(
      const std::string& message,
      ProtocolId protoId = ProtocolId::COMPACT,
      const std::string& name = "process") noexcept;

  // The thrift processor used to execute RPCs (server side only).
  // Owned by H2ThriftServer.
  ThriftProcessor* processor_{nullptr};

  std::shared_ptr<Cpp2Worker> worker_;

  // Event base on which all methods in this object must be invoked.
  folly::EventBase* evb_{nullptr};

  folly::Function<proxygen::HTTPTransaction*(SingleRpcChannel*)>
      transactionFactory_;

  // Transaction object for use on client side to communicate with the
  // Proxygen layer.
  proxygen::HTTPTransaction* httpTransaction_;

  // Callback for client side.
  std::unique_ptr<ThriftClientCallback> callback_;

  std::unique_ptr<proxygen::HTTPMessage> headers_;
  std::unique_ptr<folly::IOBuf> contents_;

  bool receivedThriftRPC_{false};
  bool receivedH2Stream_{false};

  std::shared_ptr<void> activeRequestsGuard_;
};

} // namespace apache::thrift
