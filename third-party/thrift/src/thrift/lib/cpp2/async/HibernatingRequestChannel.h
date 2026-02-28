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

#include <chrono>

#include <folly/io/async/AsyncTimeout.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift {

// Simple RequestChannel wrapper. Will close the underlying channel when
// inactive for sufficiently long. Will reopen when it receives traffic.
class HibernatingRequestChannel : public RequestChannel {
 public:
  using Impl = RequestChannel;
  using ImplPtr = std::shared_ptr<Impl>;
  using ImplCreator = folly::Function<ImplPtr(folly::EventBase&)>;
  using UniquePtr = std::unique_ptr<
      HibernatingRequestChannel,
      folly::DelayedDestruction::Destructor>;

  static UniquePtr newChannel(
      folly::EventBase& evb,
      std::chrono::milliseconds waitTime,
      ImplCreator implCreator) {
    return {
        new HibernatingRequestChannel(evb, waitTime, std::move(implCreator)),
        {}};
  }

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions& options,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestNoResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf>) override {
    LOG(FATAL) << "Not supported";
  }

  void sendRequestStream(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      StreamClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf>) override {
    clientCallback->onFirstResponseError(
        folly::make_exception_wrapper<transport::TTransportException>(
            "This channel doesn't support stream RPC"));
  }

  void sendRequestSink(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      SinkClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf>) override {
    clientCallback->onFirstResponseError(
        folly::make_exception_wrapper<transport::TTransportException>(
            "This channel doesn't support sink RPC"));
  }

  void setCloseCallback(CloseCallback*) override {
    LOG(FATAL) << "Not supported";
  }

  folly::EventBase* getEventBase() const override { return &evb_; }

  uint16_t getProtocolId() override { return impl()->getProtocolId(); }

 protected:
  ~HibernatingRequestChannel() override { timeout_->cancelTimeout(); }

 private:
  HibernatingRequestChannel(
      folly::EventBase& evb,
      std::chrono::milliseconds waitTime,
      ImplCreator implCreator)
      : implCreator_(std::move(implCreator)),
        evb_(evb),
        waitTime_(waitTime),
        timeout_(
            folly::AsyncTimeout::make(
                evb_, [&impl = impl_]() mutable noexcept { impl.reset(); })) {}

  ImplPtr& impl();

  class RequestCallback;

  ImplPtr impl_;
  ImplCreator implCreator_;
  folly::EventBase& evb_;
  std::chrono::milliseconds waitTime_;
  std::unique_ptr<folly::AsyncTimeout> timeout_;
};
} // namespace apache::thrift
