/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <signal.h>

#include <gflags/gflags.h>

#include <folly/fibers/FiberManagerMap.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>

#include "mcrouter/lib/carbon/example/gen/HelloGoodbye.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McServerRequestContext.h"

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/config.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;
using namespace hellogoodbye;

namespace {
std::atomic<uint64_t> gCurrentConns{0};
std::atomic<uint64_t> gTotalConns{0};
constexpr uint16_t kPort = 11303;

struct ThreadAffinityOnRequest {
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

void connectionAccepted(McServerSession&) {
  XLOG(INFO, "Accepted connection");
  gCurrentConns++;
  gTotalConns++;
}

void connectionClosed(McServerSession&, bool onAcceptedCalled) {
  if (onAcceptedCalled) {
    gCurrentConns--;
  }
}

inline void spawnServer(AsyncMcServer& server) {
  server.installShutdownHandler({SIGINT, SIGTERM});
  server.spawn([](size_t /* threadId */,
                  folly::EventBase& evb,
                  AsyncMcServerWorker& worker) {
    worker.setOnRequest(HelloGoodbyeRequestHandler<ThreadAffinityOnRequest>());
    worker.setOnConnectionAccepted(&connectionAccepted);
    worker.setOnConnectionCloseFinish(&connectionClosed);

    while (worker.isAlive() || worker.writesPending()) {
      evb.loopOnce();
    }
  });
}

void sendHelloRequestSync(
    CarbonRouterClient<HelloGoodbyeRouterInfo>* client,
    std::string key) {
  HelloRequest req(std::move(key));
  folly::fibers::Baton baton;

  client->send(req, [&baton](const HelloRequest&, HelloReply&& reply) {
    XLOG(INFO) << "Reply received! Result: "
               << carbon::resultToString(*reply.result_ref());
    baton.post();
  });

  // Ensure proxies have a chance to send all outstanding requests. Note the
  // extra synchronization required when using a remote-thread client.
  baton.wait();
}

AsyncMcServer::Options getOpts() {
  AsyncMcServer::Options opts;
  opts.worker.debugFifoPath = "./ta-hello-goodbye";
  opts.ports.push_back(kPort);
  opts.numThreads = 4;
  return opts;
}
} // namespace

using Pointer = typename CarbonRouterClient<HelloGoodbyeRouterInfo>::Pointer;

// GFLAGs
static constexpr int32_t kMaxNumProxies = 1024;
static constexpr int32_t kMaxNumClients = 1024;

static bool ValidateNumProxies(const char* flagname, int32_t value) {
  if (value > 0 && value < kMaxNumProxies) {
    return true;
  }
  XLOGF(
      ERR,
      "{}: Number of proxies must be > 0 && < {}",
      flagname,
      kMaxNumProxies);
  return false;
}
static bool ValidateNumClients(const char* flagname, int32_t value) {
  if (value > 0 && value < kMaxNumClients) {
    return true;
  }
  XLOGF(
      ERR,
      "{}: Number of proxies must be > 0 && < {}",
      flagname,
      kMaxNumClients);
  return false;
}
static bool ValidateNumRequestsPerClient(const char* flagname, int32_t value) {
  if (value > 0) {
    return true;
  }
  XLOGF(ERR, "{}: Number of proxies must be > 0", flagname);
  return false;
}

DEFINE_int32(server_port, 2, "AsyncMcServer listening port");
DEFINE_int32(num_proxies, 2, "Number of mcrouter proxy threads to create");
DEFINE_int32(num_clients, 10, "Number of clients to create");
DEFINE_int32(num_req_per_client, 10, "Number of requests per client to send");
DEFINE_bool(
    thread_affinity,
    true,
    "Enables/disables thread affinity in CarbonRouterClient");
DEFINE_validator(num_proxies, &ValidateNumProxies);
DEFINE_validator(num_clients, &ValidateNumClients);
DEFINE_validator(num_req_per_client, &ValidateNumRequestsPerClient);

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  // Start a single AsyncMcServer instance
  XLOG(INFO, "Starting AsyncMcServer");
  facebook::memcache::AsyncMcServer server(getOpts());
  spawnServer(server);

  // Create CarbonRouterInstance
  XLOG(INFO, "Creating CarbonRouterInstance");
  McrouterOptions routerOpts;
  routerOpts.num_proxies = FLAGS_num_proxies;
  routerOpts.asynclog_disable = true;
  routerOpts.thread_affinity = FLAGS_thread_affinity;
  // Make port configurable
  routerOpts.config_str = R"(
  {
    "pools": {
      "A": {
        "servers": [ "127.0.0.1:11303" ],
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
      "threadAffinityRouter", routerOpts);
  if (!router) {
    XLOG(ERR) << "Failed to initialize router!";
    return 0;
  }

  SCOPE_EXIT {
    // Release all router resources on exit
    router->shutdown();
    server.shutdown();
    server.join();
    freeAllRouters();
  };

  XLOGF(INFO, "Creating {} CarbonRouterClient", FLAGS_num_clients);
  // Create CarbonRouterClient's
  std::vector<Pointer> clients;
  for (int i = 0; i < FLAGS_num_clients; ++i) {
    clients.push_back(
        router->createClient(0 /* max_outstanding_requests */, false));
  }
  for (int i = 0; i < FLAGS_num_clients; ++i) {
    XLOGF(
        INFO,
        "Sending {} requests to CarbonRouterClient {}",
        FLAGS_num_req_per_client,
        i);
    for (int j = 0; j < FLAGS_num_req_per_client; ++j) {
      sendHelloRequestSync(clients[i].get(), folly::sformat("key:{}{}", i, j));
    }
  }

  XLOGF(
      INFO,
      "Total Connections: {}. Current Connections: {}.",
      gTotalConns.load(),
      gCurrentConns.load());
  return 0;
}
