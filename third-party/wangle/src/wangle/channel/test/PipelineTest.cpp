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

#include <boost/thread/barrier.hpp>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/Handler.h>
#include <wangle/channel/OutputBufferingHandler.h>
#include <wangle/channel/Pipeline.h>
#include <wangle/channel/StaticPipeline.h>
#include <wangle/channel/test/MockHandler.h>

using namespace folly;
using namespace wangle;
using namespace testing;

using IntHandler = StrictMock<MockHandlerAdapter<int, int>>;
class IntHandler2 : public StrictMock<MockHandlerAdapter<int, int>> {};

ACTION(FireRead) {
  arg0->fireRead(arg1);
}

ACTION(FireReadEOF) {
  arg0->fireReadEOF();
}

ACTION(FireReadException) {
  arg0->fireReadException(arg1);
}

ACTION(FireWrite) {
  arg0->fireWrite(arg1);
}

ACTION(FireClose) {
  arg0->fireClose();
}

// Test move only types, among other things
TEST(PipelineTest, RealHandlersCompile) {
  EventBase eb;
  auto socket = folly::to_shared_ptr(AsyncSocket::newSocket(&eb));
  // static
  {
    auto pipeline = StaticPipeline<
        IOBufQueue&,
        std::unique_ptr<IOBuf>,
        AsyncSocketHandler,
        OutputBufferingHandler>::
        create(AsyncSocketHandler(socket), OutputBufferingHandler());
    EXPECT_TRUE(pipeline->getHandler<AsyncSocketHandler>());
    EXPECT_TRUE(pipeline->getHandler<OutputBufferingHandler>());
  }
  // dynamic
  {
    auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
    (*pipeline)
        .addBack(AsyncSocketHandler(socket))
        .addBack(OutputBufferingHandler())
        .finalize();
    EXPECT_TRUE(pipeline->getHandler<AsyncSocketHandler>());
    EXPECT_TRUE(pipeline->getHandler<OutputBufferingHandler>());
  }
}

// Test that handlers correctly fire the next handler when directed
TEST(PipelineTest, FireActions) {
  IntHandler handler1;
  IntHandler2 handler2;

  {
    InSequence sequence;
    EXPECT_CALL(handler2, attachPipeline(_));
    EXPECT_CALL(handler1, attachPipeline(_));
  }

  auto pipeline = StaticPipeline<int, int, IntHandler, IntHandler2>::create(
      &handler1, &handler2);

  EXPECT_CALL(handler1, read_(_, _)).WillOnce(FireRead());
  EXPECT_CALL(handler2, read_(_, _)).Times(1);
  pipeline->read(1);

  EXPECT_CALL(handler1, readEOF(_)).WillOnce(FireReadEOF());
  EXPECT_CALL(handler2, readEOF(_)).Times(1);
  pipeline->readEOF();

  EXPECT_CALL(handler1, readException(_, _)).WillOnce(FireReadException());
  EXPECT_CALL(handler2, readException(_, _)).Times(1);
  pipeline->readException(make_exception_wrapper<std::runtime_error>("blah"));

  EXPECT_CALL(handler2, write_(_, _)).WillOnce(FireWrite());
  EXPECT_CALL(handler1, write_(_, _)).Times(1);
  EXPECT_NO_THROW(pipeline->write(1).value());

  EXPECT_CALL(handler2, close_(_)).WillOnce(FireClose());
  EXPECT_CALL(handler1, close_(_)).Times(1);
  EXPECT_NO_THROW(pipeline->close().value());

  {
    InSequence sequence;
    EXPECT_CALL(handler1, detachPipeline(_));
    EXPECT_CALL(handler2, detachPipeline(_));
  }
}

