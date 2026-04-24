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
#include <folly/synchronization/Baton.h>
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

    handler->setPipeline(*pipeline);

    return {std::move(handler), std::move(pipeline)};
  }

  /**
   * Creates a TransportHandler and Pipeline with a MockHandler that can capture
   * exceptions.
   */
  std::tuple<TransportHandler::Ptr, PipelineImpl::Ptr, MockHandler*>
  createHandlerAndPipelineWithExceptionHandler() {
    auto socket = folly::AsyncTransport::UniquePtr(
        new NiceMock<folly::test::MockAsyncTransport>());
    mockSocket_ = static_cast<folly::test::MockAsyncTransport*>(socket.get());

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

    handler->setPipeline(*pipeline);

    return {std::move(handler), std::move(pipeline), mockHandlerPtr};
  }

  folly::EventBase evb_;
  folly::test::MockAsyncTransport* mockSocket_{nullptr};
  MockAppHandler appHandler_;
  SimpleBufferAllocator allocator_;
};

// Test: Write Path
TEST_F(TransportHandlerTest, WritePath) {
  auto [handler, pipeline] = createHandlerAndPipeline();

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

  // Destructor closeInternal uses pauseRead() which is idempotent - no extra
  // setReadCB(nullptr) call since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);
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

  // Destructor closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);
}

// Test: Resume Read is idempotent
TEST_F(TransportHandlerTest, ResumeReadIdempotent) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  // Initially paused
  EXPECT_TRUE(handler->readPaused_);

  // First resume should set callback
  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  // Destructor will call closeInternal which calls setReadCB(nullptr) and
  // closeNow
  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);
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

  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);
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

  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);
}

// Test: onConnect resumes reading and fires connect event to pipeline
TEST_F(TransportHandlerTest, OnConnectResumesReadAndFiresConnect) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // Initially paused
  EXPECT_TRUE(handler->readPaused_);
  EXPECT_EQ(mockHandler->pipelineActivatedCount(), 0);

  // onConnect should resume reading and fire connect to the pipeline
  EXPECT_CALL(*mockSocket_, setReadCB(handler.get())).Times(1);
  // Destructor will call closeInternal which calls setReadCB(nullptr) and
  // closeNow
  EXPECT_CALL(*mockSocket_, setReadCB(nullptr)).Times(1);
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);
  handler->onConnect();

  EXPECT_FALSE(handler->readPaused_);
  EXPECT_EQ(mockHandler->pipelineActivatedCount(), 1);
}

// Test: Write Backpressure When Write Pending
TEST_F(TransportHandlerTest, WriteBackpressureWhenWritePending) {
  auto [handler, pipeline] = createHandlerAndPipeline();

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
}

// Test: Write Success Clears Pending State
TEST_F(TransportHandlerTest, WriteSuccessClearsPendingState) {
  auto [handler, pipeline] = createHandlerAndPipeline();

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
}

// Test: Write Error Clears Pending State
TEST_F(TransportHandlerTest, WriteErrorClearsPendingState) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));
  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

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

// Test: Pipeline reset during pending write completes safely
TEST_F(TransportHandlerTest, PipelineResetDuringPendingWriteCompletesSafely) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));

  // Start a write operation
  auto bytes = folly::IOBuf::copyBuffer("test data");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));
  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_GT(handler->writePending_, 0);

  // Reset the pipeline while write is pending
  // The DestructorGuard in handler should keep pipeline alive
  pipeline.reset();

  // Complete the write via callback - should not crash
  // because pipeline is kept alive by pipelineGuard_
  capturedCallback->writeSuccess();
  EXPECT_EQ(handler->writePending_, 0);
}

// Test: Pipeline reset during read operation completes safely
TEST_F(TransportHandlerTest, PipelineResetDuringReadCompletesSafely) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  folly::Baton<> baton;
  bool readCompleted = false;

  // Set up app handler to wait on a baton during onMessage
  appHandler_.setOnReadCallback([&](TypeErasedBox&&) {
    baton.wait();
    readCompleted = true;
    return Result::Success;
  });

  // Schedule onRead to run through the eventBase
  evb_.runInEventBaseThread([&handler]() {
    auto bytes = folly::IOBuf::copyBuffer("test data");
    handler->readBufferAvailable(std::move(bytes));
  });

  // Drive the event base to start the read (but it will block on baton)
  std::thread t1([evb = &evb_]() { evb->loopOnce(); });

  // Reset the pipeline while read is in progress (blocked on baton)
  pipeline.reset();

  // Post to baton to let the read complete
  baton.post();

  // Wait for the read to complete
  t1.join();

  // Drive event loop to completion
  evb_.loopOnce();

  // Verify the read completed successfully
  EXPECT_TRUE(readCompleted);
  EXPECT_EQ(appHandler_.readCount(), 1);
}

