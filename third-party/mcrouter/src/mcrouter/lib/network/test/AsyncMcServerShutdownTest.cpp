/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <folly/fibers/FiberManagerMap.h>
#include <folly/io/async/EventBaseManager.h>

#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeServer.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McServerRequestContext.h"

using namespace facebook::memcache;

int newSocketAnyPort() {
  int sock = socket(PF_INET6, SOCK_STREAM, 0);
  CHECK_NE(sock, -1);

  sockaddr_in6 addr;
  addr.sin6_family = AF_INET6;
  addr.sin6_port = 0;
  addr.sin6_addr = in6addr_any;

  int bound = ::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  CHECK_NE(bound, -1);

  return sock;
}

int getSocketPort(int socketFd) {
  sockaddr_in6 addr;
  socklen_t len = sizeof(struct sockaddr_in6);
  CHECK_NE(getsockname(socketFd, reinterpret_cast<sockaddr*>(&addr), &len), -1);

  return ntohs(addr.sin6_port);
}

class TestOnRequest {
 public:
  explicit TestOnRequest(folly::EventBase& evb) {
    fiberManager_ = &folly::fibers::getFiberManager(evb);
  }

  void onRequest(
      McServerRequestContext&& context,
      hellogoodbye::HelloRequest&& request) {
    fiberManager_->addTaskRemote([ctx = std::move(context),
                                  req = std::move(request)]() mutable {
      McServerRequestContext::reply(std::move(ctx), hellogoodbye::HelloReply());
    });
  }

 private:
  folly::fibers::FiberManager* fiberManager_;
};

class TestServer {
 public:
  explicit TestServer(const size_t threadCount)
      : socketFd_(newSocketAnyPort()), port_(getSocketPort(socketFd_)) {
    AsyncMcServer::Options opts;
    opts.numThreads = threadCount;
    opts.worker.sendTimeout = std::chrono::milliseconds(10);
    opts.existingSocketFds = {socketFd_};
    server_ = std::make_unique<AsyncMcServer>(opts);
  }

  ~TestServer() {
    close(socketFd_);
  }

  uint16_t getPort() const {
    return port_;
  }

  void start() {
    server_->spawn([](size_t /* threadId */,
                      folly::EventBase& evb,
                      AsyncMcServerWorker& worker) {
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
  std::unique_ptr<AsyncMcServer> server_;
  const int socketFd_;
  const uint16_t port_;
};

class TestClient : public std::enable_shared_from_this<TestClient> {
 public:
  explicit TestClient(uint16_t port)
      : eventBase_(folly::EventBaseManager::get()->getEventBase()),
        fm_(folly::fibers::getFiberManager(*eventBase_)) {
    ConnectionOptions opts("localhost", port, mc_caret_protocol);
    client_ = std::make_unique<AsyncMcClient>(*eventBase_, opts);
  }

  void send() {
    fm_.addTask([self = shared_from_this()]() {
      self->client_->sendSync(
          hellogoodbye::HelloRequest(), std::chrono::milliseconds(100));
    });
  }

  std::unique_ptr<AsyncMcClient> client_;
  folly::EventBase* eventBase_;
  folly::fibers::FiberManager& fm_;
};

TEST(AsyncMcServerShutdownTest, postShutdownConnections) {
  const size_t kNumWorkers = 16;
  const size_t kNumClients = 64;
  const size_t kNumRequests = 50;

  TestServer server(kNumWorkers);
  server.start();

  std::vector<std::thread> threads;
  for (size_t i = 0; i < kNumClients; ++i) {
    threads.emplace_back([&]() {
      auto client = std::make_shared<TestClient>(server.getPort());
      for (size_t r = 0; r < kNumRequests; ++r) {
        client->send();
      }
    });
  }

  server.shutdown();
  for (auto& t : threads) {
    t.join();
  }
}