// Test that nothing bad happens when actions reach the end of the pipeline
// (a warning will be logged, however)
TEST(PipelineTest, ReachEndOfPipeline) {
  IntHandler handler;
  EXPECT_CALL(handler, attachPipeline(_));
  auto pipeline = StaticPipeline<int, int, IntHandler>::create(&handler);

  EXPECT_CALL(handler, read_(_, _)).WillOnce(FireRead());
  pipeline->read(1);

  EXPECT_CALL(handler, readEOF(_)).WillOnce(FireReadEOF());
  pipeline->readEOF();

  EXPECT_CALL(handler, readException(_, _)).WillOnce(FireReadException());
  pipeline->readException(make_exception_wrapper<std::runtime_error>("blah"));

  EXPECT_CALL(handler, write_(_, _)).WillOnce(FireWrite());
  EXPECT_NO_THROW(pipeline->write(1).value());

  EXPECT_CALL(handler, close_(_)).WillOnce(FireClose());
  EXPECT_NO_THROW(pipeline->close().value());

  EXPECT_CALL(handler, detachPipeline(_));
}

// Test having the last read handler turn around and write
TEST(PipelineTest, TurnAround) {
  IntHandler handler1;
  IntHandler2 handler2;

  {
    InSequence sequence;
    EXPECT_CALL(handler2, attachPipeline(_));
    EXPECT_CALL(handler1, attachPipeline(_));
  }

  auto pipeline = StaticPipeline<int, int, IntHandler, IntHandler2>::create(
      &handler1, &handler2);

  EXPECT_CALL(handler1, read_(_, _)).WillOnce(FireRead());
  EXPECT_CALL(handler2, read_(_, _)).WillOnce(FireWrite());
  EXPECT_CALL(handler1, write_(_, _)).Times(1);
  pipeline->read(1);

  {
    InSequence sequence;
    EXPECT_CALL(handler1, detachPipeline(_));
    EXPECT_CALL(handler2, detachPipeline(_));
  }
}

TEST(PipelineTest, DynamicFireActions) {
  IntHandler handler1, handler2, handler3;
  EXPECT_CALL(handler2, attachPipeline(_));
  auto pipeline = StaticPipeline<int, int, IntHandler>::create(&handler2);

  {
    InSequence sequence;
    EXPECT_CALL(handler3, attachPipeline(_));
    EXPECT_CALL(handler1, attachPipeline(_));
  }

  (*pipeline).addFront(&handler1).addBack(&handler3).finalize();

  EXPECT_TRUE(pipeline->getHandler<IntHandler>(0));
  EXPECT_TRUE(pipeline->getHandler<IntHandler>(1));
  EXPECT_TRUE(pipeline->getHandler<IntHandler>(2));

  EXPECT_CALL(handler1, read_(_, _)).WillOnce(FireRead());
  EXPECT_CALL(handler2, read_(_, _)).WillOnce(FireRead());
  EXPECT_CALL(handler3, read_(_, _)).Times(1);
  pipeline->read(1);

  EXPECT_CALL(handler3, write_(_, _)).WillOnce(FireWrite());
  EXPECT_CALL(handler2, write_(_, _)).WillOnce(FireWrite());
  EXPECT_CALL(handler1, write_(_, _)).Times(1);
  EXPECT_NO_THROW(pipeline->write(1).value());

  {
    InSequence sequence;
    EXPECT_CALL(handler1, detachPipeline(_));
    EXPECT_CALL(handler2, detachPipeline(_));
    EXPECT_CALL(handler3, detachPipeline(_));
  }
}

TEST(PipelineTest, DynamicAttachDetachOrder) {
  IntHandler handler1, handler2;
  auto pipeline = Pipeline<int, int>::create();
  {
    InSequence sequence;
    EXPECT_CALL(handler2, attachPipeline(_));
    EXPECT_CALL(handler1, attachPipeline(_));
  }
  (*pipeline).addBack(&handler1).addBack(&handler2).finalize();
  {
    InSequence sequence;
    EXPECT_CALL(handler1, detachPipeline(_));
    EXPECT_CALL(handler2, detachPipeline(_));
  }
}

TEST(PipelineTest, GetContext) {
  IntHandler handler;
  EXPECT_CALL(handler, attachPipeline(_));
  auto pipeline = StaticPipeline<int, int, IntHandler>::create(&handler);
  EXPECT_TRUE(handler.getContext());
  EXPECT_CALL(handler, detachPipeline(_));
}

