/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <signal.h>

#include <folly/logging/xlog.h>
#include <gflags/gflags.h>

#include <folly/fibers/FiberManagerMap.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McServerRequestContext.h"

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/CarbonRouterFactory.h"
#include "mcrouter/CarbonRouterInstance.h"

#include "mcrouter/config.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {
static constexpr int32_t kMaxNumProxies = 1024;
static constexpr int32_t kMaxNumClients = 1024;

void sendGetRequest(
    CarbonRouterClient<MemcacheRouterInfo>* client,
    std::string key) {
  McGetRequest cacheRequest(std::move(key));
  folly::fibers::Baton baton;
  client->send(cacheRequest, [&baton](const McGetRequest&, McGetReply&&) {
    baton.post();
  });
  baton.wait();
  return;
}
} // namespace

using Pointer = typename CarbonRouterClient<MemcacheRouterInfo>::Pointer;

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
      "{}: Number of clients must be > 0 && < {}",
      flagname,
      kMaxNumClients);
  return false;
}
static bool ValidateNumRequestsPerClient(const char* flagname, int32_t value) {
  if (value > 0) {
    return true;
  }
  XLOGF(ERR, "{}: Number of requests per client must be > 0", flagname);
  return false;
}

DEFINE_string(flavor, "web", "Flavor to use in CarbonRouterInstance");
DEFINE_string(routing_config_override, "", "Override flavor routing config");
DEFINE_int32(num_proxies, 2, "Number of mcrouter proxy threads to create");
DEFINE_int32(num_clients, 10, "Number of clients to create");
DEFINE_int32(num_client_threads, 1, "Number of client threads to create");
DEFINE_int32(num_req_per_client, 10, "Number of requests per client to send");
DEFINE_bool(
    thread_affinity,
    true,
    "Enables/disables thread affinity in CarbonRouterClient");
DEFINE_string(ssl_service_identity, "memcache", "Service Identity to use");
DEFINE_bool(
    ssl_service_identity_authorization_log,
    false,
    "Enables/disables client authorization logging");
DEFINE_bool(
    ssl_service_identity_authorization_enforce,
    false,
    "Enables/disables client authorization enforcement");
DEFINE_validator(num_proxies, &ValidateNumProxies);
DEFINE_validator(num_clients, &ValidateNumClients);
DEFINE_validator(num_req_per_client, &ValidateNumRequestsPerClient);

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  // Create CarbonRouterInstance
  XLOG(INFO, "Creating CarbonRouterInstance");
  std::unordered_map<std::string, std::string> flavorOverrides;
  flavorOverrides.emplace("num_proxies", std::to_string(FLAGS_num_proxies));
  flavorOverrides.emplace("asynclog_disable", "true");
  if (FLAGS_thread_affinity) {
    flavorOverrides.emplace("thread_affinity", "true");
  } else {
    flavorOverrides.emplace("thread_affinity", "false");
  }
  // Service identity
  flavorOverrides.emplace("ssl_service_identity", FLAGS_ssl_service_identity);
  if (FLAGS_ssl_service_identity_authorization_log) {
    flavorOverrides.emplace("ssl_service_identity_authorization_log", "true");
  } else {
    flavorOverrides.emplace("ssl_service_identity_authorization_log", "false");
  }
  if (FLAGS_ssl_service_identity_authorization_enforce) {
    flavorOverrides.emplace(
        "ssl_service_identity_authorization_enforce", "true");
  } else {
    flavorOverrides.emplace(
        "ssl_service_identity_authorization_enforce", "false");
  }
  if (!FLAGS_routing_config_override.empty()) {
    flavorOverrides.emplace("config", "file:" + FLAGS_routing_config_override);
  }

  std::shared_ptr<CarbonRouterInstance<MemcacheRouterInfo>> router;
  try {
    router = createRouterFromFlavor<MemcacheRouterInfo>(
        FLAGS_flavor, flavorOverrides);
  } catch (std::exception& ex) {
    XLOGF(
        ERR,
        "Error creating router for threadAffinity for flavor {}. Exception: {}",
        FLAGS_flavor,
        ex.what());
    return 1;
  }

  SCOPE_EXIT {
    // Release all router resources on exit
    router->shutdown();
    freeAllRouters();
  };

  XLOGF(INFO, "Creating {} CarbonRouterClient", FLAGS_num_clients);
  std::vector<std::thread> threads;
  for (int t = 0; t < FLAGS_num_client_threads; ++t) {
    threads.push_back(std::thread([&]() {
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
          sendGetRequest(clients[i].get(), folly::sformat("key:{}{}", i, j));
        }
      }
    }));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  for (int i = 0; i < FLAGS_num_proxies; ++i) {
    XLOGF(
        INFO,
        "Total Connections proxy_{}: {}",
        i,
        router->getProxyBase(i)->stats().getValue(num_servers_up_stat));
    XLOGF(
        INFO,
        "Authorization Successes proxy_{}: {}",
        i,
        router->getProxyBase(i)->stats().getValue(
            num_authorization_successes_stat));
    XLOGF(
        INFO,
        "Authorization Failures proxy_{}: {}",
        i,
        router->getProxyBase(i)->stats().getValue(
            num_authorization_failures_stat));
  }
  return 0;
}
