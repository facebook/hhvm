/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <vector>

#include <folly/Conv.h>
#include <folly/fibers/EventBaseLoopController.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

#include <mcrouter/lib/network/AsyncMcClient.h>
#include <mcrouter/options.h>
#include "mcrouter/lib/network/gen/MemcacheConnection.h"
#include "mcrouter/lib/network/test/ListenSocket.h"
#include "mcrouter/lib/network/test/MockMcThriftServerHandler.h"
#include "mcrouter/lib/network/test/TestClientServerUtil.h"

using facebook::memcache::test::TestServer;

namespace {
std::pair<
    std::shared_ptr<apache::thrift::ThriftServer>,
    std::unique_ptr<std::thread>>
startMockMcThriftServer(const facebook::memcache::ListenSocket& socket) {
  auto server = std::make_shared<apache::thrift::ThriftServer>();
  server->setInterface(
      std::make_shared<facebook::memcache::test::MockMcThriftServerHandler>());
  server->setNumIOWorkerThreads(1);
  server->useExistingSocket(socket.getSocketFd());
  auto thread = std::make_unique<std::thread>([server]() {
    LOG(INFO) << "Starting thrift server.";
    server->serve();
    LOG(INFO) << "Shutting down thrift server.";
  });
  auto conn = std::make_unique<facebook::memcache::MemcacheExternalConnection>(
      facebook::memcache::ConnectionOptions(
          "localhost", socket.getPort(), mc_thrift_protocol));
  bool started = conn->healthCheck();
  for (int i = 0; !started && i < 5; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    LOG(INFO) << folly::sformat(
        "health check thrift server on port {}, retry={}", socket.getPort(), i);
    started = conn->healthCheck();
  }
  conn.reset();
  EXPECT_TRUE(started) << folly::sformat(
      "fail to start thrift server on port {} after max retries",
      socket.getPort());
  return std::make_pair(server, std::move(thread));
}
} // namespace

TEST(MemcacheExternalConnectionTest, simpleExternalConnection) {
  TestServer::Config config;
  config.outOfOrder = false;
  config.useSsl = false;
  auto server = TestServer::create(std::move(config));
  auto conn = std::make_unique<facebook::memcache::MemcacheExternalConnection>(
      facebook::memcache::ConnectionOptions(
          "localhost", server->getListenPort(), mc_caret_protocol));
  facebook::memcache::McSetRequest request("hello");
  request.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "world");
  folly::fibers::Baton baton;
  conn->sendRequestOne(
      request,
      [&baton](
          const facebook::memcache::McSetRequest& /* req */,
          facebook::memcache::McSetReply&& reply) {
        EXPECT_EQ(carbon::Result::STORED, *reply.result_ref());
        baton.post();
      });
  baton.wait();
  baton.reset();
  facebook::memcache::McGetRequest getReq("hello");
  conn->sendRequestOne(
      getReq,
      [&baton](
          const facebook::memcache::McGetRequest& /* req */,
          facebook::memcache::McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
        EXPECT_EQ("hello", folly::StringPiece(reply.value_ref()->coalesce()));
        baton.post();
      });
  baton.wait();
  conn.reset();
  server->shutdown();
  server->join();
}

TEST(MemcacheExternalConnectionTest, simpleExternalConnectionThrift) {
  facebook::memcache::ListenSocket socket;
  auto serverInfo = startMockMcThriftServer(socket);
  auto conn = std::make_unique<facebook::memcache::MemcacheExternalConnection>(
      facebook::memcache::ConnectionOptions(
          "localhost", socket.getPort(), mc_thrift_protocol));
  facebook::memcache::McSetRequest request("hello");
  request.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "world");
  folly::fibers::Baton baton;
  conn->sendRequestOne(
      request,
      [&baton](
          const facebook::memcache::McSetRequest& /* req */,
          facebook::memcache::McSetReply&& reply) {
        EXPECT_EQ(carbon::Result::STORED, *reply.result_ref());
        baton.post();
      });
  baton.wait();
  baton.reset();
  facebook::memcache::McGetRequest getReq("hello");
  conn->sendRequestOne(
      getReq,
      [&baton](
          const facebook::memcache::McGetRequest& /* req */,
          facebook::memcache::McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
        EXPECT_EQ("world", folly::StringPiece(reply.value_ref()->coalesce()));
        baton.post();
      });
  baton.wait();
  conn.reset();
  serverInfo.first->stop();
  serverInfo.second->join();
}

