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

#undef protected
#undef private

#include <gtest/gtest.h>

#include <memory>

#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GMock.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#define private public
#define protected public
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::transport {

using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using MockAppHandler = MockTailHandler; // App receives reads (Tail)
using namespace testing;

namespace {
// Build a simple byte buffer for testing
std::unique_ptr<folly::IOBuf> buildTestData(size_t size) {
  auto buf = folly::IOBuf::create(size);
  std::memset(buf->writableData(), 'x', size);
  buf->append(size);
  return buf;
}
} // namespace

HANDLER_TAG(exception_handler);

class TransportHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { appHandler_.reset(); }

  /**
   * Creates a TransportHandler and Pipeline.
   */
  std::pair<TransportHandler::Ptr, PipelineImpl::Ptr>
  createHandlerAndPipeline() {
    auto socket = folly::AsyncTransport::UniquePtr(
        new NiceMock<folly::test::MockAsyncTransport>());
    mockSocket_ = static_cast<folly::test::MockAsyncTransport*>(socket.get());
    // onConnect() DCHECKs the socket is good. Tests that want to drive
    // not-good behavior override this in-line.
    ON_CALL(*mockSocket_, good()).WillByDefault(Return(true));
    // DrainTimeout needs a valid EventBase from the socket.
    ON_CALL(*mockSocket_, getEventBase()).WillByDefault(Return(&evb_));

    auto handler = TransportHandler::create(std::move(socket), 256, 4096);

    auto pipeline = PipelineBuilder<
                        TransportHandler,
                        MockAppHandler,
                        SimpleBufferAllocator>()
                        .setEventBase(&evb_)
                        .setHead(handler.get())
                        .setTail(&appHandler_)
                        .setAllocator(&allocator_)
                        .build();

    handler->setPipeline(pipeline.get());

    return {std::move(handler), std::move(pipeline)};
  }

  /**
   * Creates a TransportHandler and Pipeline with a MockHandler that can capture
   * exceptions.
   *
   * Pipeline must die before TransportHandler so its destruction defers via
   * TH's pipelineGuard_ and runs handlerRemoved while TH is still alive
   * (inside TH's ~T body via resetPipeline -> pipelineGuard_.reset()). Reverse
   * order causes UAF.
   *
   * Class member destruction order is reverse declaration order in both
   * libstdc++ and libc++, so declare `pipeline` last to make it destroyed
   * first. std::tuple does NOT have a portable destruction order — libstdc++
   * destroys front-to-back, libc++ destroys back-to-front — so we cannot
   * return one here.
   */
  struct HandlerAndPipelineWithExceptionHandler {
    TransportHandler::Ptr handler;
    MockHandler* mockHandler;
    PipelineImpl::Ptr pipeline;
  };

  HandlerAndPipelineWithExceptionHandler
  createHandlerAndPipelineWithExceptionHandler() {
    auto socket = folly::AsyncTransport::UniquePtr(
        new NiceMock<folly::test::MockAsyncTransport>());
    mockSocket_ = static_cast<folly::test::MockAsyncTransport*>(socket.get());
    // onConnect() DCHECKs the socket is good. Tests that want to drive
    // not-good behavior override this in-line.
    ON_CALL(*mockSocket_, good()).WillByDefault(Return(true));
    // DrainTimeout needs a valid EventBase from the socket.
    ON_CALL(*mockSocket_, getEventBase()).WillByDefault(Return(&evb_));

    auto handler = TransportHandler::create(std::move(socket), 256, 4096);

    auto mockHandler = std::make_unique<MockHandler>();
    auto* mockHandlerPtr = mockHandler.get();

    auto pipeline = PipelineBuilder<
                        TransportHandler,
                        MockAppHandler,
                        SimpleBufferAllocator>()
                        .setEventBase(&evb_)
                        .setHead(handler.get())
                        .setTail(&appHandler_)
                        .setAllocator(&allocator_)
                        .addNextDuplex<MockHandler>(
                            exception_handler_tag, std::move(mockHandler))
                        .build();

    handler->setPipeline(pipeline.get());

    return {std::move(handler), mockHandlerPtr, std::move(pipeline)};
  }

  folly::EventBase evb_;
  folly::test::MockAsyncTransport* mockSocket_{nullptr};
  MockAppHandler appHandler_;
  SimpleBufferAllocator allocator_;
};