TEST(PipelineTest, HandlerInMultiplePipelines) {
  IntHandler handler;
  EXPECT_CALL(handler, attachPipeline(_)).Times(2);
  auto pipeline1 = StaticPipeline<int, int, IntHandler>::create(&handler);
  auto pipeline2 = StaticPipeline<int, int, IntHandler>::create(&handler);
  EXPECT_FALSE(handler.getContext());
  EXPECT_CALL(handler, detachPipeline(_)).Times(2);
}

TEST(PipelineTest, HandlerInPipelineTwice) {
  auto handler = std::make_shared<IntHandler>();
  EXPECT_CALL(*handler, attachPipeline(_)).Times(2);
  auto pipeline = Pipeline<int, int>::create();
  pipeline->addBack(handler);
  pipeline->addBack(handler);
  pipeline->finalize();
  EXPECT_FALSE(handler->getContext());
  EXPECT_CALL(*handler, detachPipeline(_)).Times(2);
}

TEST(PipelineTest, NoDetachOnOwner) {
  IntHandler handler;
  EXPECT_CALL(handler, attachPipeline(_));
  auto pipeline = StaticPipeline<int, int, IntHandler>::create(&handler);
  pipeline->setOwner(&handler);
}

template <class Rin, class Rout = Rin, class Win = Rout, class Wout = Rin>
class ConcreteHandler : public Handler<Rin, Rout, Win, Wout> {
  using Context = typename Handler<Rin, Rout, Win, Wout>::Context;

 public:
  void read(Context*, Rin /* msg */) override {}
  Future<Unit> write(Context*, Win /* msg */) override {
    return makeFuture();
  }
};

using StringHandler = HandlerAdapter<std::string, std::string>;
using IntToStringHandler = ConcreteHandler<int, std::string>;
using StringToIntHandler = ConcreteHandler<std::string, int>;

TEST(Pipeline, MissingInboundOrOutbound) {
  auto pipeline = Pipeline<int, int>::create();
  (*pipeline).addBack(HandlerAdapter<std::string, std::string>{}).finalize();
  EXPECT_THROW(pipeline->read(0), std::invalid_argument);
  EXPECT_THROW(pipeline->readEOF(), std::invalid_argument);
  EXPECT_THROW(
      pipeline->readException(exception_wrapper(std::runtime_error("blah"))),
      std::invalid_argument);
  EXPECT_THROW(pipeline->write(0), std::invalid_argument);
  EXPECT_THROW(pipeline->close(), std::invalid_argument);
}

TEST(Pipeline, DynamicConstruction) {
  {
    auto pipeline = Pipeline<std::string, std::string>::create();
    pipeline->addBack(StringHandler());
    pipeline->addBack(StringHandler());

    // Exercise both addFront and addBack. Final pipeline is
    // StI <-> ItS <-> StS <-> StS <-> StI <-> ItS
    EXPECT_NO_THROW((*pipeline)
                        .addFront(IntToStringHandler{})
                        .addFront(StringToIntHandler{})
                        .addBack(StringToIntHandler{})
                        .addBack(IntToStringHandler{})
                        .finalize());
  }
}

TEST(Pipeline, RemovePointer) {
  IntHandler handler1, handler2;
  EXPECT_CALL(handler1, attachPipeline(_));
  EXPECT_CALL(handler2, attachPipeline(_));
  auto pipeline = Pipeline<int, int>::create();
  (*pipeline).addBack(&handler1).addBack(&handler2).finalize();

  EXPECT_CALL(handler1, detachPipeline(_));
  (*pipeline).remove(&handler1).finalize();

  EXPECT_CALL(handler2, read_(_, _));
  pipeline->read(1);

  EXPECT_CALL(handler2, detachPipeline(_));
}

TEST(Pipeline, RemoveFront) {
  IntHandler handler1, handler2;
  EXPECT_CALL(handler1, attachPipeline(_));
  EXPECT_CALL(handler2, attachPipeline(_));
  auto pipeline = Pipeline<int, int>::create();
  (*pipeline).addBack(&handler1).addBack(&handler2).finalize();

  EXPECT_CALL(handler1, detachPipeline(_));
  (*pipeline).removeFront().finalize();

  EXPECT_CALL(handler2, read_(_, _));
  pipeline->read(1);

  EXPECT_CALL(handler2, detachPipeline(_));
}