TEST(MemcachePooledConnectionTest, PooledExternalConnection) {
  TestServer::Config config;
  config.outOfOrder = false;
  config.useSsl = false;
  auto server = TestServer::create(std::move(config));
  std::vector<std::unique_ptr<facebook::memcache::MemcacheConnection>> conns;
  for (int i = 0; i < 4; i++) {
    conns.push_back(
        std::make_unique<facebook::memcache::MemcacheExternalConnection>(
            facebook::memcache::ConnectionOptions(
                "localhost", server->getListenPort(), mc_caret_protocol)));
  }
  auto pooledConn =
      std::make_unique<facebook::memcache::MemcachePooledConnection>(
          std::move(conns));
  facebook::memcache::McSetRequest request("pooled");
  request.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "connection");
  folly::fibers::Baton baton;
  pooledConn->sendRequestOne(
      request,
      [&baton](
          const facebook::memcache::McSetRequest& /* req */,
          facebook::memcache::McSetReply&& reply) {
        EXPECT_EQ(carbon::Result::STORED, *reply.result_ref());
        baton.post();
      });
  baton.wait();
  baton.reset();
  facebook::memcache::McGetRequest getReq("pooled");
  pooledConn->sendRequestOne(
      getReq,
      [&baton](
          const facebook::memcache::McGetRequest& /* req */,
          facebook::memcache::McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
        EXPECT_EQ("pooled", folly::StringPiece(reply.value_ref()->coalesce()));
        baton.post();
      });
  baton.wait();
  pooledConn.reset();
  server->shutdown();
  server->join();
}

TEST(MemcacheInternalConnectionTest, simpleInternalConnection) {
  folly::SingletonVault::singleton()->destroyInstances();
  folly::SingletonVault::singleton()->reenableInstances();

  TestServer::Config config;
  config.outOfOrder = false;
  config.useSsl = false;
  auto server = TestServer::create(std::move(config));
  facebook::memcache::McrouterOptions mcrouterOptions;
  mcrouterOptions.num_proxies = 1;
  mcrouterOptions.default_route = "/oregon/*/";
  mcrouterOptions.config_str = folly::sformat(
      R"(
        {{
          "pools": {{
            "A": {{
              "servers": [ "{}:{}" ],
              "protocol": "caret"
            }}
          }},
          "route": "Pool|A"
        }}
      )",
      "localhost",
      server->getListenPort());
  auto conn = std::make_unique<facebook::memcache::MemcacheInternalConnection>(
      "simple-internal-test", mcrouterOptions);
  facebook::memcache::McSetRequest request("internal");
  request.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "connection");
  folly::fibers::Baton baton;
  conn->sendRequestOne(
      request,
      [&baton](
          const facebook::memcache::McSetRequest& /* req */,
          facebook::memcache::McSetReply&& reply) {
        EXPECT_EQ(carbon::Result::STORED, *reply.result_ref());
        baton.post();
      });
  baton.wait();
  baton.reset();
  facebook::memcache::McGetRequest getReq("internal");
  conn->sendRequestOne(
      getReq,
      [&baton](
          const facebook::memcache::McGetRequest& /* req */,
          facebook::memcache::McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
        EXPECT_EQ(
            "internal", folly::StringPiece(reply.value_ref()->coalesce()));
        baton.post();
      });
  baton.wait();
  conn.reset();
  server->shutdown();
  server->join();
}