// Test: Write Path
TEST_F(TransportHandlerTest, WritePath) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  std::shared_ptr<folly::IOBuf> capturedBuf;
  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(DoAll(
          SaveArg<0>(&capturedCallback),
          SaveArg<1>(&capturedBuf),
          InvokeWithoutArgs([&]() {
            if (capturedCallback) {
              capturedCallback->writeSuccess();
            }
          })));

  auto bytes = folly::IOBuf::copyBuffer("test data");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler->writePending_, 0);
}

// Test: Pause and Resume Read
TEST_F(TransportHandlerTest, PauseAndResumeRead) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  // Initially readPaused_ is true (not reading yet)
  EXPECT_TRUE(handler->readPaused_);

  // resumeRead should set the handler itself as read callback (implements
  // ReadCallback)
  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  handler->resumeRead();
  EXPECT_FALSE(handler->readPaused_);

  // Now pause
  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
  handler->pauseRead();
  EXPECT_TRUE(handler->readPaused_);
}

// Test: Pause Read is idempotent
TEST_F(TransportHandlerTest, PauseReadIdempotent) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  // Initially paused, so pauseRead should not call setReadCB
  EXPECT_TRUE(handler->readPaused_);
  handler->pauseRead();
  EXPECT_TRUE(handler->readPaused_);

  // Resume first
  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  handler->resumeRead();
  EXPECT_FALSE(handler->readPaused_);

  // Now pause should call setReadCB(nullptr)
  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
  handler->pauseRead();
  EXPECT_TRUE(handler->readPaused_);

  // Second pause should not call setReadCB again
  handler->pauseRead();
  EXPECT_TRUE(handler->readPaused_);
}

// Test: Resume Read is idempotent
TEST_F(TransportHandlerTest, ResumeReadIdempotent) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  // Initially paused
  EXPECT_TRUE(handler->readPaused_);

  // First resume should set callback
  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  handler->resumeRead();
  EXPECT_FALSE(handler->readPaused_);

  // Second resume should not call setReadCB again (already resumed)
  handler->resumeRead();
  EXPECT_FALSE(handler->readPaused_);
}

TEST_F(TransportHandlerTest, OnReadReadyResumesRead) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  EXPECT_TRUE(handler->readPaused_);

  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  handler->onReadReady();

  EXPECT_FALSE(handler->readPaused_);
}

TEST_F(
    TransportHandlerTest,
    ReadBackpressureThenPipelineReadReadyResumesSocketRead) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  appHandler_.setOnReadCallback(
      [&](TypeErasedBox&&) { return Result::Backpressure; });

  {
    InSequence seq;
    EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
    EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
    EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  }

  handler->resumeRead();
  EXPECT_FALSE(handler->readPaused_);

  auto data = buildTestData(32);
  handler->readBufferAvailable(std::move(data));
  EXPECT_TRUE(handler->readPaused_);

  pipeline->onReadReady();
  EXPECT_FALSE(handler->readPaused_);
}

// Test: onConnect resumes reading and fires connect event to pipeline
TEST_F(TransportHandlerTest, OnConnectResumesReadAndFiresConnect) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();

  // Initially paused
  EXPECT_TRUE(handler->readPaused_);
  EXPECT_EQ(mockHandler->pipelineActivatedCount(), 0);

  // onConnect should resume reading and fire connect to the pipeline.
  // resetPipeline / dtor also call setReadCB(nullptr) on teardown.
  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(AnyNumber());
  handler->onConnect();

  EXPECT_FALSE(handler->readPaused_);
  EXPECT_EQ(mockHandler->pipelineActivatedCount(), 1);
}