// --- Close Behavior Tests ---

// Test: readEOF closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, ReadEOFCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  EXPECT_NE(handler->pipeline_, nullptr);
  EXPECT_NE(handler->pipelineGuard_, nullptr);

  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->readEOF();

  EXPECT_TRUE(handler->closed_);
  EXPECT_EQ(handler->pipeline_, nullptr);
  EXPECT_EQ(handler->pipelineGuard_, nullptr);
  EXPECT_TRUE(callbackInvoked);
}

// Test: readErr closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, ReadErrCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  EXPECT_NE(handler->pipeline_, nullptr);
  EXPECT_NE(handler->pipelineGuard_, nullptr);

  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "read error");
  handler->readErr(ex);

  EXPECT_TRUE(handler->closed_);
  EXPECT_EQ(handler->pipeline_, nullptr);
  EXPECT_EQ(handler->pipelineGuard_, nullptr);
  EXPECT_TRUE(callbackInvoked);
}

// Test: writeErr closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, WriteErrCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));
  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  auto bytes = folly::IOBuf::copyBuffer("test write");
  Result result = handler->onWrite(TypeErasedBox(std::move(bytes)));
  EXPECT_EQ(result, Result::Backpressure);

  EXPECT_NE(handler->pipeline_, nullptr);
  EXPECT_NE(handler->pipelineGuard_, nullptr);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "write failed");
  capturedCallback->writeErr(0, ex);

  EXPECT_TRUE(handler->closed_);
  EXPECT_EQ(handler->pipeline_, nullptr);
  EXPECT_EQ(handler->pipelineGuard_, nullptr);
  EXPECT_TRUE(callbackInvoked);
}

// Test: onClose closes socket, resets pipeline, and invokes close callback
TEST_F(TransportHandlerTest, OnCloseCloseBehavior) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  EXPECT_NE(handler->pipeline_, nullptr);
  EXPECT_NE(handler->pipelineGuard_, nullptr);

  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->onClose(folly::exception_wrapper{});

  EXPECT_TRUE(handler->closed_);
  EXPECT_EQ(handler->pipeline_, nullptr);
  EXPECT_EQ(handler->pipelineGuard_, nullptr);
  EXPECT_TRUE(callbackInvoked);
}

// Test: Close is idempotent - callback only invoked once
TEST_F(TransportHandlerTest, CloseCallbackInvokedOnlyOnce) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  int callbackCount = 0;
  handler->setCloseCallback([&callbackCount]() { callbackCount++; });

  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  // Close multiple times via different paths
  handler->onClose(folly::exception_wrapper{});
  handler->onClose(
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

  // No callback set - should not crash
  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->onClose(folly::exception_wrapper{});

  EXPECT_TRUE(handler->closed_);
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
  EXPECT_DEATH(handler->setPipeline(*pipeline2), "must reset pipeline");
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

// Test: setPipeline fires onConnect when socket is already connected
TEST_F(TransportHandlerTest, SetPipelineFiresConnectWhenSocketGood) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());

  // Mock socket->good() returning true (already connected)
  ON_CALL(*mockSocket, good()).WillByDefault(Return(true));
  // Expect setReadCB called twice: once from onConnect (resumeRead), once from
  // destructor (closeInternal -> pauseRead since not paused)
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

  // Initially no connect received
  EXPECT_EQ(mockHandlerPtr->pipelineActivatedCount(), 0);

  // setPipeline should fire onConnect since socket is good
  handler->setPipeline(*pipeline);

  EXPECT_EQ(mockHandlerPtr->pipelineActivatedCount(), 1);
}

// Test: setPipeline does not fire onConnect when socket is not connected
TEST_F(TransportHandlerTest, SetPipelineDoesNotFireConnectWhenSocketNotGood) {
  auto socket = folly::AsyncTransport::UniquePtr(
      new NiceMock<folly::test::MockAsyncTransport>());
  auto* mockSocket =
      static_cast<folly::test::MockAsyncTransport*>(socket.get());

  // Mock socket->good() returning false (not connected)
  ON_CALL(*mockSocket, good()).WillByDefault(Return(false));

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

  // setPipeline should NOT fire onConnect since socket is not good
  handler->setPipeline(*pipeline);

  EXPECT_EQ(mockHandlerPtr->pipelineActivatedCount(), 0);
}

