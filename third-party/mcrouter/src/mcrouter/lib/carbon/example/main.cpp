/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>

#include <folly/CppAttributes.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/synchronization/Baton.h>
#include <glog/logging.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbye.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/carbon/example/gen/gen-cpp2/HelloGoodbye.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McServerRequestContext.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;
using namespace hellogoodbye;

namespace {

constexpr uint16_t kPort = 11303;
constexpr uint16_t kPort2 = 11305;

struct HelloGoodbyeOnRequest {
  void onRequest(McServerRequestContext&& ctx, HelloRequest&& request) {
    LOG(INFO) << "Hello! Server " << reinterpret_cast<uintptr_t>(this)
              << " got key " << request.key_ref()->fullKey().str();
    McServerRequestContext::reply(
        std::move(ctx), HelloReply(carbon::Result::OK));
  }

  void onRequest(McServerRequestContext&& ctx, GoodbyeRequest&& request) {
    LOG(INFO) << "Good bye! Server " << reinterpret_cast<uintptr_t>(this)
              << " got key " << request.key_ref()->fullKey().str();
    McServerRequestContext::reply(
        std::move(ctx), GoodbyeReply(carbon::Result::OK));
  }
};

class ThriftHandler : virtual public hellogoodbye::thrift::HelloGoodbyeSvIf {
 public:
  ThriftHandler() = default;
  virtual ~ThriftHandler() = default;

  void async_eb_hello(
      std::unique_ptr<apache::thrift::HandlerCallback<hellogoodbye::HelloReply>>
          callback,
      const hellogoodbye::HelloRequest& request) override {
    LOG(INFO) << "Hello! Thrift server " << reinterpret_cast<uintptr_t>(this)
              << " got key " << request.key_ref()->fullKey().str();
    auto ctx = callback->getConnectionContext();
    if (ctx) {
      auto headers = ctx->getHeaders();
      auto it = headers.find("shardId");
      if (it != headers.end()) {
        LOG(INFO) << "Got shardId " << it->second << " from thrift header.";
      }
      it = headers.find("message");
      if (it != headers.end()) {
        LOG(INFO) << "Got message " << it->second << " from thrift header.";
      }
      it = headers.find("priority");
      if (it != headers.end()) {
        LOG(INFO) << "Got priority " << it->second << " from thrift header.";
      }
      it = headers.find("crypto_auth_tokens");
      if (it != headers.end()) {
        LOG(INFO) << "Got optional header props " << it->second
                  << " from thrift header.";
      }
    } else {
      LOG(ERROR) << "Cannot get context.";
    }
    hellogoodbye::HelloReply reply(carbon::Result::OK);
    callback->result(std::move(reply));
  }

  void async_eb_goodbye(
      std::unique_ptr<
          apache::thrift::HandlerCallback<hellogoodbye::GoodbyeReply>> callback,
      const hellogoodbye::GoodbyeRequest& request) override {
    LOG(INFO) << "Good bye! Thrift server " << reinterpret_cast<uintptr_t>(this)
              << " got key " << request.key_ref()->fullKey().str();
    hellogoodbye::GoodbyeReply reply(carbon::Result::OK);
    callback->result(std::move(reply));
  }