// Test: Write Backpressure When Write Pending
TEST_F(TransportHandlerTest, WriteBackpressureWhenWritePending) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  // With the new behavior, both writes go through to the socket
  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .Times(2)
      .WillRepeatedly(SaveArg<0>(&capturedCallback));

  auto bytes1 = folly::IOBuf::copyBuffer("first write");
  Result result1 = handler->onWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(bytes1)));

  // First write returns Backpressure to signal pending write
  EXPECT_EQ(result1, Result::Backpressure);
  EXPECT_EQ(handler->writePending_, 1);

  auto bytes2 = folly::IOBuf::copyBuffer("second write");
  Result result2 = handler->onWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(bytes2)));

  // Second write also goes through and returns Backpressure
  EXPECT_EQ(result2, Result::Backpressure);
  EXPECT_EQ(handler->writePending_, 2);
  // Drain both pending writes so close doesn't hang in Closing.
  capturedCallback->writeSuccess();
  capturedCallback->writeSuccess();
}

// Test: Write Success Clears Pending State
TEST_F(TransportHandlerTest, WriteSuccessClearsPendingState) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .Times(2)
      .WillRepeatedly(SaveArg<0>(&capturedCallback));

  auto bytes1 = folly::IOBuf::copyBuffer("first write");
  Result result1 = handler->onWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(bytes1)));

  EXPECT_EQ(result1, Result::Backpressure);
  EXPECT_GT(handler->writePending_, 0);

  capturedCallback->writeSuccess();

  EXPECT_EQ(handler->writePending_, 0);

  auto bytes2 = folly::IOBuf::copyBuffer("second write");
  Result result2 = handler->onWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(bytes2)));

  EXPECT_EQ(result2, Result::Backpressure);
  // Drain the second pending write so close doesn't hang in Closing.
  capturedCallback->writeSuccess();
}

// Test: Write Error Clears Pending State
TEST_F(TransportHandlerTest, WriteErrorClearsPendingState) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));

  auto bytes = folly::IOBuf::copyBuffer("test write");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_GT(handler->writePending_, 0);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "write failed");
  capturedCallback->writeErr(0, ex);

  EXPECT_EQ(handler->writePending_, 0);
}

// Test: Multiple Writes After Completion
TEST_F(TransportHandlerTest, MultipleWritesAfterCompletion) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .Times(3)
      .WillRepeatedly(SaveArg<0>(&capturedCallback));

  for (int i = 0; i < 3; ++i) {
    auto bytes = folly::IOBuf::copyBuffer("write " + std::to_string(i));
    Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));
    EXPECT_EQ(result, Result::Backpressure);
    EXPECT_GT(handler->writePending_, 0);

    capturedCallback->writeSuccess();
    EXPECT_EQ(handler->writePending_, 0);
  }
}

// --- Read Tests ---

// Test: readBufferAvailable forwards raw bytes to pipeline
TEST_F(TransportHandlerTest, ReadBufferAvailableForwardsToPipeline) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  EXPECT_EQ(appHandler_.readCount(), 0);

  // Build raw bytes (no frame parsing - just raw data)
  auto data = buildTestData(20);
  handler->readBufferAvailable(std::move(data));

  EXPECT_EQ(appHandler_.readCount(), 1);
}

// Test: Multiple readBufferAvailable calls
TEST_F(TransportHandlerTest, MultipleReadBufferAvailableCalls) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  for (int i = 0; i < 5; ++i) {
    auto data = buildTestData(10);
    handler->readBufferAvailable(std::move(data));
  }

  EXPECT_EQ(appHandler_.readCount(), 5);
}

// --- Lifecycle Tests ---

// --- Close Behavior Tests ---

// Test: readEOF closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, ReadEOFCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  EXPECT_NE(handler->pipeline_, nullptr);

  handler->readEOF();

  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
  EXPECT_TRUE(callbackInvoked);
}

