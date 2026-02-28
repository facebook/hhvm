/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/coro/HTTPHandlerChain.h>

namespace proxygen::coro::test {

class TestHTTPChainHandler : public HTTPChainHandler {
 public:
  TestHTTPChainHandler() = default;
  ~TestHTTPChainHandler() override = default;

  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override {
    co_return HTTPSourceHolder();
  }
};

TEST(HTTPHandlerChainTests, getNextHandlerEmpty) {
  TestHTTPChainHandler handler;
  EXPECT_FALSE(handler.getNextHandler());
}

TEST(HTTPHandlerChainTests, setNextHandler) {
  TestHTTPChainHandler handlerA;
  auto handlerB = std::make_shared<TestHTTPChainHandler>();

  handlerA.setNextHandler(handlerB);
  EXPECT_EQ(handlerA.getNextHandler(), handlerB);

  handlerA.setNextHandler(nullptr);
  EXPECT_FALSE(handlerA.getNextHandler());
}

TEST(HTTPHandlerChainTests, getFrontEmpty) {
  HTTPHandlerChain chain;
  EXPECT_FALSE(chain.getFront());
}

TEST(HTTPHandlerChainTests, getBackEmpty) {
  HTTPHandlerChain chain;
  EXPECT_FALSE(chain.getBack());
}

TEST(HTTPHandlerChainTests, getFrontNonEmpty) {
  HTTPHandlerChain chainFront;
  chainFront.insertFront(std::make_shared<TestHTTPChainHandler>());
  EXPECT_TRUE(chainFront.getFront());

  HTTPHandlerChain chainBack;
  chainBack.insertBack(std::make_shared<TestHTTPChainHandler>());
  EXPECT_TRUE(chainBack.getFront());
}

TEST(HTTPHandlerChainTests, getBackNonEmpty) {
  HTTPHandlerChain chainFront;
  chainFront.insertFront(std::make_shared<TestHTTPChainHandler>());
  EXPECT_TRUE(chainFront.getFront());

  HTTPHandlerChain chainBack;
  chainBack.insertBack(std::make_shared<TestHTTPChainHandler>());
  EXPECT_TRUE(chainBack.getFront());
}

TEST(HTTPHandlerChainTests, insertFrontMultiple) {
  HTTPHandlerChain chain;

  auto chainHandlerA = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerB = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerC = std::make_shared<TestHTTPChainHandler>();

  chain.insertFront(chainHandlerA);
  chain.insertFront(chainHandlerB);
  chain.insertFront(chainHandlerC);

  // Check for expected ends
  EXPECT_EQ(chain.getFront(), chainHandlerC);
  EXPECT_EQ(chain.getBack(), chainHandlerA);

  // Check for expected chain order
  EXPECT_EQ(chainHandlerC->getNextHandler(), chainHandlerB);
  EXPECT_EQ(chainHandlerB->getNextHandler(), chainHandlerA);
  EXPECT_FALSE(chainHandlerA->getNextHandler());
}

TEST(HTTPHandlerChainTests, insertBackMultiple) {
  HTTPHandlerChain chain;

  auto chainHandlerA = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerB = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerC = std::make_shared<TestHTTPChainHandler>();

  chain.insertBack(chainHandlerA);
  chain.insertBack(chainHandlerB);
  chain.insertBack(chainHandlerC);

  // Check for expected ends
  EXPECT_EQ(chain.getFront(), chainHandlerA);
  EXPECT_EQ(chain.getBack(), chainHandlerC);

  // Check for expected chain order
  EXPECT_EQ(chainHandlerA->getNextHandler(), chainHandlerB);
  EXPECT_EQ(chainHandlerB->getNextHandler(), chainHandlerC);
  EXPECT_FALSE(chainHandlerC->getNextHandler());
}

TEST(HTTPHandlerChainTests, insertMultiple) {
  HTTPHandlerChain chain;

  auto chainHandlerA = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerB = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerC = std::make_shared<TestHTTPChainHandler>();
  auto chainHandlerD = std::make_shared<TestHTTPChainHandler>();

  chain.insertFront(chainHandlerA);
  chain.insertBack(chainHandlerB);
  chain.insertFront(chainHandlerC);
  chain.insertBack(chainHandlerD);

  // Check for expected ends
  EXPECT_EQ(chain.getFront(), chainHandlerC);
  EXPECT_EQ(chain.getBack(), chainHandlerD);

  // Check for expected chain order
  EXPECT_EQ(chainHandlerC->getNextHandler(), chainHandlerA);
  EXPECT_EQ(chainHandlerA->getNextHandler(), chainHandlerB);
  EXPECT_EQ(chainHandlerB->getNextHandler(), chainHandlerD);
  EXPECT_FALSE(chainHandlerD->getNextHandler());
}

TEST(HTTPHandlerChainTests, insertFrontNull) {
  HTTPHandlerChain chain;

  auto chainHandler = std::make_shared<TestHTTPChainHandler>();
  chain.insertFront(chainHandler);
  chain.insertFront(nullptr);

  // Check for expected ends, nullptr should not have been added
  EXPECT_EQ(chain.getFront(), chainHandler);
  EXPECT_EQ(chain.getBack(), chainHandler);
}

TEST(HTTPHandlerChainTests, insertBackNull) {
  HTTPHandlerChain chain;

  auto chainHandler = std::make_shared<TestHTTPChainHandler>();
  chain.insertBack(chainHandler);
  chain.insertBack(nullptr);

  // Check for expected ends, nullptr should not have been added
  EXPECT_EQ(chain.getFront(), chainHandler);
  EXPECT_EQ(chain.getBack(), chainHandler);
}

} // namespace proxygen::coro::test
