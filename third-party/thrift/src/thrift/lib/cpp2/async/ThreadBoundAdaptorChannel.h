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

#include <folly/executors/IOExecutor.h>
#include <folly/io/async/EventBaseLocal.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift {

class ThreadBoundAdaptorChannel : public apache::thrift::RequestChannel {
 public:
  ThreadBoundAdaptorChannel(
      folly::EventBase* evb,
      std::shared_ptr<apache::thrift::RequestChannel> threadSafeChannel);

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  // RequestChannel
  void sendRequestResponse(
      const apache::thrift::RpcOptions& options,
      MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::RequestClientCallback::Ptr cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestNoResponse(
      const apache::thrift::RpcOptions& options,
      MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::RequestClientCallback::Ptr cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestStream(
      const apache::thrift::RpcOptions& options,
      MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::StreamClientCallback* cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestSink(
      const apache::thrift::RpcOptions& options,
      MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::SinkClientCallback* cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void setCloseCallback(apache::thrift::CloseCallback*) override;

  folly::EventBase* getEventBase() const override;

  uint16_t getProtocolId() override;

  void terminateInteraction(apache::thrift::InteractionId id) override;

  apache::thrift::InteractionId createInteraction(
      ManagedStringView&& name) override;

  apache::thrift::RequestChannel* getChannel() const {
    return threadSafeChannel_.get();
  }

 private:
  std::shared_ptr<apache::thrift::RequestChannel> threadSafeChannel_;
  folly::EventBase* evb_;
};
} // namespace apache::thrift
