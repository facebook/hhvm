/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <functional>
#include <string>

#include <folly/Function.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"
#include "mcrouter/lib/network/test/ListenSocket.h"

namespace folly {
class AsyncSocket;
} // namespace folly

// Used to set up server options
constexpr const char* getDefaultCaPath() {
  return "mcrouter/lib/network/test/ca_cert.pem";
}
constexpr const char* getDefaultKeyPath() {
  return "mcrouter/lib/network/test/test_key.pem";
}
constexpr const char* getDefaultCertPath() {
  return "mcrouter/lib/network/test/test_cert.pem";
}

namespace facebook {
namespace memcache {

class CompressionCodecMap;
struct RpcStatsContext;

namespace test {

class TestServerOnRequest {
 public:
  TestServerOnRequest(folly::fibers::Baton& shutdownLock, bool outOfOrder);

  void onRequest(McServerRequestContext&& ctx, McGetRequest&& req);
  void onRequest(McServerRequestContext&& ctx, McSetRequest&& req);
  void onRequest(McServerRequestContext&& ctx, McVersionRequest&& req);

  template <class Request>
  void onRequest(McServerRequestContext&&, Request&&) {
    LOG(ERROR) << "Unhandled operation: " << Request::name;
  }

 protected:
  template <class Reply>
  void processReply(McServerRequestContext&& context, Reply&& reply) {
    if (outOfOrder_) {
      McServerRequestContext::reply(std::move(context), std::move(reply));
    } else {
      waitingReplies_.push_back(
          [ctx = std::move(context), reply = std::move(reply)]() mutable {
            McServerRequestContext::reply(std::move(ctx), std::move(reply));
          });
      if (waitingReplies_.size() == 1) {
        flushQueue();
      }
    }
  }

 private:
  folly::fibers::Baton& shutdownLock_;
  bool outOfOrder_;
  std::vector<folly::Function<void()>> waitingReplies_;

  void flushQueue();
};

class TestServer {
 public:
  ~TestServer();

  struct Config {
    bool outOfOrder = true;
    bool useSsl = true;
    int maxInflight = 10;
    int timeoutMs = 250;
    size_t maxConns = 100;
    bool useDefaultVersion = false;
    size_t numThreads = 1;
    bool useTicketKeySeeds = false;
    size_t goAwayTimeoutMs = 1000;
    const CompressionCodecMap* compressionCodecMap = nullptr;
    bool tfoEnabled = false;
    std::string caPath = getDefaultCaPath();
    std::string certPath = getDefaultCertPath();
    std::string keyPath = getDefaultKeyPath();
    bool requirePeerCerts = true;
    bool tlsPreferOcbCipher = false;
    std::function<void(McServerSession&)> onConnectionAcceptedAdditionalCb;
    size_t tcpZeroCopyThresholdBytes = 0;
    bool useKtls12 = false;
    bool tosReflection = false;
  };

  template <class OnRequest = TestServerOnRequest>
  static std::unique_ptr<TestServer> create() {
    return create(Config());
  }

  template <class OnRequest = TestServerOnRequest>
  static std::unique_ptr<TestServer> create(
      Config config,
      folly::Function<MemcacheRequestHandler<OnRequest>(
          folly::fibers::Baton& shutdownLock,
          bool outOfOrder)> onRequestCreator =
          [](folly::fibers::Baton& shutdownLockArg, bool outOfOrderArg) {
            return MemcacheRequestHandler<OnRequest>(
                shutdownLockArg, outOfOrderArg);
          }) {
    std::unique_ptr<TestServer> server(new TestServer(config));
    server->run([&config, &onRequestCreator, &s = *server](
                    AsyncMcServerWorker& worker) mutable {
      worker.setCompressionCodecMap(config.compressionCodecMap);
      worker.setOnRequest(onRequestCreator(s.shutdownLock_, s.outOfOrder_));
    });
    return server;
  }

