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

#include <gtest/gtest.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Calculator.h>
#include <thrift/lib/cpp2/test/gen-cpp2/CalculatorAsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;
using namespace apache::thrift;

TEST(InternalPriorityTest, AddInternalPriorities) {
  ServerRequest reqNoPri, reqLowPri, reqMidPri, reqHighPri;
  detail::ServerRequestHelper::setInternalPriority(
      reqLowPri, folly::Executor::LO_PRI);
  detail::ServerRequestHelper::setInternalPriority(
      reqMidPri, folly::Executor::MID_PRI);
  detail::ServerRequestHelper::setInternalPriority(
      reqHighPri, folly::Executor::HI_PRI);

  // Default case
  {
    auto opts = RoundRobinRequestPile::addInternalPriorities(
        RoundRobinRequestPile::Options());
    EXPECT_EQ((std::vector<unsigned int>{1, 1}), opts.numBucketsPerPriority);
    EXPECT_EQ(1, opts.pileSelectionFunction(reqNoPri).first);
    EXPECT_EQ(1, opts.pileSelectionFunction(reqLowPri).first);
    EXPECT_EQ(0, opts.pileSelectionFunction(reqMidPri).first);
    EXPECT_EQ(0, opts.pileSelectionFunction(reqHighPri).first);
  }

  // Some priorities
  {
    auto opts = RoundRobinRequestPile::Options();
    opts.setShape({1, 2, 3});
    opts = RoundRobinRequestPile::addInternalPriorities(std::move(opts));
    EXPECT_EQ(
        (std::vector<unsigned int>{1, 1, 2, 2, 3, 3}),
        opts.numBucketsPerPriority);
    EXPECT_EQ(1, opts.pileSelectionFunction(reqNoPri).first);
    EXPECT_EQ(1, opts.pileSelectionFunction(reqLowPri).first);
    EXPECT_EQ(0, opts.pileSelectionFunction(reqMidPri).first);
    EXPECT_EQ(0, opts.pileSelectionFunction(reqHighPri).first);
  }

  // Some priorities with pileSelectionFunction
  {
    auto opts = RoundRobinRequestPile::Options();
    opts.setShape({1, 2, 3});
    opts.setPileSelectionFunction(
        [](const ServerRequest&) -> std::pair<
                                     RoundRobinRequestPile::Priority,
                                     RoundRobinRequestPile::Bucket> {
          // always choose lowest priority
          return {2, 0};
        });
    opts = RoundRobinRequestPile::addInternalPriorities(std::move(opts));
    EXPECT_EQ(
        (std::vector<unsigned int>{1, 1, 2, 2, 3, 3}),
        opts.numBucketsPerPriority);
    EXPECT_EQ(5, opts.pileSelectionFunction(reqNoPri).first);
    EXPECT_EQ(5, opts.pileSelectionFunction(reqLowPri).first);
    EXPECT_EQ(4, opts.pileSelectionFunction(reqMidPri).first);
    EXPECT_EQ(4, opts.pileSelectionFunction(reqHighPri).first);
  }
}

struct CalculatorHandler : public ServiceHandler<test::Calculator> {
  struct AdditionHandler : ServiceHandler<test::Calculator>::AdditionIf {
    explicit AdditionHandler(CalculatorHandler& handler) : handler_(handler) {}
    int32_t getPrimitive() override {
      handler_.requestsProcessed_.push_back(
          getRequestContext()->getHeaders().at("my_request_id"));
      return 0;
    }
    void noop() override {
      handler_.blockTest_.post();
      handler_.blockServer_.wait();
    }
    CalculatorHandler& handler_;
  };
  std::unique_ptr<AdditionIf> createAddition() override {
    return std::make_unique<AdditionHandler>(*this);
  }
  TileAndResponse<ServiceHandler<test::Calculator>::AdditionIf, void>
  newAddition() override {
    requestsProcessed_.push_back(
        getRequestContext()->getHeaders().at("my_request_id"));
    return TileAndResponse<ServiceHandler<test::Calculator>::AdditionIf, void>{
        std::make_unique<AdditionHandler>(*this)};
  }

  void async_eb_veryFastAddition(
      HandlerCallbackPtr<TileAndResponse<
          ServiceHandler<test::Calculator>::AdditionFastIf,
          void>> cb) override {
    cb->result({std::make_unique<AdditionFastIf>()});
  }

  folly::Baton<> blockServer_;
  folly::Baton<> blockTest_;
  std::vector<std::string> requestsProcessed_;
};

CO_TEST(InternalPriorityTest, IntegrationTest) {
  auto handler = std::make_shared<CalculatorHandler>();
  ScopedServerInterfaceThread server(handler, [](ThriftServer& thriftServer) {
    thriftServer.requireResourcePools();
    // This will create 5 priorities within a single RoundRobinRequestPile, we
    // expect this to double to 10 due to internal priority
    thriftServer.setThreadManagerType(
        ThriftServer::ThreadManagerType::PRIORITY_QUEUE);
  });
  auto client = server.newStickyClient<Client<test::Calculator>>();

  // create an interaction before blocking server CPU thread
  RpcOptions opts0;
  opts0.setWriteHeader("my_request_id", "0");
  auto existingInteraction = co_await client->co_newAddition(opts0);

  // send a request to block the server's CPU thread to force all requests to be
  // enqueued
  co_await existingInteraction.co_noop();
  handler->blockTest_.wait(); // wait until server is blocked

  RpcOptions opts1;
  opts1.setPriority(concurrency::PRIORITY::BEST_EFFORT);
  opts1.setWriteHeader("my_request_id", "1");
  auto sf1 = client->semifuture_newAddition(opts1);

  // request 2 should execute before request 1 because it is a continuation
  RpcOptions opts2;
  opts2.setPriority(concurrency::PRIORITY::BEST_EFFORT);
  opts2.setWriteHeader("my_request_id", "2");
  auto sf2 = existingInteraction.semifuture_getPrimitive(opts2);

  // request 3 should execute before request 1 and 2 even though request 2 is a
  // continuation due to higher priority
  RpcOptions opts3;
  opts3.setPriority(concurrency::PRIORITY::HIGH);
  opts3.setWriteHeader("my_request_id", "3");
  auto sf3 = client->semifuture_newAddition(opts3);

  // Send a EB-mode request to flush EventBases and ensure all prior requests
  // are enqueued
  co_await client->co_veryFastAddition();
  handler->blockServer_.post();

  co_await folly::collectAll(std::move(sf1), std::move(sf2), std::move(sf3));

  EXPECT_EQ(
      (std::vector<std::string>{"0", "3", "2", "1"}),
      handler->requestsProcessed_);
}
