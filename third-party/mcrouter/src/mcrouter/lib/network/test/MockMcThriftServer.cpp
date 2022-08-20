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
#include <thrift/lib/cpp2/server/ThriftServer.h>

#include "mcrouter/lib/network/test/MockMcThriftServerHandler.h"

/**
 * Mock Memcached implementation over Thrift.
 *
 * The purpose of this program is to serve as a Memcached over thrift mock for
 * other project's integration tests;
 *
 * The intention is to have the same semantics as our Memcached fork.
 *
 * Certain keys with __mockmc__. prefix provide extra functionality
 * useful for testing.
 */
[[noreturn]] void usage(char** argv) {
  std::cerr << "Arguments:\n"
               "  -P <port>      TCP port on which to listen\n"
               "  -t <fd>        TCP listen sock fd\n"
               "Usage:\n"
               "  $ "
            << argv[0] << " -p 15213\n";
  exit(1);
}

// Configure folly to enable INFO+ messages, and everything else to
// enable WARNING+.
// Set the default log handler to log asynchronously by default.
FOLLY_INIT_LOGGING_CONFIG(".=WARNING,folly=INFO; default:async=true");

std::shared_ptr<apache::thrift::ThriftServer> gServer;

void sigHandler(int /* signo */) {
  gServer->stop();
}

int main(int argc, char** argv) {
  folly::SingletonVault::singleton()->registrationComplete();

  uint16_t port = 0;
  int existingSocketFd = 0;

  int c;
  while ((c = getopt(argc, argv, "P:t:h")) >= 0) {
    switch (c) {
      case 'P':
        port = folly::to<uint16_t>(optarg);
        break;
      case 't':
        existingSocketFd = folly::to<int>(optarg);
        break;
      default:
        usage(argv);
    }
  }

  try {
    LOG(INFO) << "Starting thrift server.";
    gServer = std::make_shared<apache::thrift::ThriftServer>();
    auto handler =
        std::make_shared<facebook::memcache::test::MockMcThriftServerHandler>();
    gServer->setInterface(handler);
    if (port > 0) {
      gServer->setPort(port);
    } else if (existingSocketFd > 0) {
      gServer->useExistingSockets({existingSocketFd});
    }
    gServer->setNumIOWorkerThreads(2);
    signal(SIGINT, sigHandler);
    gServer->serve();
    LOG(INFO) << "Shutting down thrift server.";
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
  }
}