// Test: readErr closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, ReadErrCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  EXPECT_NE(handler->pipeline_, nullptr);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "read error");
  handler->readErr(ex);

  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
  EXPECT_TRUE(callbackInvoked);
}

// Test: writeErr closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, WriteErrCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));

  auto bytes = folly::IOBuf::copyBuffer("test write");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));
  EXPECT_EQ(result, Result::Backpressure);

  EXPECT_NE(handler->pipeline_, nullptr);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "write failed");
  capturedCallback->writeErr(0, ex);

  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
  EXPECT_TRUE(callbackInvoked);
}

// Test: onClose closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, OnCloseCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  EXPECT_NE(handler->pipeline_, nullptr);

  handler->close(folly::exception_wrapper{});

  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
  EXPECT_TRUE(callbackInvoked);
}

// Test: Close is idempotent - callback only invoked once
TEST_F(TransportHandlerTest, CloseCallbackInvokedOnlyOnce) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  int callbackCount = 0;
  handler->setCloseCallback([&callbackCount]() { callbackCount++; });

  // Close multiple times via different paths
  handler->close(folly::exception_wrapper{});
  handler->close(
      folly::exception_wrapper{}); // Second call should be idempotent
  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "error");
  handler->readErr(ex);

  // Callback should only be invoked once
  EXPECT_EQ(callbackCount, 1);
}

// Test: Close callback can be set to nullptr (no-op)
TEST_F(TransportHandlerTest, CloseWithNoCallback) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  handler->close(folly::exception_wrapper{});

  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
}

// --- setPipeline Death Tests ---

// Test: setPipeline without reset triggers FATAL
TEST_F(TransportHandlerTest, SetPipelineWithoutResetDeath) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  // Create a second pipeline
  MockAppHandler appHandler2;
  auto pipeline2 =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(handler.get())
          .setTail(&appHandler2)
          .setAllocator(&allocator_)
          .build();

  // Attempting to set a second pipeline without resetting should trigger FATAL
  // (state machine: setPipeline is only valid in Created state).
  EXPECT_DEBUG_DEATH(handler->setPipeline(pipeline2.get()), "");
}

// Test: write with null bytes triggers DCHECK
TEST_F(TransportHandlerTest, WriteWithNullBytesDeath) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  EXPECT_DEBUG_DEATH(
      (void)handler->onWrite(TypeErasedBox(BytesPtr{})), "bytes");
}

// Test: isBufferMovable returns true
TEST_F(TransportHandlerTest, IsBufferMovableReturnsTrue) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  EXPECT_TRUE(handler->isBufferMovable());
}

// Test: setPipeline is a pure setter and does not fire onConnect, even when
// the socket is already good. Activation is the caller's responsibility:
// they call onConnect() once the socket is connected (server: right after
// accept; client: from their AsyncSocket connect callback).
TEST_F(TransportHandlerTest, SetPipelineIsInert) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());

  // Even with a good socket, setPipeline must not touch readCB.
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));
  EXPECT_CALL(*mockSocket, setReadCB(_)).Times(0);

  auto handler = TransportHandler::create(std::move(socket), 256, 4096);

  auto mockHandler = std::make_unique<MockHandler>();
  auto* mockHandlerPtr = mockHandler.get();

  auto pipeline =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(handler.get())
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(
              exception_handler_tag, std::move(mockHandler))
          .build();

  handler->setPipeline(pipeline.get());

  EXPECT_EQ(mockHandlerPtr->pipelineActivatedCount(), 0);
}