  void async_eb_mcVersion(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McVersionReply>> callback,
      const facebook::memcache::McVersionRequest& /* request */) override {
    callback->result(McVersionReply(carbon::Result::OK));
  }
};

inline void spawnServer(AsyncMcServer& server) {
  server.spawn([](size_t /* threadId */,
                  folly::EventBase& evb,
                  AsyncMcServerWorker& worker) {
    worker.setOnRequest(HelloGoodbyeRequestHandler<HelloGoodbyeOnRequest>());

    while (worker.isAlive() || worker.writesPending()) {
      evb.loopOnce();
    }
  });
}

AsyncMcServer::Options getOpts(uint16_t port) {
  AsyncMcServer::Options opts;
  opts.worker.debugFifoPath = "./hello-goodbye";
  opts.ports.push_back(port);
  opts.numThreads = 4;
  return opts;
}

[[maybe_unused]] void testClientServer() {
  // Run simple HelloGoodbye server
  AsyncMcServer server(getOpts(kPort));
  spawnServer(server);

  // Send a few Hello/Goodbye requests
  folly::EventBase evb;
  AsyncMcClient client(
      evb, ConnectionOptions("localhost", kPort, mc_caret_protocol));
  auto& fm = folly::fibers::getFiberManager(evb);
  for (size_t i = 0; i < 100; ++i) {
    using namespace std::chrono_literals;

    if (i % 2 == 0) {
      fm.addTask([&client, i]() {
        auto reply =
            client.sendSync(HelloRequest(folly::sformat("key:{}", i)), 200ms);
        if (*reply.result_ref() != carbon::Result::OK) {
          LOG(ERROR) << "Unexpected result: "
                     << carbon::resultToString(*reply.result_ref());
        }
      });
    } else {
      fm.addTask([&client, i]() {
        auto reply =
            client.sendSync(GoodbyeRequest(folly::sformat("key:{}", i)), 200ms);
        if (*reply.result_ref() != carbon::Result::OK) {
          LOG(ERROR) << "Unexpected result: "
                     << carbon::resultToString(*reply.result_ref());
        }
      });
    }
  }

  while (fm.hasTasks()) {
    evb.loopOnce();
  }

  // Shutdown server
  server.shutdown();
  server.join();
}

void sendHelloRequestSync(
    CarbonRouterClient<HelloGoodbyeRouterInfo>* client,
    std::string key) {
  HelloRequest req(std::move(key));
  req.setCryptoAuthToken("test_cat_token_from_client_req");
  req.shardId_ref() = 1;
  req.message_ref() = "test";
  req.priority_ref() = EnumUInt32::YESTERDAY;
  ;
  folly::fibers::Baton baton;

  client->send(req, [&baton](const HelloRequest&, HelloReply&& reply) {
    LOG(INFO) << "Reply received! Result: "
              << carbon::resultToString(*reply.result_ref())
              << ". Message: " << *reply.message_ref();
    baton.post();
  });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
}

[[maybe_unused]] void testRouter() {
  // Run 2 simple HelloGoodbye server
  AsyncMcServer server1(getOpts(kPort));
  spawnServer(server1);
  AsyncMcServer server2(getOpts(kPort2));
  spawnServer(server2);

  // Start mcrouter
  McrouterOptions routerOpts;
  routerOpts.num_proxies = 2;
  routerOpts.asynclog_disable = true;
  // routerOpts.config_str = R"({"route": "NullRoute"})";
  routerOpts.config_str = R"(
  {
    "pools": {
      "A": {
        "servers": [ "127.0.0.1:11303", "127.0.0.1:11305" ],
        "protocol": "caret"
      }
    },
    "route": {
      "type": "DuplicateRoute",
      "target": "PoolRoute|A",
      "copies": 5
    }
  }
  )";

  auto router = CarbonRouterInstance<HelloGoodbyeRouterInfo>::init(
      "remoteThreadClientTest", routerOpts);
  if (!router) {
    LOG(ERROR) << "Failed to initialize router!";
    return;
  }

  SCOPE_EXIT {
    // Release all router resources on exit
    router->shutdown();
    server1.shutdown();
    server1.join();
    server2.shutdown();
    server2.join();
    freeAllRouters();
  };

  for (int i = 0; i < 10; ++i) {
    auto client = router->createClient(0 /* max_outstanding_requests */);
    sendHelloRequestSync(client.get(), folly::sformat("key:{}", i));
  }
}

