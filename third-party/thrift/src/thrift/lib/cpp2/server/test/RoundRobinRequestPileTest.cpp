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

#include <vector>

#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;

TEST(RoundRobinRequestPileTest, NormalCases) {
  using Func =
      std::function<std::pair<unsigned, unsigned>(const ServerRequest&)>;

  vector<unique_ptr<THeader>> tHeaderStorage;
  vector<unique_ptr<Cpp2RequestContext>> contextStorage;

  auto getRequestObj = [](Cpp2RequestContext* ctx) {
    ServerRequest req(
        nullptr /* ResponseChannelRequest::UniquePtr  */,
        SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
        ctx,
        static_cast<protocol::PROTOCOL_TYPES>(0),
        nullptr, /* requestContext  */
        nullptr, /* asyncProcessor  */
        nullptr /* methodMetadata  */);
    return req;
  };

  auto getContext = [&](int pri, int bucket) {
    tHeaderStorage.emplace_back(new THeader);
    auto headerSize = tHeaderStorage.size();
    auto headerPtr = tHeaderStorage[headerSize - 1].get();

    contextStorage.emplace_back(new Cpp2RequestContext(nullptr, headerPtr));
    auto ctxSize = contextStorage.size();
    auto ctx = contextStorage[ctxSize - 1].get();

    auto header = ctx->getHeader();
    THeader::StringToStringMap map;
    map["PRIORITY"] = folly::to<std::string>(pri);
    map["BUCKET"] = folly::to<std::string>(bucket);
    header->setReadHeaders(std::move(map));
    return ctx;
  };

  auto getRequest = [&](int pri, int bucket) {
    return getRequestObj(getContext(pri, bucket));
  };

  Func scopeFunc = [](const ServerRequest& request) {
    auto ctx = request.requestContext();
    auto headers = ctx->getHeaders();
    auto priIter = headers.find("PRIORITY");
    unsigned prio = folly::to<unsigned>(priIter->second);
    auto bucketIter = headers.find("BUCKET");
    unsigned bucket = folly::to<unsigned>(bucketIter->second);

    return make_pair(prio, bucket);
  };

  RoundRobinRequestPile::Options opts;
  opts.setNumPriorities(5);
  for (int i = 0; i < 5; ++i) {
    opts.setNumBucketsPerPriority(i, 10);
  }
  opts.pileSelectionFunction = scopeFunc;
  RoundRobinRequestPile pile(opts);

  auto check = [&pile](int pri, int bucket) {
    auto [req, _] = pile.dequeue();
    auto ctx = req->requestContext();

    auto headers = ctx->getHeaders();
    auto priIter = headers.find("PRIORITY");
    unsigned prio = folly::to<unsigned>(priIter->second);
    auto bucketIter = headers.find("BUCKET");
    unsigned bucketTmp = folly::to<unsigned>(bucketIter->second);

    EXPECT_EQ(prio, pri);
    EXPECT_EQ(bucketTmp, bucket);
  };

  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 1));
  pile.enqueue(getRequest(0, 1));
  pile.enqueue(getRequest(0, 2));
  pile.enqueue(getRequest(0, 3));

  check(0, 0);
  check(0, 1);
  check(0, 2);
  check(0, 3);
  check(0, 0);
  check(0, 1);
  check(0, 0);

  auto [req1, _] = pile.dequeue();
  EXPECT_EQ(req1, std::nullopt);

  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(2, 0));
  pile.enqueue(getRequest(2, 0));
  pile.enqueue(getRequest(1, 0));
  pile.enqueue(getRequest(2, 1));
  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(1, 1));

  check(0, 0);
  check(0, 0);
  check(0, 0);
  check(1, 0);
  check(1, 1);
  check(2, 0);
  check(2, 1);
  check(2, 0);

  auto [req2, __] = pile.dequeue();
  EXPECT_EQ(req2, std::nullopt);

  EXPECT_EQ(pile.requestCount(), 0);

  opts.numMaxRequests = 1;
  RoundRobinRequestPile limitedPile(opts);

  auto res = limitedPile.enqueue(getRequest(0, 0));
  if (res) {
    ADD_FAILURE() << "Should have enqueued successfully";
  }

  res = limitedPile.enqueue(getRequest(0, 0));
  if (!res) {
    ADD_FAILURE() << "Shouldn't have enqueued successfully";
  }

  EXPECT_EQ(limitedPile.requestCount(), 1);
  limitedPile.dequeue();
  limitedPile.dequeue();
  EXPECT_EQ(limitedPile.requestCount(), 0);

  res = limitedPile.enqueue(getRequest(0, 0));
  if (res) {
    EXPECT_EQ(1, 2);
  } else {
    EXPECT_EQ(1, 1);
  }
}