// Test: onConnect() fires activation when the socket is already connected.
TEST_F(TransportHandlerTest, OnConnectActivatesPipelineWhenSocketGood) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());

  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));
  // Expect setReadCB called twice: once from start->onConnect (resumeRead),
  // once from destructor (closeImmediately -> pauseRead since not paused).
  EXPECT_CALL(*mockSocket, setReadCB(_)).Times(2);
  EXPECT_CALL(*mockSocket, closeNow()).Times(1);

  auto handler = TransportHandler::create(std::move(socket), 256, 4096);

  auto mockHandler = std::make_unique<MockHandler>();
  auto* mockHandlerPtr = mockHandler.get();

  auto pipeline =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(handler.get())
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(
              exception_handler_tag, std::move(mockHandler))
          .build();

  handler->setPipeline(pipeline.get());
  EXPECT_EQ(mockHandlerPtr->pipelineActivatedCount(), 0);

  handler->onConnect();
  EXPECT_EQ(mockHandlerPtr->pipelineActivatedCount(), 1);
}

// --- Exception Firing Tests ---

// Test: readEOF fires exception to pipeline
TEST_F(TransportHandlerTest, ReadEOFFiresExceptionToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<
                    apache::thrift::transport::TTransportException>());
        return Result::Success;
      });

  handler->readEOF();

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: readErr fires exception to pipeline
TEST_F(TransportHandlerTest, ReadErrFiresExceptionToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<
                    apache::thrift::transport::TTransportException>());
        return Result::Success;
      });

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "read error");
  handler->readErr(ex);

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: writeErr fires exception to pipeline
TEST_F(TransportHandlerTest, WriteErrFiresExceptionToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<folly::AsyncSocketException>());
        return Result::Success;
      });

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));

  auto bytes = folly::IOBuf::copyBuffer("test write");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));
  EXPECT_EQ(result, Result::Backpressure);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "write failed");
  capturedCallback->writeErr(0, ex);

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: onClose with exception fires exception to pipeline
TEST_F(TransportHandlerTest, OnCloseWithExceptionFiresExceptionToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<std::runtime_error>());
        return Result::Success;
      });

  handler->close(
      folly::make_exception_wrapper<std::runtime_error>("connection closed"));

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: onClose without exception does not fire exception to pipeline
TEST_F(TransportHandlerTest, OnCloseWithoutExceptionDoesNotFireException) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  handler->close(folly::exception_wrapper{});

  EXPECT_EQ(mockHandler->exceptionCount(), 0);
}

// --- Disconnect Event Tests ---

// Test: close fires disconnect to all handlers
TEST_F(TransportHandlerTest, CloseFiresDisconnectToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 0);

  handler->close(folly::exception_wrapper{});

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: readEOF fires disconnect to pipeline
TEST_F(TransportHandlerTest, ReadEOFFiresDisconnectToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  handler->readEOF();

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: readErr fires disconnect to pipeline
TEST_F(TransportHandlerTest, ReadErrFiresDisconnectToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "read error");
  handler->readErr(ex);

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: writeErr fires disconnect to pipeline
TEST_F(TransportHandlerTest, WriteErrFiresDisconnectToPipeline) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));

  auto bytes = folly::IOBuf::copyBuffer("test write");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));
  EXPECT_EQ(result, Result::Backpressure);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "write failed");
  capturedCallback->writeErr(0, ex);

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: disconnect is only fired once even with multiple close calls
TEST_F(TransportHandlerTest, DisconnectFiredOnlyOnceOnMultipleClose) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  // Close multiple times
  handler->close(folly::exception_wrapper{});
  handler->close(folly::exception_wrapper{});
  handler->close(folly::exception_wrapper{});

  // Disconnect should only be called once (close is idempotent)
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// --- Destructor Tests ---

// Test: Destructor is idempotent when caller already called close() — no
// second deactivate.
TEST_F(TransportHandlerTest, DestructorNoOpWhenAlreadyClosed) {
  auto [handler, mockHandler, pipeline] =
      createHandlerAndPipelineWithExceptionHandler();
  handler->onConnect();

  handler->close(folly::exception_wrapper{});
  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);

  handler->resetPipeline();
  pipeline.reset();
  handler.reset();
}

// --- State Machine Tests ---

