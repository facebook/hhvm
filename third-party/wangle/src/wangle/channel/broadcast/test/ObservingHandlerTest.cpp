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

#include <wangle/channel/broadcast/test/Mocks.h>
#include <wangle/channel/test/MockHandler.h>
#include <wangle/codec/MessageToByteEncoder.h>

using namespace wangle;
using namespace folly;
using namespace testing;

class ObservingHandlerTest : public Test {
 public:
  class MockIntToByteEncoder : public MessageToByteEncoder<int> {
   public:
    std::unique_ptr<IOBuf> encode(int& data) override {
      return IOBuf::copyBuffer(folly::to<std::string>(data));
    }
  };

  void SetUp() override {
    prevHandler = new StrictMock<MockBytesToBytesHandler>();
    observingHandler = new StrictMock<MockObservingHandler>(&pool);
    broadcastHandler = std::make_unique<StrictMock<MockBroadcastHandler>>();

    pipeline = ObservingPipeline<int>::create();
    pipeline->addBack(
        std::shared_ptr<StrictMock<MockBytesToBytesHandler>>(prevHandler));
    pipeline->addBack(MockIntToByteEncoder());
    pipeline->addBack(
        std::shared_ptr<StrictMock<MockObservingHandler>>(observingHandler));
    pipeline->finalize();
  }

  void TearDown() override {
    Mock::VerifyAndClear(broadcastHandler.get());

    broadcastHandler.reset();
    pipeline.reset();
  }

 protected:
  ObservingPipeline<int>::Ptr pipeline;

  StrictMock<MockBytesToBytesHandler>* prevHandler{nullptr};
  StrictMock<MockObservingHandler>* observingHandler{nullptr};
  std::unique_ptr<StrictMock<MockBroadcastHandler>> broadcastHandler;

  StrictMock<MockBroadcastPool> pool;
};

TEST_F(ObservingHandlerTest, Success) {
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          broadcastHandler.get())));
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(1);
  // Verify that ingress is resumed
  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));

  // Initialize the pipeline
  pipeline->transportActive();

  EXPECT_CALL(*observingHandler, mockWrite(_, 1))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));
  EXPECT_CALL(*observingHandler, mockWrite(_, 2))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Broadcast some data
  observingHandler->onNext(1);
  observingHandler->onNext(2);

  EXPECT_CALL(*observingHandler, mockClose(_))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Finish the broadcast
  observingHandler->onCompleted();
}

TEST_F(ObservingHandlerTest, ConnectError) {
  // Test when an error occurs while fetching the broadcast handler
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  // Inject error
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          make_exception_wrapper<std::exception>())));
  EXPECT_CALL(*observingHandler, mockClose(_))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Initialize the pipeline
  pipeline->transportActive();
}

TEST_F(ObservingHandlerTest, ConnectHandlerDeletion) {
  // Test when the handler goes away before the broadcast handler
  // is obtained
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  Promise<BroadcastHandler<int, std::string>*> promise;
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(makeMoveWrapper(promise.getFuture())));

  // Initialize the pipeline
  pipeline->transportActive();

  // Delete the handler and then fulfil the promise
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(0);
  pipeline.reset();
  promise.setValue(broadcastHandler.get());
}

TEST_F(ObservingHandlerTest, ConnectErrorHandlerDeletion) {
  // Test when an error occurs while fetching the broadcast handler
  // after the handler is deleted
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  Promise<BroadcastHandler<int, std::string>*> promise;
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(makeMoveWrapper(promise.getFuture())));

  // Initialize the pipeline
  pipeline->transportActive();

  // Delete the handler and then inject an error
  pipeline.reset();
  promise.setException(std::exception());
}

TEST_F(ObservingHandlerTest, BroadcastError) {
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          broadcastHandler.get())));
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(1);
  // Verify that ingress is resumed
  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));

  // Initialize the pipeline
  pipeline->transportActive();

  EXPECT_CALL(*observingHandler, mockWrite(_, _))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Broadcast some data
  observingHandler->onNext(1);

  EXPECT_CALL(*observingHandler, mockClose(_))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Inject broadcast error
  observingHandler->onError(make_exception_wrapper<std::exception>());
}

TEST_F(ObservingHandlerTest, ReadEOF) {
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          broadcastHandler.get())));
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(1);
  // Verify that ingress is resumed
  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));

  // Initialize the pipeline
  pipeline->transportActive();

  EXPECT_CALL(*observingHandler, mockWrite(_, _))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Broadcast some data
  observingHandler->onNext(1);

  EXPECT_CALL(*observingHandler, mockClose(_)).WillOnce(InvokeWithoutArgs([&] {
    // Delete the pipeline
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));
  EXPECT_CALL(*broadcastHandler, unsubscribe(_)).Times(1);

  // Client closes connection
  observingHandler->readEOF(nullptr);
}

TEST_F(ObservingHandlerTest, ReadError) {
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          broadcastHandler.get())));
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(1);
  // Verify that ingress is resumed
  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));

  // Initialize the pipeline
  pipeline->transportActive();

  EXPECT_CALL(*observingHandler, mockWrite(_, _))
      .WillOnce(Return(makeMoveWrapper(makeFuture())));

  // Broadcast some data
  observingHandler->onNext(1);

  EXPECT_CALL(*observingHandler, mockClose(_)).WillOnce(InvokeWithoutArgs([&] {
    // Delete the pipeline
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));
  EXPECT_CALL(*broadcastHandler, unsubscribe(_)).Times(1);

  // Inject read error
  observingHandler->readException(
      nullptr, make_exception_wrapper<std::exception>());
}

TEST_F(ObservingHandlerTest, WriteError) {
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          broadcastHandler.get())));
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(1);
  // Verify that ingress is resumed
  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));

  // Initialize the pipeline
  pipeline->transportActive();

  // Inject write error
  EXPECT_CALL(*observingHandler, mockWrite(_, _))
      .WillOnce(Return(
          MoveWrapper<Future<Unit>>(make_exception_wrapper<std::exception>())));
  EXPECT_CALL(*observingHandler, mockClose(_)).WillOnce(InvokeWithoutArgs([&] {
    // Delete the pipeline
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));
  EXPECT_CALL(*broadcastHandler, unsubscribe(_)).Times(1);

  // Broadcast some data
  observingHandler->onNext(1);
}

TEST_F(ObservingHandlerTest, WriteErrorHandlerDeletion) {
  // Test when write error occurs asynchronously after the handler
  // has been deleted.
  InSequence dummy;

  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));
  // Verify that ingress is paused
  EXPECT_CALL(*prevHandler, transportInactive(_)).WillOnce(Return());
  EXPECT_CALL(pool, mockGetHandler(_))
      .WillOnce(Return(MoveWrapper<Future<BroadcastHandler<int, std::string>*>>(
          broadcastHandler.get())));
  EXPECT_CALL(*broadcastHandler, subscribe(_)).Times(1);
  // Verify that ingress is resumed
  EXPECT_CALL(*prevHandler, transportActive(_))
      .WillOnce(Invoke([&](MockBytesToBytesHandler::Context* ctx) {
        ctx->fireTransportActive();
      }));

  // Initialize the pipeline
  pipeline->transportActive();

  Promise<Unit> promise;
  EXPECT_CALL(*observingHandler, mockWrite(_, _))
      .WillOnce(Return(makeMoveWrapper(promise.getFuture())));

  // Broadcast some data
  observingHandler->onNext(1);

  // Delete the pipeline and then fail the write
  EXPECT_CALL(*broadcastHandler, unsubscribe(_)).Times(1);
  pipeline.reset();
  promise.setException(std::exception());
}
