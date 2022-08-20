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

using namespace wangle;
using namespace folly;
using namespace testing;

class BroadcastHandlerTest : public Test {
 public:
  class MockBroadcastHandler
      : public BroadcastHandler<std::string, std::string> {
   public:
    MOCK_METHOD1(
        mockClose,
        folly::MoveWrapper<folly::Future<folly::Unit>>(Context*));

    folly::Future<folly::Unit> close(Context* ctx) override {
      return mockClose(ctx).move();
    }
  };

  void SetUp() override {
    prevHandler = new StrictMock<MockBytesToBytesHandler>();
    EXPECT_CALL(*prevHandler, read(_, _))
        .WillRepeatedly(Invoke([&](MockBytesToBytesHandler::Context* ctx,
                                   IOBufQueue& q) { ctx->fireRead(q); }));

    decoder = new StrictMock<MockByteToMessageDecoder<std::string>>();
    handler = new StrictMock<MockBroadcastHandler>();

    pipeline = DefaultPipeline::create();
    pipeline->addBack(
        std::shared_ptr<StrictMock<MockBytesToBytesHandler>>(prevHandler));
    pipeline->addBack(
        std::shared_ptr<StrictMock<MockByteToMessageDecoder<std::string>>>(
            decoder));
    pipeline->addBack(
        std::shared_ptr<StrictMock<MockBroadcastHandler>>(handler));
    pipeline->finalize();
  }

  void TearDown() override {
    Mock::VerifyAndClear(&subscriber0);
    Mock::VerifyAndClear(&subscriber1);

    pipeline.reset();
  }

 protected:
  DefaultPipeline::Ptr pipeline;

  StrictMock<MockBytesToBytesHandler>* prevHandler{nullptr};
  StrictMock<MockByteToMessageDecoder<std::string>>* decoder{nullptr};
  StrictMock<MockBroadcastHandler>* handler{nullptr};

  StrictMock<MockSubscriber<std::string, std::string>> subscriber0;
  StrictMock<MockSubscriber<std::string, std::string>> subscriber1;
};