TEST_F(TransportHandlerTest, StateTransitions) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));

  auto handler = TransportHandler::create(std::move(socket), 256, 4096);
  EXPECT_EQ(handler->state(), TransportHandler::State::Created);

  auto pipeline =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(handler.get())
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .build();

  handler->setPipeline(pipeline.get());
  EXPECT_EQ(handler->state(), TransportHandler::State::Ready);

  handler->onConnect();
  EXPECT_EQ(handler->state(), TransportHandler::State::Open);

  // No pending writes -> Open transitions straight to Closed (no drain).
  handler->close(folly::exception_wrapper{});
  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);

  // resetPipeline drops the pipeline pointer; state stays Closed (terminal).
  handler->resetPipeline();
  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);

  pipeline.reset();
}

TEST_F(TransportHandlerTest, OnWriteAcceptedInReadyRejectedInClosed) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  // Ready (pipeline attached, not connected yet) — AsyncSocket buffers
  // pre-connect writes, so we must accept them and hand off to the socket.
  // The mock's default writeChain is a no-op, so writePending_ stays at 1
  // and we return Backpressure.
  folly::AsyncTransport::WriteCallback* readyCb = nullptr;
  EXPECT_CALL(*mockSocket_, writeChain(_, _, _)).WillOnce(SaveArg<0>(&readyCb));
  EXPECT_EQ(
      handler->onWrite(TypeErasedBox(folly::IOBuf::copyBuffer("x"))),
      Result::Backpressure);
  readyCb->writeSuccess();

  handler->onConnect();
  folly::AsyncTransport::WriteCallback* openCb = nullptr;
  EXPECT_CALL(*mockSocket_, writeChain(_, _, _)).WillOnce(SaveArg<0>(&openCb));
  EXPECT_EQ(
      handler->onWrite(TypeErasedBox(folly::IOBuf::copyBuffer("x"))),
      Result::Backpressure);
  // Drain so socketDrainer doesn't pin us alive after close.
  openCb->writeSuccess();

  handler->close(folly::exception_wrapper{});
  // state == Closed (no pending writes -> direct transition)
  EXPECT_EQ(
      handler->onWrite(TypeErasedBox(folly::IOBuf::copyBuffer("x"))),
      Result::Error);

  handler->resetPipeline();
  pipeline.reset();
}

TEST_F(TransportHandlerTest, OnConnectFromCreatedDeath) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));

  auto handler = TransportHandler::create(std::move(socket), 256, 4096);
  // state == Created (no setPipeline yet)
  EXPECT_DEBUG_DEATH(handler->onConnect(), "");
}

TEST_F(TransportHandlerTest, OnConnectTwiceDeath) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();
  EXPECT_DEBUG_DEATH(handler->onConnect(), "");
  // Don't call resetPipeline directly — destructor handles it (forces close
  // from Open first). resetPipeline on Open would DCHECK-fail by design.
}

TEST_F(TransportHandlerTest, ResetPipelineDeathFromOpen) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();
  // resetPipeline is a destruction helper, not a public state-machine entry.
  // Calling it while still Open / Closing must DCHECK-fail.
  EXPECT_DEBUG_DEATH(handler->resetPipeline(), "");
}

// --- Drain Tests ---

