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

#include <folly/portability/GMock.h>

#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/gen-cpp2/StreamService.h>

namespace testutil::testservice {

class TestStreamServiceMock
    : public apache::thrift::ServiceHandler<StreamService> {
 public:
  TestStreamServiceMock() {}

  int32_t echo(int32_t value) override;

  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;
  apache::thrift::ServerStream<std::string> buffers(int32_t count) override;
  apache::thrift::ServerStream<std::string> customBuffers(
      int32_t count, int32_t size) override;
  apache::thrift::ServerStream<int32_t> slowRange(
      int32_t from, int32_t to, int32_t millis) override;
  apache::thrift::ServerStream<int32_t> slowCancellation() override;
  apache::thrift::ServerStream<Message> returnNullptr() override;
  apache::thrift::ResponseAndServerStream<int, Message> throwError() override;

  apache::thrift::ResponseAndServerStream<int32_t, int32_t> leakCheck(
      int32_t from, int32_t to) override;
  apache::thrift::ResponseAndServerStream<int32_t, int32_t> leakCheckWithSleep(
      int32_t from, int32_t to, int32_t sleepMs) override;
  int32_t instanceCount() override;

  apache::thrift::ResponseAndServerStream<int32_t, int32_t> sleepWithResponse(
      int32_t timeMs) override;

  apache::thrift::ServerStream<int32_t> sleepWithoutResponse(
      int32_t timeMs) override;

  apache::thrift::ResponseAndServerStream<int32_t, int32_t> streamServerSlow()
      override;

  void sendMessage(int32_t messageId, bool complete, bool error) override;
  apache::thrift::ServerStream<int32_t> registerToMessages() override;

  apache::thrift::ServerStream<Message> streamThrows(int32_t whichEx) override;
  apache::thrift::ResponseAndServerStream<int32_t, Message>
  responseAndStreamThrows(int32_t whichEx) override;

  apache::thrift::ServerStream<int32_t> requestWithBlob(
      std::unique_ptr<folly::IOBuf> val) override;

  void async_eb_leakCallback(
      apache::thrift::HandlerCallbackPtr<apache::thrift::ServerStream<int32_t>>)
      override;

  void async_eb_orderRequestStream(
      apache::thrift::HandlerCallbackPtr<
          apache::thrift::ResponseAndServerStream<int32_t, int32_t>>) override;

  void async_eb_orderRequestResponse(
      apache::thrift::HandlerCallbackPtr<int32_t>) override;

  apache::thrift::ServerStream<int32_t> leakPublisherCheck() override;

 protected:
  folly::ScopedEventBaseThread executor_;

  std::unique_ptr<apache::thrift::ServerStreamPublisher<int32_t>> messages_;

  std::atomic<int32_t> order_{0};

  folly::Synchronized<
      std::unique_ptr<apache::thrift::ServerStreamPublisher<int32_t>>>
      publisher_;
};

} // namespace testutil::testservice
