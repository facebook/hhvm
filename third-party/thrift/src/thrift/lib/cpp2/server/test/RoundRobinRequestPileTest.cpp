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

#include <folly/Benchmark.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <folly/portability/GTest.h>

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

namespace {

ServerRequest getRequestObj(Cpp2RequestContext* ctx) {
  ServerRequest req(
      nullptr /* ResponseChannelRequest::UniquePtr  */,
      SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
      ctx,
      static_cast<protocol::PROTOCOL_TYPES>(0),
      nullptr, /* requestContext  */
      nullptr, /* asyncProcessor  */
      nullptr /* methodMetadata  */);
  return req;
}

Cpp2RequestContext* getContext(
    int pri,
    int bucket,
    vector<unique_ptr<THeader>>& tHeaderStorage,
    vector<unique_ptr<Cpp2RequestContext>>& contextStorage,
    std::mutex* lock = nullptr) {
  if (lock) {
    lock->lock();
  }
  tHeaderStorage.emplace_back(new THeader);
  auto headerSize = tHeaderStorage.size();
  auto headerPtr = tHeaderStorage[headerSize - 1].get();

  contextStorage.emplace_back(new Cpp2RequestContext(nullptr, headerPtr));
  auto ctxSize = contextStorage.size();
  auto ctx = contextStorage[ctxSize - 1].get();

  if (lock) {
    lock->unlock();
  }

  auto header = ctx->getHeader();
  THeader::StringToStringMap map;
  map["PRIORITY"] = folly::to<std::string>(pri);
  map["BUCKET"] = folly::to<std::string>(bucket);
  header->setReadHeaders(std::move(map));
  return ctx;
}

ServerRequest getServerRequest(
    int pri,
    int bucket,
    vector<unique_ptr<THeader>>& tHeaderStorage,
    vector<unique_ptr<Cpp2RequestContext>>& contextStorage,
    std::mutex* lock = nullptr) {
  return getRequestObj(
      getContext(pri, bucket, tHeaderStorage, contextStorage, lock));
}

std::function<std::pair<unsigned, unsigned>(const ServerRequest&)>
getScopeFunc() {
  std::function<std::pair<unsigned, unsigned>(const ServerRequest&)> scopeFunc =
      [](const ServerRequest& request) {
        auto ctx = request.requestContext();
        auto headers = ctx->getHeaders();
        auto priIter = headers.find("PRIORITY");
        unsigned prio = folly::to<unsigned>(priIter->second);
        auto bucketIter = headers.find("BUCKET");
        unsigned bucket = folly::to<unsigned>(bucketIter->second);

        return make_pair(prio, bucket);
      };

  return scopeFunc;
}

void checkResult(RoundRobinRequestPile& pile, int pri, int bucket) {
  auto req = pile.dequeue();
  auto ctx = req->requestContext();

  auto headers = ctx->getHeaders();
  auto priIter = headers.find("PRIORITY");
  unsigned prio = folly::to<unsigned>(priIter->second);
  auto bucketIter = headers.find("BUCKET");
  unsigned bucketTmp = folly::to<unsigned>(bucketIter->second);

  EXPECT_EQ(prio, pri);
  EXPECT_EQ(bucketTmp, bucket);
}

} // namespace

TEST(RoundRobinRequestPileTest, NormalCases) {
  vector<unique_ptr<THeader>> tHeaderStorage;
  vector<unique_ptr<Cpp2RequestContext>> contextStorage;

  auto getRequest = [&](int pri, int bucket) {
    return getServerRequest(pri, bucket, tHeaderStorage, contextStorage);
  };

  RoundRobinRequestPile::Options opts;
  opts.setNumPriorities(5);
  for (int i = 0; i < 5; ++i) {
    opts.setNumBucketsPerPriority(i, 10);
  }
  opts.pileSelectionFunction = getScopeFunc();
  RoundRobinRequestPile pile(opts);

  auto check = [&pile](int pri, int bucket) { checkResult(pile, pri, bucket); };

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

  auto req1 = pile.dequeue();
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

  auto req2 = pile.dequeue();
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
    ADD_FAILURE() << "Should have enqueued successfully";
  }
}

TEST(RoundRobinRequestPileTest, SingleBucket) {
  vector<unique_ptr<THeader>> tHeaderStorage;
  vector<unique_ptr<Cpp2RequestContext>> contextStorage;

  auto getRequest = [&](int pri, int bucket) {
    return getServerRequest(pri, bucket, tHeaderStorage, contextStorage);
  };

  RoundRobinRequestPile::Options opts;
  opts.setNumPriorities(1);
  opts.setNumBucketsPerPriority(0, 1);
  opts.pileSelectionFunction = getScopeFunc();
  RoundRobinRequestPile pile(opts);

  auto check = [&pile](int pri, int bucket) { checkResult(pile, pri, bucket); };

  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));
  pile.enqueue(getRequest(0, 0));

  check(0, 0);
  check(0, 0);
  check(0, 0);

  auto req1 = pile.dequeue();
  EXPECT_EQ(req1, std::nullopt);

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
    ADD_FAILURE() << "Shouldn't have enqueued successfully";
  }
}