// THE original bug: multi-WR queued; peer disconnect cascades writeErrs
// through AsyncSocket's failAllWrites loop. closeCallback_ fires from the
// FIRST writeErr and synchronously *destroys* the connection wrapper that
// owns the handler (mimicking ConnectionHandler::removeConnection →
// connections_.erase). The drain guard inside SocketDrainer defers actual
// deletion until the cascade finishes — without it, the 2nd/3rd writeErr
// would dispatch on freed memory.
TEST_F(TransportHandlerTest, MultiWrErrCascadeDoesNotUAF) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));

  // Member declaration: pipeline first, handler second. struct member
  // destruction is reverse of declaration → handler.Ptr.reset() runs first
  // (defers via drain guard), then pipeline.Ptr.reset() (defers via TH's
  // pipelineGuard_). Both defer; actual destruction happens later when
  // the drain completes (via cascading writeErrs releasing drainGuard).
  struct ConnectionWrapper {
    PipelineImpl::Ptr pipeline;
    TransportHandler::Ptr handler;
  };
  auto wrapper = std::make_unique<ConnectionWrapper>();
  wrapper->handler = TransportHandler::create(std::move(socket), 256, 4096);
  wrapper->pipeline =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(wrapper->handler.get())
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .build();
  wrapper->handler->setPipeline(wrapper->pipeline.get());
  wrapper->handler->onConnect();
  auto* rawHandler = wrapper->handler.get();

  std::vector<folly::AsyncTransport::WriteCallback*> capturedCallbacks;
  EXPECT_CALL(*mockSocket, writeChain(_, _, _))
      .Times(3)
      .WillRepeatedly(
          [&](folly::AsyncTransport::WriteCallback* cb,
              std::shared_ptr<folly::IOBuf>,
              folly::WriteFlags) { capturedCallbacks.push_back(cb); });

  for (int i = 0; i < 3; ++i) {
    (void)rawHandler->onWrite(TypeErasedBox(folly::IOBuf::copyBuffer("x")));
  }
  EXPECT_EQ(rawHandler->writePending_, 3u);

  // closeCallback_ destroys the wrapper synchronously — exactly what
  // removeConnection does in production. Both members defer destruction
  // (handler via drainGuard, pipeline via pipelineGuard_) until the drain
  // cascades writeErrs and writePending_ hits 0.
  bool callbackFired = false;
  auto wrapperHolder =
      std::make_shared<std::unique_ptr<ConnectionWrapper>>(std::move(wrapper));
  rawHandler->setCloseCallback([&callbackFired, wrapperHolder]() {
    callbackFired = true;
    wrapperHolder->reset();
  });

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "broken pipe");

  // First two writeErrs: state goes Open -> Closing on the first; subsequent
  // ones just decrement pending. closeCallback does NOT fire yet — it only
  // fires when state reaches Closed (after drain completes).
  capturedCallbacks[0]->writeErr(0, ex);
  EXPECT_FALSE(callbackFired);
  EXPECT_EQ(rawHandler->writePending_, 2u);
  EXPECT_EQ(rawHandler->state(), TransportHandler::State::Closing);
  EXPECT_TRUE(rawHandler->socketDrainer_.active());

  capturedCallbacks[1]->writeErr(0, ex);
  EXPECT_FALSE(callbackFired);
  EXPECT_EQ(rawHandler->writePending_, 1u);

  // Last writeErr drains pending to 0, drainer releases -> closeNow ->
  // closeCallback fires -> wrapper is torn down -> handler.destroy() is
  // called. The local DG inside writeErr defers actual delete until the
  // call returns; ASan would catch any UAF in the cascade.
  capturedCallbacks[2]->writeErr(0, ex);
  EXPECT_TRUE(callbackFired);
  // rawHandler is now freed; do not touch it.
}

TEST_F(TransportHandlerTest, DrainGuardReleasedByWriteSuccessAfterClose) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* cb = nullptr;
  EXPECT_CALL(*mockSocket_, writeChain(_, _, _)).WillOnce(SaveArg<0>(&cb));
  (void)handler->onWrite(TypeErasedBox(folly::IOBuf::copyBuffer("x")));
  EXPECT_EQ(handler->writePending_, 1u);

  handler->close(folly::exception_wrapper{});
  // writePending_ > 0 → state Closing, drain guard held.
  EXPECT_EQ(handler->state(), TransportHandler::State::Closing);
  EXPECT_TRUE(handler->socketDrainer_.active());

  // Pending write completes naturally; drainer transitions Closing -> Closed.
  cb->writeSuccess();
  EXPECT_EQ(handler->writePending_, 0u);
  EXPECT_FALSE(handler->socketDrainer_.active());
  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);

  handler->resetPipeline();
  pipeline.reset();
}

