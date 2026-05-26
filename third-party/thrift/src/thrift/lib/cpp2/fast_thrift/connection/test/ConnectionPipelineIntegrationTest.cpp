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

/**
 * AcceptancePipelineIntegrationTest
 *
 * End-to-end exercise of the acceptance pipeline head:
 *   ConnectionListener (head) → MockTailHandler
 *
 * Drives connectionAccepted() directly with a socketpair fd and asserts
 * the mock tail receives a ConnectionMessage carrying a plain
 * AsyncSocket. Bypasses the bind/listen path so the test doesn't need a
 * real listening port.
 */

#include <sys/socket.h>
#include <array>
#include <memory>

#include <gtest/gtest.h>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionListener.h>

namespace apache::thrift::fast_thrift::connection {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::Result;
using channel_pipeline::SimpleBufferAllocator;
using channel_pipeline::TypeErasedBox;
using channel_pipeline::test::MockTailHandler;

namespace {

std::pair<folly::NetworkSocket, folly::NetworkSocket> makeSocketPair() {
  std::array<int, 2> fds{};
  PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
  return {folly::NetworkSocket(fds[0]), folly::NetworkSocket(fds[1])};
}

} // namespace

class AcceptancePipelineIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();
  }
  void TearDown() override { evbThread_.reset(); }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  SimpleBufferAllocator allocator_;
};

// Listener → tail delivers the raw AsyncSocket transport synchronously.
TEST_F(AcceptancePipelineIntegrationTest, DeliversRawSocket) {
  MockTailHandler tail;
  ConnectionMessage captured;
  tail.setOnReadCallback([&captured](TypeErasedBox&& msg) {
    captured = msg.take<ConnectionMessage>();
    return Result::Success;
  });

  ConnectionListener::Ptr listener(new ConnectionListener(
      evb_,
      folly::SocketAddress("::1", 0),
      SocketOptions{},
      /*enableReusePortBpfSpread=*/false));
  auto pipeline = PipelineBuilder<
                      ConnectionListener,
                      MockTailHandler,
                      SimpleBufferAllocator>()
                      .setEventBase(evb_)
                      .setHead(listener.get())
                      .setTail(&tail)
                      .setAllocator(&allocator_)
                      .build();
  listener->setPipeline(pipeline.get());
  // Activate the pipeline directly without start(); we don't need to
  // bind a real listening socket for this test.
  evb_->runInEventBaseThreadAndWait([&] { pipeline->activate(); });

  auto sp = makeSocketPair();
  folly::SocketAddress clientAddr("127.0.0.1", 4001);
  evb_->runInEventBaseThreadAndWait([&] {
    listener->connectionAccepted(
        sp.second,
        clientAddr,
        folly::AsyncServerSocket::AcceptCallback::AcceptInfo{});
  });

  EXPECT_EQ(tail.readCount(), 1);
  EXPECT_EQ(captured.clientAddr, clientAddr);
  EXPECT_NE(
      dynamic_cast<folly::AsyncSocket*>(captured.transport.get()), nullptr);

  evb_->runInEventBaseThreadAndWait([&] {
    captured.transport.reset();
    listener->resetPipeline();
    pipeline.reset();
    listener.reset();
  });
  ::close(sp.first.toFd());
}

} // namespace apache::thrift::fast_thrift::connection