TEST(RoundRobinRequestPileTest, SingleBucket) {
  using Func =
      std::function<std::pair<unsigned, unsigned>(const ServerRequest&)>;

  vector<unique_ptr<THeader>> tHeaderStorage;
  vector<unique_ptr<Cpp2RequestContext>> contextStorage;

  auto getRequestObj = [](Cpp2RequestContext* ctx) {
    ServerRequest req(
        nullptr /* ResponseChannelRequest::UniquePtr  */,
        SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
        ctx,
        static_cast<protocol::PROTOCOL_TYPES>(0),
        nullptr, /* requestContext  */
        nullptr, /* asyncProcessor  */
        nullptr /* methodMetadata  */);
    return req;
  };

  auto getContext = [&](int pri, int bucket) {
    tHeaderStorage.emplace_back(new THeader);
    auto headerSize = tHeaderStorage.size();
    auto headerPtr = tHeaderStorage[headerSize - 1].get();

    contextStorage.emplace_back(new Cpp2RequestContext(nullptr, headerPtr));
    auto ctxSize = contextStorage.size();
    auto ctx = contextStorage[ctxSize - 1].get();

    auto header = ctx->getHeader();
    THeader::StringToStringMap map;
    map["PRIORITY"] = folly::to<std::string>(pri);
    map["BUCKET"] = folly::to<std::string>(bucket);
    header->setReadHeaders(std::move(map));
    return ctx;
  };

  auto getRequest = [&](int pri, int bucket) {
    return getRequestObj(getContext(pri, bucket));
  };

  Func scopeFunc = [](const ServerRequest& request) {
    auto ctx = request.requestContext();
    auto headers = ctx->getHeaders();
    auto priIter = headers.find("PRIORITY");
    unsigned prio = folly::to<unsigned>(priIter->second);
    auto bucketIter = headers.find("BUCKET");
    unsigned bucket = folly::to<unsigned>(bucketIter->second);

    return make_pair(prio, bucket);
  };

  RoundRobinRequestPile::Options opts;
  opts.setNumPriorities(1);
  opts.setNumBucketsPerPriority(0, 1);
  opts.pileSelectionFunction = scopeFunc;
  RoundRobinRequestPile pile(opts);

  auto check = [&pile](int pri, int bucket) {
    auto [req, _] = pile.dequeue();
    auto ctx = req->requestContext();

    auto headers = ctx->getHeaders();
    auto priIter = headers.find("PRIORITY");
    unsigned prio = folly::to<unsigned>(priIter->second);
    auto bucketIter = headers.find("BUCKET");
    unsigned bucketTmp = folly::to<unsigned>(bucketIter->second);

    EXPECT_EQ(prio, pri);
    EXPECT_EQ(bucketTmp, bucket);
  };

  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));

  check(0, 0);
  check(0, 0);
  check(0, 0);

  auto [req1, _] = pile.dequeue();
  EXPECT_EQ(req1, std::nullopt);
}

TEST(RoundRobinRequestPileTest, requestCount) {
  THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, true);

  class BlockingCallTestService
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::SemiFuture<int32_t> semifuture_echoInt(int32_t) override {
      return folly::makeSemiFuture(1);
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<BlockingCallTestService>());

  auto& thriftServer = dynamic_cast<ThriftServer&>(runner.getThriftServer());

  // grab the resource pool
  // and set the number to 0
  auto& rpSet = thriftServer.resourcePoolSet();
  auto& rp = rpSet.resourcePool(ResourcePoolHandle::defaultAsync());
  ConcurrencyControllerInterface& cc = *rp.concurrencyController();
  cc.setExecutionLimitRequests(0);

  auto client = runner.newClient<TestServiceAsyncClient>();

  client->semifuture_echoInt(0);

  // This is an e2e test we need to give request
  // time to hit the server
  usleep(2000000);

  EXPECT_EQ(cc.getExecutionLimitRequests(), 0);
  EXPECT_EQ(rpSet.numQueued(), 1);

  cc.setExecutionLimitRequests(1);
}
