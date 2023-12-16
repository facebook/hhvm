/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <folly/fibers/Baton.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/ExecutorObserver.h"
#include "mcrouter/McReqUtil.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeServer.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

#include "mcrouter/stats.h"

using facebook::memcache::McGetReply;
using facebook::memcache::McGetRequest;
using facebook::memcache::McStatsReply;
using facebook::memcache::McStatsRequest;
using facebook::memcache::MemcacheRouterInfo;
using facebook::memcache::mcrouter::CarbonRouterClient;
using facebook::memcache::mcrouter::CarbonRouterInstance;
using facebook::memcache::mcrouter::defaultTestOptions;

/**
 * This test provides an example of how to use the CarbonRouterClient API.
 *
 * The recommended usage pattern is:
 *   1. In order to use mcrouter, the client needs a CarbonRouterInstance,
 *      obtained through one of the static factory methods. In most long-lived
 *      programs, CarbonRouterInstance::init() is the way to go.
 *   2. Create a CarbonRouterClient object associated to the
 *      CarbonRouterInstance via CarbonRouterInstance::createClient() or
 *      CarbonRouterInstance::createSameThreadClient().
 *   3. Send requests through mcrouter via CarbonRouterClient::send(). (With
 *      some caveats; read the comments below.)
 */

TEST(CarbonRouterClient, basicUsageSameThreadClient) {
  // Don't log stats in tests
  auto opts = defaultTestOptions();
  opts.num_proxies = 4;
  // We only want to demonstrate client usage in this test, so reply to each
  // request with the corresponding default reply.
  opts.config_str = R"({ "route": "NullRoute" })";

  // Every program that uses mcrouter must have at least one (usually exactly
  // one) CarbonRouterInstance, which manages (re)configuration, starting up
  // request-handling proxies, stats logging, and more.
  // Using createSameThreadClient() makes most sense in situations where the
  // user controls their own EventBases, as below.
  std::shared_ptr<folly::IOThreadPoolExecutor> ioThreadPool =
      std::make_shared<folly::IOThreadPoolExecutor>(
          opts.num_proxies, opts.num_proxies);
  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "basicUsageSameThreadClient", opts, ioThreadPool);

  // When using createSameThreadClient(), users must ensure that client->send()
  // is only ever called on the same thread as the associated Proxy.
  // Note that client->send() hands the request off to the Proxy, which
  // processes/sends the request asynchronously, i.e., after client->send()
  // returns.
  // Alternatively, users may opt to obtain a client via router->createClient(),
  // in which case client->send() is thread-safe.
  //
  // In any case, we go must ensure that client will remain alive throughout the
  // entire request/reply transaction in client->send() below.
  // (router->shutdown() will complete before client is destroyed.)
  auto client =
      router->createSameThreadClient(0 /* max_outstanding_requests */);

  // Explicitly control which proxy should handle requests from this client.
  // Currently, this is necessary when using createSameThreadClient() with more
  // than one thread.
  // extract event bases
  auto evbs = facebook::memcache::mcrouter::extractEvbs(*ioThreadPool);

  auto& eventBase = *evbs.front();
  client->setProxyIndex(0);

  bool replyReceived = false;
  eventBase.runInEventBaseThread([client = client.get(), &replyReceived]() {
    // We must ensure that req will remain alive all the way through the reply
    // callback given to client->send(). This demonstrates one way of ensuring
    // this.
    auto req = std::make_unique<McGetRequest>("key");
    auto reqRawPtr = req.get();
    client->send(
        *reqRawPtr,
        [req = std::move(req), &replyReceived](
            const McGetRequest&, McGetReply&& reply) {
          EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
          replyReceived = true;
        });
  });

  // Wait for proxy threads to complete outstanding requests and exit
  // gracefully. This ensures graceful destruction of the static
  // CarbonRouterInstance instance.
  router->shutdown();
  ioThreadPool.reset();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, basicUsageRemoteThreadClient) {
  // This test is a lot like the previous one, except this test demonstrates
  // the use of a client that can safely send a request through a Proxy
  // on another thread.  Much of the code is the exact same as before.
  auto opts = defaultTestOptions();
  opts.config_str = R"({ "route": "NullRoute" })";

  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "basicUsageRemoteThreadClient", opts);

  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(
      0 /* max_outstanding_requests */,
      false /* max_outstanding_requests_error */);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  const McGetRequest req("key");
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req, [&baton, &replyReceived](const McGetRequest&, McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, basicUsageRemoteThreadClientThreadPool) {
  // This test is a lot like the previous one, except this test demonstrates
  // the use of a client that can safely send a request through a Proxy
  // on another thread.  Much of the code is the exact same as before.
  auto opts = defaultTestOptions();
  opts.config_str = R"({ "route": "NullRoute" })";

  auto ioThreadPool =
      std::make_shared<folly::IOThreadPoolExecutor>(opts.num_proxies);

  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "basicUsageRemoteThreadClient", opts, ioThreadPool);

  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(
      0 /* max_outstanding_requests */,
      false /* max_outstanding_requests_error */);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  const McGetRequest req("key");
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req, [&baton, &replyReceived](const McGetRequest&, McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, basicUsageRemoteThreadClientThreadAffinity) {
  // This test is a lot like the previous one, except this test demonstrates
  // the use of a client that can safely send a request through a Proxy
  // on another thread with thread affinity.
  auto opts = defaultTestOptions();
  opts.config_str = R"({ "route": "NullRoute" })";
  opts.thread_affinity = true;
  opts.num_proxies = 3;

  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "basicUsageRemoteThreadClientThreadAffinity", opts);

  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(
      0 /* max_outstanding_requests */,
      false /* max_outstanding_requests_error */);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  const McGetRequest req("key");
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req, [&baton, &replyReceived](const McGetRequest&, McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, basicUsageRemoteThreadClientThreadAffinityMulti) {
  // This test is a lot like the previous one, but it shows how to send a batch
  // of requests at once.
  auto opts = defaultTestOptions();
  opts.config_str = R"(
  {
    "route": {
      "type": "PoolRoute",
      "pool": {
        "name": "A",
        "servers": [
          "10.0.0.1:11111",
          "10.0.0.2:11111",
          "10.0.0.3:11111",
          "10.0.0.4:11111",
          "10.0.0.5:11111",
          "10.0.0.6:11111"
        ]
      }
    }
  })";
  opts.thread_affinity = true;
  opts.num_proxies = 3;
  opts.server_timeout_ms = 1;
  opts.miss_on_get_errors = true;

  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "basicUsageRemoteThreadClientThreadAffinityMulti", opts);

  auto client = router->createClient(
      0 /* max_outstanding_requests */,
      false /* max_outstanding_requests_error */);

  std::vector<McGetRequest> requests{
      McGetRequest("key1"),
      McGetRequest("key2"),
      McGetRequest("key3"),
      McGetRequest("key4"),
      McGetRequest("key5"),
      McGetRequest("key6"),
      McGetRequest("key7"),
      McGetRequest("key8"),
      McGetRequest("key9"),
      McGetRequest("key10"),
      McGetRequest("key11"),
      McGetRequest("key12"),
      McGetRequest("key13"),
      McGetRequest("key14"),
      McGetRequest("key15")};
  folly::fibers::Baton baton;
  std::atomic<size_t> replyCount = 0;

  client->send(
      requests.begin(),
      requests.end(),
      [&baton, &replyCount, &requests](
          const McGetRequest&, McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
        if (++replyCount == requests.size()) {
          baton.post();
        }
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  EXPECT_EQ(requests.size(), replyCount.load());

  // Make sure that all proxies were notified, and that each proxy was notified
  // just once.
  const auto& proxies = router->getProxies();
  for (size_t i = 0; i < proxies.size(); ++i) {
    EXPECT_EQ(
        1,
        proxies[i]->stats().getValue(
            facebook::memcache::mcrouter::client_queue_notifications_stat));
  }
  EXPECT_EQ(3, proxies.size());

  router->shutdown();
}

TEST(CarbonRouterClient, remoteThreadStatsRequestUsage) {
  // This test is a lot like the previous one, except this test demonstrates
  // how to collect libmcrouter stats using the McStatsRequest.
  auto opts = defaultTestOptions();
  opts.config_str = R"({ "route": "NullRoute" })";

  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "remoteThreadStatsRequestUsage", opts);

  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(0 /* max_outstanding_requests */);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  const McStatsRequest req("all");
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req,
      [&baton, &replyReceived](const McStatsRequest&, McStatsReply&& reply) {
        EXPECT_GT(reply.stats_ref()->size(), 1);
        EXPECT_EQ(carbon::Result::OK, *reply.result_ref());
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, externalStatsTest) {
  // This test checks that we can properly register and invoke
  // the external stats callback from a stats request
  auto opts = defaultTestOptions();
  opts.config_str = R"({ "route": "NullRoute" })";

  auto router = CarbonRouterInstance<MemcacheRouterInfo>::init(
      "externalStatsRequest", opts);

  /* Register static test stats */
  bool statsCbInvoked = false;
  const uint64_t exampleValue = 500;
  const auto staticTestStatsStr = "static_test_stats";
  router->externalStatsHandler().registerExternalStats(
      staticTestStatsStr, [&statsCbInvoked, &exampleValue](auto& statsData) {
        statsData.prefix_acl_active_entries_filter_hit = exampleValue;
        statsCbInvoked = true;
      });

  /* Create a client */
  auto client = router->createClient(0 /* max_outstanding_requests */);

  /* Test `getStats()` works on the router */
  {
    const auto statsMap = router->externalStatsHandler().getStats();
    EXPECT_EQ(
        statsMap.at("prefix_acl_active_entries_filter_hit"), exampleValue);
  }
  /* Test `dumpStats()` works on the router */
  {
    const folly::dynamic statsJson = router->externalStatsHandler().dumpStats();
    EXPECT_STREQ(
        statsJson.at("prefix_acl_active_entries_filter_hit").asString().c_str(),
        folly::to<std::string>(exampleValue).c_str());
  }
  /* Test `dumpStats()` works on the router with filterZero = true */
  {
    const folly::dynamic statsJson =
        router->externalStatsHandler().dumpStats(true /* filterZeroes */);
    EXPECT_STREQ(
        statsJson.at("prefix_acl_active_entries_filter_hit").asString().c_str(),
        folly::to<std::string>(exampleValue).c_str());
    EXPECT_EQ(statsJson.size(), 1);
  }

  // issue a external stats request
  const McStatsRequest req("external");
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req,
      [&baton, &replyReceived](const McStatsRequest&, McStatsReply&& reply) {
        /* Expect at least one stat */
        EXPECT_GT(reply.stats_ref()->size(), 0);
        EXPECT_EQ(carbon::Result::OK, *reply.result_ref());
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();

  EXPECT_TRUE(statsCbInvoked);
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, requestExpiryTest) {
  // This test sends a request with deadline time set, and then waits for longer
  // than deadline time before sending the request, essentially expecting the
  // request to exceed the deadline time. Validates by checking that the
  // request indeed has exceeded the deadline.
  auto opts = defaultTestOptions();
  opts.config_str = R"(
  {
    "route": {
      "type": "PoolRoute",
      "pool": {
        "name": "A",
        "servers": [
          "127.0.0.1:11111",
          "127.0.0.1:11112",
          "127.0.0.1:11113",
        ]
      }
    }
  })";

  auto router =
      CarbonRouterInstance<hellogoodbye::HelloGoodbyeRouterInfo>::init(
          "remoteThreadStatsRequestUsage", opts);

  opts.thread_affinity = true;
  opts.num_proxies = 3;
  opts.server_timeout_ms = 1;
  opts.miss_on_get_errors = true;
  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(0 /* max_outstanding_requests */, false);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  hellogoodbye::HelloRequest req;
  req.key_ref() = "test1";
  setRequestDeadline(req, 10);
  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req,
      [&baton, &replyReceived](
          const hellogoodbye::HelloRequest&, hellogoodbye::HelloReply&& reply) {
        // for now, REMOTE_ERROR is returned in place of DEADLINE_EXCEEDED
        EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());
        EXPECT_NE(
            reply.message_ref()->find("deadline exceeded"), std::string::npos);
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

TEST(CarbonRouterClient, requestExpiryTestNoExpiry) {
  // This test sends a request without deadline time set and validate that
  // the DEADLINE_EXCEEDED error is not received.
  auto opts = defaultTestOptions();
  opts.config_str = R"(
  {
    "route": {
      "type": "PoolRoute",
      "pool": {
        "name": "A",
        "servers": [
          "127.0.0.1:11111",
          "127.0.0.1:11112",
          "127.0.0.1:11113",
        ]
      }
    }
  })";

  auto router =
      CarbonRouterInstance<hellogoodbye::HelloGoodbyeRouterInfo>::init(
          "remoteThreadStatsRequestUsage", opts);

  opts.thread_affinity = true;
  opts.num_proxies = 3;
  opts.server_timeout_ms = 1;
  opts.miss_on_get_errors = true;
  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(0 /* max_outstanding_requests */, false);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  hellogoodbye::HelloRequest req;
  req.key_ref() = "test1";
  setRequestDeadline(req, 10);
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req,
      [&baton, &replyReceived](
          const hellogoodbye::HelloRequest&, hellogoodbye::HelloReply&& reply) {
        // for now, REMOTE_ERROR is returned in place of DEADLINE_EXCEEDED
        EXPECT_NE(carbon::Result::REMOTE_ERROR, *reply.result_ref());
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  router->shutdown();
  EXPECT_TRUE(replyReceived);
}

class TestOnRequest {
 public:
  explicit TestOnRequest(folly::EventBase& evb) {
    fiberManager_ = &folly::fibers::getFiberManager(evb);
  }

  void onRequest(
      facebook::memcache::McServerRequestContext&& context,
      hellogoodbye::HelloRequest&& request) {
    fiberManager_->addTaskRemote(
        [ctx = std::move(context), req = std::move(request)]() mutable {
          facebook::memcache::McServerRequestContext::reply(
              std::move(ctx), hellogoodbye::HelloReply());
        });
  }

 private:
  folly::fibers::FiberManager* fiberManager_;
};

class TestServer {
 public:
  explicit TestServer(const size_t threadCount, uint16_t p) : port_(p) {
    facebook::memcache::AsyncMcServer::Options opts;
    opts.numThreads = threadCount;
    opts.worker.sendTimeout = std::chrono::milliseconds(10);
    // opts.existingSocketFd = socketFd_;
    opts.ports.push_back(port_);
    server_ = std::make_unique<facebook::memcache::AsyncMcServer>(opts);
  }

  ~TestServer() {
    // close(socketFd_);
  }

  uint16_t getPort() const {
    return port_;
  }

  void start() {
    server_->spawn([](size_t /* threadId */,
                      folly::EventBase& evb,
                      facebook::memcache::AsyncMcServerWorker& worker) {
      worker.setOnRequest(
          hellogoodbye::HelloGoodbyeRequestHandler<TestOnRequest>(evb));

      while (worker.isAlive() || worker.writesPending()) {
        evb.loopOnce();
      }
    });
  }

  void shutdown() {
    server_->shutdown();
    server_->join();
  }

 private:
  std::unique_ptr<facebook::memcache::AsyncMcServer> server_;
  // const int socketFd_;
  const uint16_t port_;
};
TEST(CarbonRouterClient, requestExpiryTestWithLatencyInjectionRoute) {
  // This test sends a request with deadline time set, and then waits for longer
  // than deadline time before sending the request, essentially expecting the
  // request to exceed deadline time. Validates by checking that the request
  // indeed has exceeded deadline time.
  auto opts = defaultTestOptions();
  opts.config_str = R"(
  {
    "pools": {
      "A": {
        "servers": [ "127.0.0.1:11610" ],
        "protocol": "caret"
      },
      "B": {
        "servers": [ "127.0.0.1:11611" ],
        "protocol": "caret"
      },
      "C": {
        "servers": [ "127.0.0.1:11612" ],
        "protocol": "caret"
      },
      "D": {
        "servers": [ "127.0.0.1:11613" ],
        "protocol": "caret"
      }
    },
    "route": {
      "type": "FailoverRoute",
      "children": [
                  {
                  "type": "LatencyInjectionRoute",
                  "child": "PoolRoute|A",
                  "before_latency_ms": 20
                  },
                  "PoolRoute|B",
                  "PoolRoute|C",
                  "PoolRoute|D"
      ],
      "failover_policy": {
        "type": "DeterministicOrderPolicy",
        "hash": {
          "salt": "78966653"
        },
        "max_tries": 4,
        "max_error_tries": 3
      }
    }
  })";

  TestServer server(4, 11610);
  server.start();

  auto router =
      CarbonRouterInstance<hellogoodbye::HelloGoodbyeRouterInfo>::init(
          "remoteThreadStatsRequestUsage", opts);

  opts.thread_affinity = true;
  opts.num_proxies = 3;
  opts.server_timeout_ms = 1;
  opts.miss_on_get_errors = true;
  // Create client that can safely send requests through a Proxy on another
  // thread
  auto client = router->createClient(0 /* max_outstanding_requests */, false);

  // Note, as in the previous test, that req is kept alive through the end of
  // the callback provided to client->send() below.
  // Also note that we are careful not to modify req while the proxy (in this
  // case, on another thread) may be processing it.
  hellogoodbye::HelloRequest req;
  req.key_ref() = "test1";
  setRequestDeadline(req, 10);
  bool replyReceived = false;
  folly::fibers::Baton baton;

  client->send(
      req,
      [&baton, &replyReceived](
          const hellogoodbye::HelloRequest&, hellogoodbye::HelloReply&& reply) {
        // for now, REMOTE_ERROR is returned in place of DEADLINE_EXCEEDED
        EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());
        EXPECT_NE(
            reply.message_ref()->find("deadline exceeded"), std::string::npos);
        replyReceived = true;
        baton.post();
      });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
  const auto& proxies = router->getProxies();
  uint32_t num_errors = 0;
  uint32_t num_deadline_exceeded_errors = 0;
  for (size_t i = 0; i < proxies.size(); ++i) {
    num_errors += proxies[i]->stats().getValue(
        facebook::memcache::mcrouter::failover_policy_result_error_stat);
    num_deadline_exceeded_errors += proxies[i]->stats().getValue(
        facebook::memcache::mcrouter::result_deadline_exceeded_error_all_stat);
  }
  // DEADLINE_EXCEEDED error is also failover eligible because of
  // time-sync issues so, make sure we actually failed over 3 times.
  EXPECT_EQ(num_errors, 3);
  // Enable this check after switching back to DEADLINE_EXCEEDED error
  // EXPECT_EQ(num_deadline_exceeded_errors, 3);

  router->shutdown();
  server.shutdown();
  EXPECT_TRUE(replyReceived);
}