TEST_F(TransportHandlerTest, NoDrainGuardWhenNoPendingWrites) {
  auto [handler, pipeline] = createHandlerAndPipeline();
  handler->onConnect();

  handler->close(folly::exception_wrapper{});
  EXPECT_EQ(handler->state(), TransportHandler::State::Closed);
  EXPECT_FALSE(handler->socketDrainer_.active());

  handler->resetPipeline();
  pipeline.reset();
}

TEST_F(TransportHandlerTest, DrainTimeoutForcesClose) {
  // Use a short drain timeout so the test runs quickly.
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));

  auto handler = TransportHandler::create(
      std::move(socket), 256, 4096, std::chrono::milliseconds(20));

  auto pipeline =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(handler.get())
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .build();
  handler->setPipeline(pipeline.get());
  handler->onConnect();

  folly::AsyncTransport::WriteCallback* cb = nullptr;
  EXPECT_CALL(*mockSocket, writeChain(_, _, _)).WillOnce(SaveArg<0>(&cb));
  (void)handler->onWrite(TypeErasedBox(folly::IOBuf::copyBuffer("x")));

  handler->close(folly::exception_wrapper{});
  EXPECT_TRUE(handler->socketDrainer_.active());

  // closeNow() in onDrainTimeout simulates AsyncSocket's force-cascade. We
  // simulate that by having the mock's closeNow invoke writeErr on the
  // captured callback (mimicking failAllWrites), then break out of the loop.
  bool forceCloseFired = false;
  EXPECT_CALL(*mockSocket, closeNow()).WillOnce([&]() {
    forceCloseFired = true;
    folly::AsyncSocketException ex(
        folly::AsyncSocketException::NETWORK_ERROR, "force-closed");
    cb->writeErr(0, ex);
    evb_.terminateLoopSoon();
  });

  // Run the EventBase until the drain timer fires and the mock terminates it.
  evb_.loop();

  EXPECT_TRUE(forceCloseFired);
  EXPECT_FALSE(handler->socketDrainer_.active());
  EXPECT_EQ(handler->writePending_, 0u);

  handler->resetPipeline();
  pipeline.reset();
}

// External close() with no pending writes: closeCallback_ fires, drops the
// owning wrapper synchronously. close()'s local DG defers actual destroy
// until close returns. No drain guard taken (writePending_ == 0).
TEST_F(TransportHandlerTest, CloseCallbackDropsOwnerSynchronously) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  ON_CALL(*mockSocket, getEventBase()).WillByDefault(Return(&evb_));

  struct ConnectionWrapper {
    PipelineImpl::Ptr pipeline;
    TransportHandler::Ptr handler;
  };
  auto wrapper = std::make_unique<ConnectionWrapper>();
  wrapper->handler = TransportHandler::create(std::move(socket), 256, 4096);
  wrapper->pipeline =
      PipelineBuilder<TransportHandler, MockAppHandler, SimpleBufferAllocator>()
          .setEventBase(&evb_)
          .setHead(wrapper->handler.get())
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .build();
  wrapper->handler->setPipeline(wrapper->pipeline.get());
  wrapper->handler->onConnect();
  auto* rawHandler = wrapper->handler.get();

  bool callbackFired = false;
  auto* wrapperRaw = wrapper.get();
  rawHandler->setCloseCallback([&, wrapperRaw]() {
    callbackFired = true;
    wrapperRaw->handler->resetPipeline();
    wrapperRaw->pipeline.reset();
    wrapperRaw->handler.reset();
  });

  rawHandler->close(folly::exception_wrapper{});
  EXPECT_TRUE(callbackFired);
  // rawHandler is freed at this point; wrapper itself just holds empty
  // unique_ptrs.
  wrapper.reset();
}

} // namespace apache::thrift::fast_thrift::transport
