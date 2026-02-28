/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/AsyncMcServerWorkerOptions.h"
#include "mcrouter/lib/network/McServerSession.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"
#include "mcrouter/lib/network/test/SessionTestHarness.h"

using namespace facebook::memcache;

using std::string;
using std::vector;

TEST(Session, basic) {
  AsyncMcServerWorkerOptions opts;
  opts.versionString = "Test-1.0";
  SessionTestHarness t(opts);
  t.inputPackets("get ke", "y\r\n", "version\r\n");

  EXPECT_EQ(
      vector<string>({"VALUE key 0 9\r\nkey_value\r\nEND\r\n"
                      "VERSION Test-1.0\r\n"}),
      t.flushWrites());

  t.closeSession();
}

TEST(Session, throttle) {
  AsyncMcServerWorkerOptions opts;
  opts.maxInFlight = 2;
  SessionTestHarness t(opts);

  /* Send 5 requests but don't reply them yet; only first 2 will be read */
  t.pause();
  t.inputPackets(
      "get key1\r\n",
      "get key2\r\n",
      "get key3\r\n",
      "get key4\r\n",
      "get key5\r\n");

  EXPECT_TRUE(t.flushWrites().empty());
  EXPECT_EQ(vector<string>({"key1", "key2"}), t.pausedKeys());

  /* Now reply to the first request; one more request will be read */
  t.resume(1);

  EXPECT_EQ(
      vector<string>({"VALUE key1 0 10\r\nkey1_value\r\nEND\r\n"}),
      t.flushWrites());
  EXPECT_EQ(vector<string>({"key2", "key3"}), t.pausedKeys());

  /* Finally reply to everything */
  t.resume();
  EXPECT_EQ(
      vector<string>(
          {"VALUE key2 0 10\r\nkey2_value\r\nEND\r\nVALUE key3 0 10\r\nkey3_value"
           "\r\nEND\r\n",
           "VALUE key4 0 10\r\nkey4_value\r\nEND\r\n",
           "VALUE key5 0 10\r\nkey5_value\r\nEND\r\n"}),
      t.flushWrites());
  EXPECT_TRUE(t.pausedKeys().empty());

  t.closeSession();
}

TEST(Session, throttleBigPacket) {
  AsyncMcServerWorkerOptions opts;
  opts.maxInFlight = 2;
  SessionTestHarness t(opts);

  /* Network throttling only applies to new packets.
     If an unthrottled packet contains multiple requests,
     we will process them all even if it will push us over limit. */

  /* Send 5 requests in 3 packets.
     First 3 will go through even though maxInFlight is 2 */
  t.pause();
  t.inputPackets(
      "get key1\r\nget key2\r\nget key3\r\n", "get key4\r\n", "get key5\r\n");

  EXPECT_TRUE(t.flushWrites().empty());
  EXPECT_EQ(vector<string>({"key1", "key2", "key3"}), t.pausedKeys());

  /* Now reply to the first request; no more requests will be read
     since we're still at the limit */
  t.resume(1);

  EXPECT_EQ(
      vector<string>({"VALUE key1 0 10\r\nkey1_value\r\nEND\r\n"}),
      t.flushWrites());
  EXPECT_EQ(vector<string>({"key2", "key3"}), t.pausedKeys());

  /* Finally reply to everything */
  t.resume();
  EXPECT_EQ(
      vector<string>(
          {"VALUE key2 0 10\r\nkey2_value\r\nEND\r\nVALUE key3 0 10\r\nkey3_value"
           "\r\nEND\r\n",
           "VALUE key4 0 10\r\nkey4_value\r\nEND\r\n",
           "VALUE key5 0 10\r\nkey5_value\r\nEND\r\n"}),
      t.flushWrites());
  EXPECT_TRUE(t.pausedKeys().empty());

  t.closeSession();
}

TEST(Session, quit) {
  AsyncMcServerWorkerOptions opts;
  SessionTestHarness t(opts);
  t.inputPackets("get ke", "y\r\n", "quit\r\nget key2\r\n");

  /* First get should go through; then quit will close the connection
     and second get will be ignored */

  EXPECT_EQ(
      vector<string>({"VALUE key 0 9\r\nkey_value\r\nEND\r\n"}),
      t.flushWrites());
}

/* Same as McServerAsciiParserHarness.quitWithVersion, except Async */
TEST(Session, quitWithVersion) {
  AsyncMcServerWorkerOptions opts;
  SessionTestHarness t(opts);
  t.inputPackets("quit\r\nversion");

  EXPECT_EQ(vector<string>({}), t.flushWrites());
}

TEST(Session, closeBeforeReply) {
  struct Callbacks : public McServerSession::StateCallback {
   public:
    void onAccepted(McServerSession&) final {}
    void onWriteQuiescence(McServerSession&) final {
      EXPECT_EQ(state_, ACTIVE);
    }
    void onCloseStart(McServerSession&) final {}
    void onCloseFinish(McServerSession&, bool) final {
      EXPECT_EQ(state_, ACTIVE);
      state_ = CLOSED;
    }
    void onShutdown() final {}

   private:
    enum State { ACTIVE, CLOSED };
    State state_{ACTIVE};
  } callbacks;

  AsyncMcServerWorkerOptions opts;
  SessionTestHarness t(opts, callbacks);

  // input packets, close session and then reply
  t.inputPackets("get key\r\n");
  t.closeSession();
  t.resume();
}

struct NoOpOnRequest {
 public:
  NoOpOnRequest() {}

  template <class Request>
  void onRequest(McServerRequestContext&&, Request&&) {}
};

TEST(Session, invalidSocketAdd) {
  const int invalidFd = socket(AF_INET6, SOCK_STREAM, 0);
  EXPECT_NE(invalidFd, -1);
  close(invalidFd);

  AsyncMcServerWorkerOptions opts;
  folly::EventBase base;
  AsyncMcServerWorker worker(opts, base);

  worker.setOnRequest(MemcacheRequestHandler<NoOpOnRequest>());
  worker.setOnWriteQuiescence([](McServerSession&) {});
  worker.setOnConnectionCloseStart([](McServerSession&) {});
  worker.setOnConnectionCloseFinish([](McServerSession&, bool) {});

  worker.addClientSocket(invalidFd);
}
