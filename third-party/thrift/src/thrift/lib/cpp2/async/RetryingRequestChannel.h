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

#include <future>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {

// Simple RequestChannel wrapper, which automatically retries requests if they
// fail with a TTransportException.
class RetryingRequestChannel : public apache::thrift::RequestChannel {
 public:
  using Impl = apache::thrift::RequestChannel;
  using ImplPtr = std::shared_ptr<Impl>;
  using UniquePtr = std::
      unique_ptr<RetryingRequestChannel, folly::DelayedDestruction::Destructor>;

  static UniquePtr newChannel(
      folly::EventBase& evb, int numRetries, ImplPtr impl) {
    return {new RetryingRequestChannel(evb, numRetries, std::move(impl)), {}};
  }

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestStream(
      const apache::thrift::RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      apache::thrift::SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::StreamClientCallback* clientCallback) override;

  void sendRequestResponse(
      const apache::thrift::RpcOptions& options,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      RequestClientCallback::Ptr cob) override;

  void sendRequestSink(
      const apache::thrift::RpcOptions& rpcOptions,
      apache::thrift::MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::SinkClientCallback* cb) override;

  void sendRequestNoResponse(
      const apache::thrift::RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr) override {
    LOG(FATAL) << "Not supported";
  }

  void setCloseCallback(apache::thrift::CloseCallback*) override {
    LOG(FATAL) << "Not supported";
  }

  folly::EventBase* getEventBase() const override { return &evb_; }

  uint16_t getProtocolId() override { return impl_->getProtocolId(); }

 protected:
  ~RetryingRequestChannel() override = default;

  RetryingRequestChannel(folly::EventBase& evb, int numRetries, ImplPtr impl)
      : impl_(std::move(impl)), numRetries_(numRetries), evb_(evb) {}

  class RequestCallbackBase;
  class RequestCallback;
  class StreamCallback;
  class SinkCallback;

  ImplPtr impl_;
  int numRetries_;
  folly::EventBase& evb_;
};
} // namespace thrift
} // namespace apache
