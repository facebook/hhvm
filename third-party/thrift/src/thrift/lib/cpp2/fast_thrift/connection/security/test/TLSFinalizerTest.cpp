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

// Unit test for TLSFinalizer, the head endpoint of the TLS pipeline. It runs
// in a minimal pipeline (TLSFinalizer → capturing tail, no stages) and
// verifies that a TLSRequestMessage driven in on the write path is collapsed
// into a TLSResponseMessage and turned around onto the read path.

#include <sys/socket.h>
#include <array>
#include <chrono>
#include <memory>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/TLSFinalizer.h>

namespace apache::thrift::fast_thrift::connection::security::handler::test {

namespace {

// Tail that hands each TLSResponseMessage the finalizer turns around to a
// per-test callback.
class CapturingTail {
 public:
  using OnRead = folly::Function<void(TLSResponseMessage&&) noexcept>;

  explicit CapturingTail(OnRead onRead) noexcept : onRead_(std::move(onRead)) {}

  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto m = msg.take<TLSResponseMessage>();
    if (onRead_) {
      onRead_(std::move(m));
    }
    return channel_pipeline::Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void onWriteReady() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}

 private:
  OnRead onRead_;
};

// A connected fd to give an AsyncSocket a real identity. We only need one end
// for the transport; the peer is closed immediately (no I/O happens — the
// finalizer just moves the transport through).
folly::NetworkSocket makeSocketFd() {
  std::array<int, 2> fds{};
  PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
  ::close(fds[1]);
  return folly::NetworkSocket(fds[0]);
}

} // namespace

class TLSFinalizerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();
  }

  void TearDown() override {
    if (pipeline_) {
      evb_->runInEventBaseThreadAndWait([&] {
        pipeline_->deactivate();
        pipeline_.reset();
      });
    }
    evbThread_.reset();
  }

  // Minimal pipeline: TLSFinalizer (head) → CapturingTail (tail), no stages.
  void buildPipeline(CapturingTail::OnRead onRead) {
    tail_ = std::make_unique<CapturingTail>(std::move(onRead));
    evb_->runInEventBaseThreadAndWait([&] {
      channel_pipeline::PipelineBuilder<
          TLSFinalizer,
          CapturingTail,
          channel_pipeline::SimpleBufferAllocator>
          builder;
      builder.setEventBase(evb_)
          .setHead(&finalizer_)
          .setTail(tail_.get())
          .setAllocator(&allocator_);
      pipeline_ = builder.build();
      finalizer_.setPipeline(pipeline_.get());
      pipeline_->activate();
    });
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  channel_pipeline::SimpleBufferAllocator allocator_;
  TLSFinalizer finalizer_;
  std::unique_ptr<CapturingTail> tail_;
  channel_pipeline::PipelineImpl::Ptr pipeline_;
};

// A request driven through the finalizer emerges as a response carrying the
// same transport and peer address. Negotiation state (tlsParams/extension) is
// dropped by construction — TLSResponseMessage has no such fields.
TEST_F(TLSFinalizerTest, ConvertsRequestToResponse) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr captured;
  folly::SocketAddress capturedAddr;

  buildPipeline([&](TLSResponseMessage&& resp) noexcept {
    captured = std::move(resp.transport);
    capturedAddr = resp.clientAddr;
    emitted.post();
  });

  folly::AsyncTransport* rawTransport = nullptr;
  const folly::SocketAddress addr{"127.0.0.1", 4321};
  evb_->runInEventBaseThreadAndWait([&] {
    auto sock = folly::AsyncSocket::newSocket(evb_, makeSocketFd());
    rawTransport = sock.get();
    TLSRequestMessage req{
        .transport = folly::AsyncTransport::UniquePtr(sock.release()),
        .clientAddr = addr,
        .tlsParams = nullptr,
        .extension = nullptr,
    };
    (void)pipeline_->fireWrite(channel_pipeline::erase_and_box(std::move(req)));
  });

  ASSERT_TRUE(emitted.try_wait_for(std::chrono::seconds{5}));
  EXPECT_EQ(captured.get(), rawTransport); // same transport, moved through
  EXPECT_EQ(capturedAddr, addr);

  evb_->runInEventBaseThreadAndWait([&] { captured.reset(); });
}

} // namespace apache::thrift::fast_thrift::connection::security::handler::test

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  folly::Init init(&argc, &argv);
  return RUN_ALL_TESTS();
}