TEST(RoundRobinRequestPileTest, requestCount) {
  THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, true);
  if (!apache::thrift::useResourcePoolsFlagsSet()) {
    GTEST_SKIP() << "Invalid resource pools mode";
  }

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

  auto client = runner.newClient<apache::thrift::Client<TestService>>();

  client->semifuture_echoInt(0);

  // This is an e2e test we need to give request
  // time to hit the server
  usleep(2000000);

  EXPECT_EQ(cc.getExecutionLimitRequests(), 0);
  EXPECT_EQ(rpSet.numQueued(), 1);

  cc.setExecutionLimitRequests(1);
}

/*
  WARNING: Benchmark running in DEBUG mode
  ============================================================================
  [...]er/test/RoundRobinRequestPileTest.cpp     relative  time/iter   iters/s
  ============================================================================
  DefaultPerf                                                 10.80s    92.56m
  RoundRobinBehavior                                          10.68s    85.60m
  WARNING: Benchmark running in DEBUG mode
*/
BENCHMARK(DefaultPerf) {
  folly::BenchmarkSuspender suspender;
  suspender.dismiss();
  // This benchmark is a simple vanilla case
  // where we have a RoundRobinRequestPile with only
  // one priority and one bucket without any limit
  vector<unique_ptr<THeader>> tHeaderStorage;
  vector<unique_ptr<Cpp2RequestContext>> contextStorage;
  std::mutex lock;

  auto getRequest = [&](int pri, int bucket) {
    return getServerRequest(pri, bucket, tHeaderStorage, contextStorage, &lock);
  };

  // single bucket, unlimited request pile, with control on
  RoundRobinRequestPile::Options opts;
  opts.setNumPriorities(1);
  opts.setNumBucketsPerPriority(0, 1);
  opts.pileSelectionFunction = getScopeFunc();
  RoundRobinRequestPile pile(opts);

  auto numThreads = std::thread::hardware_concurrency();
  unsigned numRoundEachWorker = 10'000;

  folly::CPUThreadPoolExecutor producer(numThreads);
  folly::CPUThreadPoolExecutor consumer(numThreads);

  folly::relaxed_atomic<unsigned> counter{0};

  auto producerFunc = [&]() {
    for (unsigned i = 0; i < numRoundEachWorker; ++i) {
      pile.enqueue(getRequest(0, 0));
    }
  };

  auto consumerFunc = [&]() {
    while (counter.load() != numThreads * numRoundEachWorker) {
      if (auto req = pile.dequeue()) {
        ++counter;
      } else {
        std::this_thread::yield();
      }
    }
  };

  for (unsigned i = 0; i < numThreads; ++i) {
    consumer.add(consumerFunc);
  }

  suspender.rehire();

  for (unsigned i = 0; i < numThreads; ++i) {
    producer.add(producerFunc);
  }

  producer.join();
  consumer.join();
}

/*
  WARNING: Benchmark running in DEBUG mode
  ============================================================================
  [...]er/test/RoundRobinRequestPileTest.cpp     relative  time/iter   iters/s
  ============================================================================
  DefaultPerf                                                 10.80s    92.56m
  RoundRobinBehavior                                          10.68s    85.60m
  WARNING: Benchmark running in DEBUG mode
*/
BENCHMARK(RoundRobinBehavior) {
  folly::BenchmarkSuspender suspender;
  suspender.dismiss();
  vector<unique_ptr<THeader>> tHeaderStorage;
  vector<unique_ptr<Cpp2RequestContext>> contextStorage;
  std::mutex lock;

  auto getRequest = [&](int pri, int bucket) {
    return getServerRequest(pri, bucket, tHeaderStorage, contextStorage, &lock);
  };

  unsigned numBuckets = 100;
  unsigned numRoundsPerWorker = 100;
  auto numThreads = std::thread::hardware_concurrency();

  // single bucket, unlimited request pile, with control on
  RoundRobinRequestPile::Options opts;
  opts.setNumPriorities(1);
  opts.setNumBucketsPerPriority(0, numBuckets);
  opts.pileSelectionFunction = getScopeFunc();
  RoundRobinRequestPile pile(opts);

  folly::CPUThreadPoolExecutor producer(numThreads);
  folly::CPUThreadPoolExecutor consumer(numThreads);

  folly::relaxed_atomic<unsigned> counter{0};

  auto producerFunc = [&]() {
    for (unsigned i = 0; i < numRoundsPerWorker; ++i) {
      for (unsigned j = 0; j < numBuckets; ++j) {
        pile.enqueue(getRequest(0, j));
      }
    }
  };

  auto sum = numThreads * numRoundsPerWorker * numBuckets;

  auto consumerFunc = [&]() {
    while (counter.load() != sum) {
      if (auto req = pile.dequeue()) {
        ++counter;
      } else {
        std::this_thread::yield();
      }
    }
  };

  for (unsigned i = 0; i < numThreads; ++i) {
    consumer.add(consumerFunc);
  }

  suspender.rehire();

  for (unsigned i = 0; i < numThreads; ++i) {
    producer.add(producerFunc);
  }

  producer.join();
  consumer.join();
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  folly::init(&argc, &argv);
  auto ret = RUN_ALL_TESTS();
  folly::runBenchmarks();
  return ret;
}