TEST(Pipeline, RemoveBack) {
  IntHandler handler1, handler2;
  EXPECT_CALL(handler1, attachPipeline(_));
  EXPECT_CALL(handler2, attachPipeline(_));
  auto pipeline = Pipeline<int, int>::create();
  (*pipeline).addBack(&handler1).addBack(&handler2).finalize();

  EXPECT_CALL(handler2, detachPipeline(_));
  (*pipeline).removeBack().finalize();

  EXPECT_CALL(handler1, read_(_, _));
  pipeline->read(1);

  EXPECT_CALL(handler1, detachPipeline(_));
}

TEST(Pipeline, RemoveType) {
  IntHandler handler1;
  IntHandler2 handler2;
  EXPECT_CALL(handler1, attachPipeline(_));
  EXPECT_CALL(handler2, attachPipeline(_));
  auto pipeline = Pipeline<int, int>::create();
  (*pipeline).addBack(&handler1).addBack(&handler2).finalize();

  EXPECT_CALL(handler1, detachPipeline(_));
  (*pipeline).remove<IntHandler>().finalize();

  EXPECT_CALL(handler2, read_(_, _));
  pipeline->read(1);

  EXPECT_CALL(handler2, detachPipeline(_));
}

// Pipeline lifetime used to be managed by DelayedDestruction, which is not
// thread safe. This test would fail. Mandatory shared_ptr ownership fixes the
// issue.
TEST(Pipeline, Concurrent) {
  NiceMock<MockHandlerAdapter<int, int>> handler1, handler2;
  auto pipeline = Pipeline<int, int>::create();
  (*pipeline).addBack(&handler1).addBack(&handler2).finalize();
  boost::barrier b{2};
  auto spam = [&] {
    for (int i = 0; i < 100000; i++) {
      b.wait();
      pipeline->read(i);
    }
  };
  std::thread t(spam);
  spam();
  t.join();
}

TEST(PipelineTest, NumHandler) {
  NiceMock<MockHandlerAdapter<int, int>> handler1, handler2;
  auto pipeline = Pipeline<int, int>::create();
  EXPECT_EQ(0, pipeline->numHandlers());

  pipeline->addBack(&handler1);
  EXPECT_EQ(1, pipeline->numHandlers());

  pipeline->addBack(&handler2);
  EXPECT_EQ(2, pipeline->numHandlers());

  pipeline->finalize();
  EXPECT_EQ(2, pipeline->numHandlers());

  pipeline->remove(&handler1);
  EXPECT_EQ(1, pipeline->numHandlers());

  pipeline->remove(&handler2);
  EXPECT_EQ(0, pipeline->numHandlers());
}

TEST(PipelineTest, HandlerReuse) {
  NiceMock<MockHandlerAdapter<int, int>> handler1, handler2, handler3;
  auto pipeline1 = Pipeline<int, int>::create();

  // pipeline1 contains the first two handlers
  (*pipeline1).addBack(&handler1).addBack(&handler2).finalize();
  pipeline1->read(42);
  EXPECT_NE(nullptr, handler2.getContext());

  // Close and detach the back handler (#2)
  pipeline1->close();
  pipeline1->removeBack();
  ASSERT_EQ(nullptr, handler2.getContext());

  auto pipeline2 = Pipeline<int, int>::create();
  (*pipeline2).addBack(&handler2).addBack(&handler3).finalize();
  pipeline2->read(24);
  EXPECT_NE(nullptr, handler2.getContext());

  // detach both
  pipeline2->remove(&handler2);
  pipeline2->remove(&handler3);
  ASSERT_EQ(nullptr, handler2.getContext());
  ASSERT_EQ(nullptr, handler3.getContext());

  auto pipeline3 = Pipeline<int, int>::create();
  (*pipeline3).addBack(&handler2).addBack(&handler3).finalize();
  pipeline3->read(1);
  EXPECT_NE(nullptr, handler2.getContext());
}