TEST(MemcachePooledConnectionTest, PooledInternalConnection) {
  folly::SingletonVault::singleton()->destroyInstances();
  folly::SingletonVault::singleton()->reenableInstances();

  TestServer::Config config;
  config.outOfOrder = false;
  config.useSsl = false;
  auto server = TestServer::create(std::move(config));
  facebook::memcache::McrouterOptions mcrouterOptions;
  mcrouterOptions.num_proxies = 1;
  mcrouterOptions.default_route = "/oregon/*/";
  mcrouterOptions.config_str = folly::sformat(
      R"(
        {{
          "pools": {{
            "A": {{
              "servers": [ "{}:{}" ],
              "protocol": "caret"
            }}
          }},
          "route": "Pool|A"
        }}
      )",
      "localhost",
      server->getListenPort());
  std::vector<std::unique_ptr<facebook::memcache::MemcacheConnection>> conns;
  for (int i = 0; i < 4; i++) {
    conns.push_back(
        std::make_unique<facebook::memcache::MemcacheInternalConnection>(
            "pooled-internal-test", mcrouterOptions));
  }
  auto pooledConn =
      std::make_unique<facebook::memcache::MemcachePooledConnection>(
          std::move(conns));
  facebook::memcache::McSetRequest request("pooled");
  request.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "internal");
  folly::fibers::Baton baton;
  pooledConn->sendRequestOne(
      request,
      [&baton](
          const facebook::memcache::McSetRequest& /* req */,
          facebook::memcache::McSetReply&& reply) {
        EXPECT_EQ(carbon::Result::STORED, *reply.result_ref());
        baton.post();
      });
  baton.wait();
  baton.reset();
  facebook::memcache::McGetRequest getReq("pooled");
  pooledConn->sendRequestOne(
      getReq,
      [&baton](
          const facebook::memcache::McGetRequest& /* req */,
          facebook::memcache::McGetReply&& reply) {
        EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
        EXPECT_EQ("pooled", folly::StringPiece(reply.value_ref()->coalesce()));
        baton.post();
      });
  baton.wait();
  pooledConn.reset();
  server->shutdown();
  server->join();
}

TEST(MemcacheResultTest, testThriftResult) {
  facebook::memcache::ListenSocket socket;
  auto serverInfo = startMockMcThriftServer(socket);
  auto conn = std::make_unique<facebook::memcache::MemcacheExternalConnection>(
      facebook::memcache::ConnectionOptions(
          "localhost", socket.getPort(), mc_thrift_protocol));

  folly::fibers::Baton baton;
  facebook::memcache::McGetRequest loadSheddingReq(
      "__mockmc__.want_load_shedding");
  carbon::Result res;
  conn->sendRequestOne(
      loadSheddingReq,
      [&](const facebook::memcache::McGetRequest& /* req */,
          facebook::memcache::McGetReply&& reply) {
        res = *reply.result_ref();
        baton.post();
      });
  baton.wait();
  EXPECT_EQ(res, carbon::Result::RES_TRY_AGAIN);
  conn.reset();
  serverInfo.first->stop();
  serverInfo.second->join();
}

TEST(MemcacheResultTest, testThriftResultForServerRestart) {
  facebook::memcache::ListenSocket socket;
  std::atomic_bool sending{true};
  std::unordered_map<carbon::Result, int> counts;
  auto reqThread = std::make_unique<std::thread>([&]() {
    folly::fibers::Baton baton;
    while (sending) {
      auto conn = facebook::memcache::MemcacheExternalConnection(
          facebook::memcache::ConnectionOptions(
              "localhost", socket.getPort(), mc_thrift_protocol));
      facebook::memcache::McGetRequest req("hello");
      conn.sendRequestOne(
          req,
          [&](const facebook::memcache::McGetRequest& /* req */,
              facebook::memcache::McGetReply&& reply) {
            counts[*reply.result_ref()]++;
            baton.post();
          });
      baton.wait();
      baton.reset();
    }
  });

  auto serverInfo = startMockMcThriftServer(socket);

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  serverInfo.first->stop();
  serverInfo.second->join();

  sending = false;
  reqThread->join();
  reqThread.reset();
  VLOG(1) << "Counts:"
          << std::accumulate(
                 counts.begin(),
                 counts.end(),
                 std::string(""),
                 [](std::string s, const auto& p) -> std::string {
                   return s +
                       folly::to<std::string>(carbon::resultToString(p.first)) +
                       ":" + folly::to<std::string>(p.second) + ",";
                 });
  // During startup and shutdown, expect most error are mc_res_connect_error,
  // may have few mc_res_local_error, but no mc_res_remote_error
  EXPECT_TRUE(counts.find(carbon::Result::CONNECT_ERROR) != counts.end());
  EXPECT_TRUE(counts.find(carbon::Result::REMOTE_ERROR) == counts.end());
}
