/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <signal.h>

#include <iostream>
#include <thread>

#include <glog/logging.h>

#include <folly/Format.h>
#include <folly/Singleton.h>
#include <folly/logging/Init.h>

#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"
#include "mcrouter/lib/network/test/MockMc.h"
#include "mcrouter/lib/network/test/MockMcOnRequest.h"

/**
 * Mock Memcached implementation.
 *
 * The purpose of this program is to:
 *
 * 1) Provide a reference AsyncMcServer use case;
 * 2) Serve as an AsyncMcServer implementation for AsyncMcServer
 *    integration tests;
 * 3) Serve as a Memcached mock for other project's integration tests;
 * 4) Provide an easy to follow Memcached logic reference.
 *
 * The intention is to have the same semantics as our Memcached fork.
 *
 * Certain keys with __mockmc__. prefix provide extra functionality
 * useful for testing.
 */

using namespace facebook::memcache;

void serverLoop(
    size_t /* threadId */,
    folly::EventBase& evb,
    AsyncMcServerWorker& worker) {
  worker.setOnRequest(MemcacheRequestHandler<MockMcOnRequest>());
  evb.loop();
}

[[noreturn]] void usage(char** argv) {
  std::cerr << "Arguments:\n"
               "  -P <port>      TCP port on which to listen\n"
               "  -t <fd>        TCP listen sock fd\n"
               "  -s             Use ssl\n"
               "  -z             Zero Copy Threshold (Default disabled)\n"
               "Usage:\n"
               "  $ "
            << argv[0] << " -p 15213\n";
  exit(1);
}

// Configure folly to enable INFO+ messages, and everything else to
// enable WARNING+.
// Set the default log handler to log asynchronously by default.
FOLLY_INIT_LOGGING_CONFIG(".=WARNING,folly=INFO; default:async=true");

int main(int argc, char** argv) {
  folly::SingletonVault::singleton()->registrationComplete();

  AsyncMcServer::Options opts;
  opts.worker.versionString = "MockMcServer-1.0";

  bool ssl = false;
  uint16_t port = 0;

  int c;
  while ((c = getopt(argc, argv, "P:t:z:sh")) >= 0) {
    switch (c) {
      case 's':
        ssl = true;
        break;
      case 'P':
        port = folly::to<uint16_t>(optarg);
        break;
      case 't':
        opts.existingSocketFds = {folly::to<int>(optarg)};
        break;
      case 'z':
        opts.worker.tcpZeroCopyThresholdBytes = folly::to<int>(optarg);
        break;
      default:
        usage(argv);
    }
  }

  if (ssl) {
    if (port) {
      opts.sslPorts.push_back(port);
    }
    opts.pemCertPath = "mcrouter/lib/network/test/test_cert.pem";
    opts.pemKeyPath = "mcrouter/lib/network/test/test_key.pem";
    opts.pemCaPath = "mcrouter/lib/network/test/ca_cert.pem";
  } else {
    if (port) {
      opts.ports.push_back(port);
    }
  }

  try {
    LOG(INFO) << "Starting server";
    AsyncMcServer server(opts);
    server.installShutdownHandler({SIGINT, SIGTERM});
    server.spawn(&serverLoop);
    server.join();
    LOG(INFO) << "Shutting down";
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
  }
}