[[maybe_unused]] void testCarbonLookasideRouter() {
  // Run 2 simple HelloGoodbye server
  AsyncMcServer server1(getOpts(kPort));
  spawnServer(server1);
  AsyncMcServer server2(getOpts(kPort2));
  spawnServer(server2);

  // Start mcrouter
  McrouterOptions routerOpts;
  routerOpts.num_proxies = 2;
  routerOpts.asynclog_disable = true;
  routerOpts.config_str = R"(
  {
    "pools": {
      "A": {
        "servers": [ "127.0.0.1:11303", "127.0.0.1:11305" ],
        "protocol": "caret"
      }
    },
    "route": {
      "type": "CarbonLookasideRoute",
      "prefix": "petra",
      "ttl": 100,
      "child": {
        "type": "DuplicateRoute",
        "target": "PoolRoute|A",
        "copies": 5
      }
    }
  }
  )";

  auto router = CarbonRouterInstance<HelloGoodbyeRouterInfo>::init(
      "remoteThreadClientTest", routerOpts);
  if (!router) {
    LOG(ERROR) << "Failed to initialize router!";
    return;
  }

  SCOPE_EXIT {
    // Release all router resources on exit
    router->shutdown();
    server1.shutdown();
    server1.join();
    server2.shutdown();
    server2.join();
    freeAllRouters();
  };

  auto client = router->createClient(0 /* max_outstanding_requests */);
  for (int i = 0; i < 10; ++i) {
    sendHelloRequestSync(client.get(), folly::sformat("key:{}", i));
  }

  for (int i = 0; i < 10; ++i) {
    sendHelloRequestSync(client.get(), folly::sformat("key:{}", i));
  }
}

std::thread startThriftServer(
    std::shared_ptr<apache::thrift::ThriftServer> server,
    uint16_t port) {
  folly::Baton baton;
  std::thread serverThread([&baton, server, port]() {
    auto handler = std::make_shared<ThriftHandler>();
    server->setInterface(handler);
    server->setPort(port);
    server->setNumIOWorkerThreads(1);
    baton.post();
    server->serve();
  });
  baton.wait();
  return serverThread;
}

[[maybe_unused]] void testCarbonThriftServer() {
  // Run one simple HelloGoodbye thrift server.
  auto server1 = std::make_shared<apache::thrift::ThriftServer>();
  auto server2 = std::make_shared<apache::thrift::ThriftServer>();
  auto server1Thread = startThriftServer(server1, kPort);
  auto server2Thread = startThriftServer(server2, kPort2);

  // Start mcrouter
  McrouterOptions routerOpts;
  routerOpts.num_proxies = 2;
  routerOpts.asynclog_disable = true;
  routerOpts.probe_delay_initial_ms = 1;
  routerOpts.probe_delay_max_ms = 10;
  routerOpts.enable_compression = true;
  routerOpts.config_str = R"(
  {
    "pools": {
      "A": {
        "servers": [ "127.0.0.1:11303", "127.0.0.1:11305" ],
        "protocol": "thrift"
      }
    },
    "route": "PoolRoute|A"
  }
  )";

  auto router = CarbonRouterInstance<HelloGoodbyeRouterInfo>::init(
      "thriftClientRouter", routerOpts);
  if (!router) {
    LOG(ERROR) << "Failed to initialize router!";
    return;
  }

  auto client = router->createClient(0 /* max_outstanding_requests */);
  for (int i = 0; i < 10; ++i) {
    sendHelloRequestSync(client.get(), folly::sformat("key:{}", i));
  }

  // stop server 1
  server1->stop();
  server1Thread.join();

  for (int i = 10; i < 20; ++i) {
    sendHelloRequestSync(client.get(), folly::sformat("key:{}", i));
  }

  // start server 1 again.
  server1 = std::make_shared<apache::thrift::ThriftServer>();
  server1Thread = startThriftServer(server1, kPort);
  usleep(1000);

  for (int i = 20; i < 30; ++i) {
    sendHelloRequestSync(client.get(), folly::sformat("key:{}", i));
  }

  router->shutdown();
  server1->stop();
  server2->stop();
  server1Thread.join();
  server2Thread.join();
}

} // namespace

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);

  // testClientServer();
  // testRouter();
  // testCarbonLookasideRouter();
  testCarbonThriftServer();

  return 0;
}