TEST_F(BroadcastHandlerTest, SubscribeUnsubscribe) {
  // Test by adding a couple of subscribers and unsubscribing them
  EXPECT_CALL(*decoder, decode(_, _, _, _))
      .WillRepeatedly(
          Invoke([&](MockByteToMessageDecoder<std::string>::Context*,
                     IOBufQueue& q,
                     std::string& data,
                     size_t&) {
            auto buf = q.move();
            if (buf) {
              buf->coalesce();
              data = buf->moveToFbString().toStdString();
              return true;
            }
            return false;
          }));

  InSequence dummy;

  // Add a subscriber
  EXPECT_EQ(handler->subscribe(&subscriber0), 0);

  EXPECT_CALL(subscriber0, onNext("data1")).Times(1);
  EXPECT_CALL(subscriber0, onNext("data2")).Times(1);

  // Push some data
  IOBufQueue q;
  q.append(IOBuf::copyBuffer("data1"));
  pipeline->read(q);
  q.reset();
  q.append(IOBuf::copyBuffer("data2"));
  pipeline->read(q);
  q.reset();

  // Add another subscriber
  EXPECT_EQ(handler->subscribe(&subscriber1), 1);

  EXPECT_CALL(subscriber0, onNext("data3")).Times(1);
  EXPECT_CALL(subscriber1, onNext("data3")).Times(1);

  // Push more data
  q.append(IOBuf::copyBuffer("data3"));
  pipeline->read(q);
  q.reset();

  // Unsubscribe one of the subscribers
  handler->unsubscribe(0);

  EXPECT_CALL(subscriber1, onNext(Eq("data4"))).Times(1);

  // Push more data
  q.append(IOBuf::copyBuffer("data4"));
  pipeline->read(q);
  q.reset();

  EXPECT_CALL(*handler, mockClose(_)).WillOnce(InvokeWithoutArgs([this] {
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));

  // Unsubscribe the other subscriber. The handler should be deleted now.
  handler->unsubscribe(1);
}

TEST_F(BroadcastHandlerTest, BufferedRead) {
  // Test with decoder that buffers data based on some local logic
  // before pushing to subscribers
  IOBufQueue bufQueue{IOBufQueue::cacheChainLength()};
  EXPECT_CALL(*decoder, decode(_, _, _, _))
      .WillRepeatedly(
          Invoke([&](MockByteToMessageDecoder<std::string>::Context*,
                     IOBufQueue& q,
                     std::string& data,
                     size_t&) {
            bufQueue.append(q);
            if (bufQueue.chainLength() < 5) {
              return false;
            }
            auto buf = bufQueue.move();
            buf->coalesce();
            data = buf->moveToFbString().toStdString();
            return true;
          }));

  InSequence dummy;

  // Add a subscriber
  EXPECT_EQ(handler->subscribe(&subscriber0), 0);

  EXPECT_CALL(subscriber0, onNext("data1")).Times(1);

  // Push some fragmented data
  IOBufQueue q;
  q.append(IOBuf::copyBuffer("da"));
  pipeline->read(q);
  q.reset();
  q.append(IOBuf::copyBuffer("ta1"));
  pipeline->read(q);
  q.reset();

  // Push more fragmented data. onNext shouldn't be called yet.
  q.append(IOBuf::copyBuffer("dat"));
  pipeline->read(q);
  q.reset();
  q.append(IOBuf::copyBuffer("a"));
  pipeline->read(q);
  q.reset();

  // Add another subscriber
  EXPECT_EQ(handler->subscribe(&subscriber1), 1);

  EXPECT_CALL(subscriber0, onNext("data3data4")).Times(1);
  EXPECT_CALL(subscriber1, onNext("data3data4")).Times(1);

  // Push rest of the fragmented data. The entire data should be pushed
  // to both subscribers.
  q.append(IOBuf::copyBuffer("3data4"));
  pipeline->read(q);
  q.reset();

  EXPECT_CALL(subscriber0, onNext("data2")).Times(1);
  EXPECT_CALL(subscriber1, onNext("data2")).Times(1);

  // Push some unfragmented data
  q.append(IOBuf::copyBuffer("data2"));
  pipeline->read(q);
  q.reset();

  EXPECT_CALL(*handler, mockClose(_)).WillOnce(InvokeWithoutArgs([this] {
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));

  // Unsubscribe all subscribers. The handler should be deleted now.
  handler->unsubscribe(0);
  handler->unsubscribe(1);
}

TEST_F(BroadcastHandlerTest, OnCompleted) {
  // Test with EOF on the handler
  EXPECT_CALL(*decoder, decode(_, _, _, _))
      .WillRepeatedly(
          Invoke([&](MockByteToMessageDecoder<std::string>::Context*,
                     IOBufQueue& q,
                     std::string& data,
                     size_t&) {
            auto buf = q.move();
            if (buf) {
              buf->coalesce();
              data = buf->moveToFbString().toStdString();
              return true;
            }
            return false;
          }));

  InSequence dummy;

  // Add a subscriber
  EXPECT_EQ(handler->subscribe(&subscriber0), 0);

  EXPECT_CALL(subscriber0, onNext("data1")).Times(1);

  // Push some data
  IOBufQueue q;
  q.append(IOBuf::copyBuffer("data1"));
  pipeline->read(q);
  q.reset();

  // Add another subscriber
  EXPECT_EQ(handler->subscribe(&subscriber1), 1);

  EXPECT_CALL(subscriber0, onNext("data2")).Times(1);
  EXPECT_CALL(subscriber1, onNext("data2")).Times(1);

  // Push more data
  q.append(IOBuf::copyBuffer("data2"));
  pipeline->read(q);
  q.reset();

  // Unsubscribe one of the subscribers
  handler->unsubscribe(0);

  EXPECT_CALL(subscriber1, onCompleted()).Times(1);

  EXPECT_CALL(*handler, mockClose(_)).WillOnce(InvokeWithoutArgs([this] {
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));

  // The handler should be deleted now
  handler->readEOF(nullptr);
}

TEST_F(BroadcastHandlerTest, OnError) {
  // Test with EOF on the handler
  EXPECT_CALL(*decoder, decode(_, _, _, _))
      .WillRepeatedly(
          Invoke([&](MockByteToMessageDecoder<std::string>::Context*,
                     IOBufQueue& q,
                     std::string& data,
                     size_t&) {
            auto buf = q.move();
            if (buf) {
              buf->coalesce();
              data = buf->moveToFbString().toStdString();
              return true;
            }
            return false;
          }));

  InSequence dummy;

  // Add a subscriber
  EXPECT_EQ(handler->subscribe(&subscriber0), 0);

  EXPECT_CALL(subscriber0, onNext("data1")).Times(1);

  // Push some data
  IOBufQueue q;
  q.append(IOBuf::copyBuffer("data1"));
  pipeline->read(q);
  q.reset();

  // Add another subscriber
  EXPECT_EQ(handler->subscribe(&subscriber1), 1);

  EXPECT_CALL(subscriber0, onNext("data2")).Times(1);
  EXPECT_CALL(subscriber1, onNext("data2")).Times(1);

  // Push more data
  q.append(IOBuf::copyBuffer("data2"));
  pipeline->read(q);
  q.reset();

  EXPECT_CALL(subscriber0, onError(_)).Times(1);
  EXPECT_CALL(subscriber1, onError(_)).Times(1);

  EXPECT_CALL(*handler, mockClose(_)).WillOnce(InvokeWithoutArgs([this] {
    pipeline.reset();
    return makeMoveWrapper(makeFuture());
  }));

  // The handler should be deleted now
  handler->readException(nullptr, make_exception_wrapper<std::exception>());
}