// --- Exception Firing Tests ---

// Test: readEOF fires exception to pipeline
TEST_F(TransportHandlerTest, ReadEOFFiresExceptionToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<
                    apache::thrift::transport::TTransportException>());
        return Result::Success;
      });

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->readEOF();

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: readErr fires exception to pipeline
TEST_F(TransportHandlerTest, ReadErrFiresExceptionToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<
                    apache::thrift::transport::TTransportException>());
        return Result::Success;
      });

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "read error");
  handler->readErr(ex);

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: writeErr fires exception to pipeline
TEST_F(TransportHandlerTest, WriteErrFiresExceptionToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

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
  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

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
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  bool exceptionReceived = false;
  mockHandler->setOnException(
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          const folly::exception_wrapper& e) {
        exceptionReceived = true;
        EXPECT_TRUE(e.is_compatible_with<std::runtime_error>());
        return Result::Success;
      });

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->onClose(
      folly::make_exception_wrapper<std::runtime_error>("connection closed"));

  EXPECT_TRUE(exceptionReceived);
  EXPECT_EQ(mockHandler->exceptionCount(), 1);
}

// Test: onClose without exception does not fire exception to pipeline
TEST_F(TransportHandlerTest, OnCloseWithoutExceptionDoesNotFireException) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->onClose(folly::exception_wrapper{});

  EXPECT_EQ(mockHandler->exceptionCount(), 0);
}

// --- Disconnect Event Tests ---

// Test: closeInternal fires disconnect to all handlers
TEST_F(TransportHandlerTest, CloseFiresDisconnectToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 0);

  handler->onClose(folly::exception_wrapper{});

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: readEOF fires disconnect to pipeline
TEST_F(TransportHandlerTest, ReadEOFFiresDisconnectToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  handler->readEOF();

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: readErr fires disconnect to pipeline
TEST_F(TransportHandlerTest, ReadErrFiresDisconnectToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "read error");
  handler->readErr(ex);

  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: writeErr fires disconnect to pipeline
TEST_F(TransportHandlerTest, WriteErrFiresDisconnectToPipeline) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  folly::AsyncTransport::WriteCallback* capturedCallback = nullptr;

  EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
      .WillOnce(SaveArg<0>(&capturedCallback));
  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

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
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  // Close multiple times
  handler->onClose(folly::exception_wrapper{});
  handler->onClose(folly::exception_wrapper{});
  handler->onClose(folly::exception_wrapper{});

  // Disconnect should only be called once (close is idempotent)
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// --- Destructor Tests ---

// Test: Destructor closes transport and fires disconnect when not already
// closed
TEST_F(TransportHandlerTest, DestructorClosesTransportWhenNotAlreadyClosed) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  EXPECT_FALSE(handler->closed_);
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 0);

  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  // Destroy the handler - destructor should call closeInternal
  handler.reset();

  // Verify disconnect was fired
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: Destructor is idempotent - no double close if already closed
TEST_F(TransportHandlerTest, DestructorNoOpWhenAlreadyClosed) {
  auto [handler, pipeline, mockHandler] =
      createHandlerAndPipelineWithExceptionHandler();

  // closeInternal uses pauseRead() which is idempotent
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  // Explicitly close
  handler->onClose(folly::exception_wrapper{});
  EXPECT_TRUE(handler->closed_);
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);

  // Destroy the handler - destructor should not close again
  handler.reset();

  // Verify disconnect was only fired once (from explicit close, not destructor)
  EXPECT_EQ(mockHandler->pipelineDeactivatedCount(), 1);
}

// Test: Destructor invokes close callback when not already closed
TEST_F(TransportHandlerTest, DestructorInvokesCloseCallback) {
  auto [handler, pipeline] = createHandlerAndPipeline();

  bool callbackInvoked = false;
  handler->setCloseCallback([&callbackInvoked]() { callbackInvoked = true; });

  // closeInternal uses pauseRead() which is idempotent - no setReadCB call
  // since already paused
  EXPECT_CALL(*mockSocket_, closeNow()).Times(1);

  // Destroy the handler - destructor should invoke close callback
  handler.reset();

  EXPECT_TRUE(callbackInvoked);
}

} // namespace apache::thrift::fast_thrift::transport
