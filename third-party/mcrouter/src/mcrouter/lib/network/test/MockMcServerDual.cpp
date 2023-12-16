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
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/Init.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

#include "mcrouter/ExecutorObserver.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/ServerLoad.h"
#include "mcrouter/lib/network/test/MockMcThriftServerHandler.h"

/**
 * Mock Memcached implementation using both AsyncMcServer and ThriftServer.
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

void serverInit(
    size_t /* threadId */,
    folly::VirtualEventBase&,
    AsyncMcServerWorker& worker) {
  worker.setOnRequest(MemcacheRequestHandler<MockMcOnRequest>());
}

[[noreturn]] void usage(char** argv) {
  std::cerr
      << "Arguments:\n"
         "  -P <port>      TCP port on which to listen for AsyncMcServer\n"
         "  -T <fd>        TCP listen sock fd for AsyncMcServer\n"
         "  -p <port>      TCP port on which to listen for thrift\n"
         "  -t <fd>        TCP listen sock fd for thrift\n"
         "  -l <load>      Fixed server load thrift server returns when requested"
         "Usage:\n"
         "  $ "
      << argv[0] << " -p 15213\n";
  exit(1);
}

// Configure folly to enable INFO+ messages, and everything else to
// enable WARNING+.
// Set the default log handler to log asynchronously by default.
FOLLY_INIT_LOGGING_CONFIG(".=WARNING,folly=INFO; default:async=true");

std::shared_ptr<apache::thrift::ThriftServer> gThriftServer;
std::shared_ptr<AsyncMcServer> gAsyncMcServer;

void shutdown() {
  if (gAsyncMcServer) {
    gAsyncMcServer->shutdown();
    gAsyncMcServer->join();
    gAsyncMcServer.reset();
  }
  if (gThriftServer) {
    gThriftServer->stop();
  }
}

class ShutdownSignalHandler : public folly::AsyncSignalHandler {
 public:
  explicit ShutdownSignalHandler(folly::EventBase* evb)
      : AsyncSignalHandler(evb) {}

  void signalReceived(int) noexcept override {
    shutdown();
  }
};

int main(int argc, char** argv) {
  folly::SingletonVault::singleton()->registrationComplete();

  AsyncMcServer::Options asyncOpts;
  asyncOpts.worker.versionString = "MockMcServer-1.0";

  uint16_t thriftPort = 0;
  int thriftExistingSocketFd = 0;
  size_t numThreads = 1;
  int64_t load = 0;
  bool hasLoad = false;

  int c;
  while ((c = getopt(argc, argv, "P:T:p:t:l:h")) >= 0) {
    switch (c) {
      case 'P':
        asyncOpts.ports.push_back(folly::to<uint16_t>(optarg));
        break;
      case 'T':
        asyncOpts.existingSocketFds = {folly::to<int>(optarg)};
        break;
      case 'p':
        thriftPort = folly::to<uint16_t>(optarg);
        break;
      case 't':
        thriftExistingSocketFd = folly::to<int>(optarg);
        break;
      case 'n':
        numThreads = folly::to<size_t>(optarg);
        break;
      case 'l':
        load = folly::to<int64_t>(optarg);
        hasLoad = true;
        break;
      default:
        usage(argv);
    }
  }

  try {
    // Create IOThreadPoolExecutor and extract event bases
    std::shared_ptr<folly::IOThreadPoolExecutor> ioThreadPool =
        std::make_shared<folly::IOThreadPoolExecutor>(numThreads);
    auto ioThreads = mcrouter::extractEvbs(*ioThreadPool);

    // Thrift server setup
    LOG(INFO) << "Configure thrift server.";
    gThriftServer = std::make_shared<apache::thrift::ThriftServer>();
    apache::thrift::server::observerFactory_.reset();
    auto handler =
        std::make_shared<facebook::memcache::test::MockMcThriftServerHandler>();
    gThriftServer->setInterface(handler);
    if (thriftPort > 0) {
      gThriftServer->setPort(thriftPort);
    } else if (thriftExistingSocketFd > 0) {
      gThriftServer->useExistingSocket(thriftExistingSocketFd);
    }
    gThriftServer->disableActiveRequestsTracking();

    gThriftServer->setIOThreadPool(ioThreadPool);
    gThriftServer->setNumCPUWorkerThreads(1);
    gThriftServer->setQueueTimeout(std::chrono::milliseconds(0));
    gThriftServer->setTaskExpireTime(std::chrono::milliseconds(0));
    gThriftServer->setSocketMaxReadsPerEvent(4);

    if (hasLoad) {
      gThriftServer->setGetLoad([load](const std::string& counter) -> int64_t {
        if (counter == "default") {
          return ServerLoad::fromPercentLoad(load).raw();
        }
        return 0;
      });
    }

    // Register signal handler.
    folly::EventBase* evb =
        gThriftServer->getEventBaseManager()->getEventBase();
    ShutdownSignalHandler shutdownHandler(evb);
    shutdownHandler.registerSignalHandler(SIGTERM);
    shutdownHandler.registerSignalHandler(SIGINT);

    // Now start AsyncMcServer
    LOG(INFO) << "Starting AsyncMcServer";
    asyncOpts.eventBases.push_back(ioThreadPool->getEventBase());
    gAsyncMcServer = std::make_shared<AsyncMcServer>(asyncOpts);

    gAsyncMcServer->startOnVirtualEB(
        &serverInit,
        [evb = gThriftServer->getEventBaseManager()->getEventBase()]() {
          evb->runInEventBaseThread([&]() { shutdown(); });
        });

    // Now start ThriftServer
    gThriftServer->serve();
    LOG(INFO) << "Shutting down AsyncMcServer and ThriftServer.";
    gThriftServer.reset();
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
  }
}