  uint16_t getListenPort() const {
    return sock_.getPort();
  }

  void join() {
    if (serverThread_.joinable()) {
      serverThread_.join();
    }
  }

  size_t getAcceptedConns() const {
    return acceptedConns_.load();
  }

  void shutdown() {
    shutdownLock_.post();
  }

  std::string version() const;

 private:
  ListenSocket sock_;
  AsyncMcServer::Options opts_;
  std::unique_ptr<AsyncMcServer> server_;
  std::thread serverThread_;
  bool outOfOrder_{false};
  bool useTicketKeySeeds_{false};
  folly::fibers::Baton shutdownLock_;
  std::atomic<size_t> acceptedConns_{0};
  std::function<void(McServerSession&)> onConnectionAcceptedAdditionalCb_;

  explicit TestServer(Config config);

  void run(std::function<void(AsyncMcServerWorker&)> init);
};

struct SSLTestPaths {
  std::string sslCertPath;
  std::string sslKeyPath;
  std::string sslCaPath;
  SecurityMech mech{SecurityMech::TLS};
  bool useOcbCipher{false};
};

// valid Client SSL Certs
SSLTestPaths validClientSsl();
// non-existent client SSL certs
SSLTestPaths invalidClientSsl();
// broken client SSL certs (handshake fails)
SSLTestPaths brokenClientSsl();
// client config w/o certs
SSLTestPaths noCertClientSsl();

// valid SSL certs for server
SSLTestPaths validSsl();

class TestClient {
 public:
  TestClient(
      std::string host,
      uint16_t port,
      int timeoutMs,
      mc_protocol_t protocol = mc_ascii_protocol,
      folly::Optional<SSLTestPaths> ssl = folly::none,
      uint64_t qosClass = 0,
      uint64_t qosPath = 0,
      std::string serviceIdentity = "",
      const CompressionCodecMap* compressionCodecMap = nullptr,
      bool enableTfo = false,
      bool offloadHandshakes = false,
      bool sessionCachingEnabled = true);

  void setThrottle(size_t maxInflight, size_t maxOutstanding) {
    client_->setThrottle(maxInflight, maxOutstanding);
  }

  void setConnectionStatusCallbacks(
      std::function<void(const folly::AsyncTransportWrapper&, int64_t)> onUp,
      std::function<void(ConnectionDownReason, int64_t)> onDown);

  void sendGet(
      std::string key,
      carbon::Result expectedResult,
      uint32_t timeoutMs = 200,
      std::function<void(const RpcStatsContext&)> rpcStatsCallback = nullptr);

  void sendSet(
      std::string key,
      std::string value,
      carbon::Result expectedResult,
      uint32_t timeoutMs = 200,
      std::function<void(const RpcStatsContext&)> rpcStatsCallback = nullptr);

  void sendVersion(std::string expectedVersion);

  /**
   * Wait while there're more than remaining requests in queue.
   */
  void waitForReplies(size_t remaining = 0);

  /**
   * Loop once client EventBase, will cause it to write requests into socket.
   */
  void loopOnce() {
    eventBase_.loopOnce();
  }

  AsyncMcClient& getClient() {
    return *client_;
  }

  /**
   * Get the max number of pending requests at any point in time.
   */
  int getMaxPendingReqs() {
    return pendingStatMax_;
  }

  /**
   * Get the max number of inflight requests at any point in time.
   */
  int getMaxInflightReqs() {
    return inflightStatMax_;
  }

 private:
  size_t inflight_{0};
  std::unique_ptr<AsyncMcClient> client_;
  folly::EventBase eventBase_;
  folly::fibers::FiberManager fm_;

  // Stats
  int pendingStat_{0};
  int inflightStat_{0};
  int pendingStatMax_{0};
  int inflightStatMax_{0};
};

std::string genBigValue();
} // namespace test
} // namespace memcache
} // namespace facebook
