/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/connpool/test/SessionPoolTestFixture.h>

#include <proxygen/lib/http/connpool/ServerIdleSessionController.h>
#include <proxygen/lib/http/connpool/SessionHolder.h>
#include <proxygen/lib/http/connpool/SessionPool.h>
#include <proxygen/lib/http/connpool/ThreadIdleSessionController.h>

#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <folly/synchronization/Baton.h>
#include <wangle/acceptor/ConnectionManager.h>

#include <proxygen/lib/http/session/test/HQSessionMocks.h>

using namespace proxygen;
using namespace std;
using namespace testing;

TEST_F(SessionPoolFixture, ParallelPoolChangedMaxSessions) {
  SessionPool p(this, 1);
  HTTPCodec::Callback* cb = nullptr;
  auto codec = makeParallelCodec();
  EXPECT_CALL(*codec, setCallback(_)).WillRepeatedly(SaveArg<0>(&cb));
  p.putSession(makeSession(std::move(codec)));

  // inserted session should be in IDLE state
  EXPECT_EQ(p.getNumSessions(), 1);
  EXPECT_EQ(p.getNumIdleSessions(), 1);

  // blocked on opening streams should transition the session to filled state
  cb->onSettings({{SettingsId::MAX_CONCURRENT_STREAMS, 0}});
  evb_.loop();

  EXPECT_EQ(p.getNumSessions(), 1);
  EXPECT_EQ(p.getNumFullSessions(), 1);
}

TEST_F(SessionPoolFixture, SerialPoolBasic) {
  SessionPool p(this, 1);
  p.putSession(makeSerialSession());
  auto txn = p.getTransaction(this);
  ASSERT_TRUE(txn != nullptr);
  ASSERT_TRUE(p.getTransaction(this) == nullptr);

  // Clear the pool
  p.setMaxIdleSessions(0);

  // Drop the transaction. All transactions on the sessions in the pool
  // must be completed before the pool can be destroyed
  txn->sendAbort();

  ASSERT_EQ(activated_, 1);
  ASSERT_EQ(deactivated_, 1);
  evb_.loop();
  ASSERT_EQ(closed_, 1);
}

TEST_F(SessionPoolFixture, ParallelPoolBasic) {
  const int numTxns = 32;
  HTTPTransaction* txns[numTxns];

  SessionPool p(this, 1);
  p.putSession(makeParallelSession());

  for (int i = 0; i < numTxns; ++i) {
    txns[i] = p.getTransaction(this);
    ASSERT_TRUE(txns[i] != nullptr);
  }

  // Clear the pool
  p.setMaxIdleSessions(0);

  // Drop the transactions. All transactions on the sessions in the pool
  // must be completed before the pool can be destroyed
  for (int i = 0; i < numTxns; ++i) {
    txns[i]->sendAbort();
  }

  ASSERT_EQ(activated_, 1);
  ASSERT_EQ(deactivated_, 1);
  evb_.loop();
  ASSERT_EQ(closed_, 1);
}

TEST_F(SessionPoolFixture, SerialPoolPurge) {
  // Put more sessions into the pool than can fit. Then open several
  // transactions on this pool and make sure we can't get out more
  // transactions than the size of the pool.
  const int sessionLimit = 10;
  const int sessionPut = 12;
  HTTPTransaction* txns[sessionPut];

  SessionPool p(this, sessionLimit);
  for (int i = 0; i < sessionPut; ++i) {
    p.putSession(makeSerialSession());
  }

  evb_.loop();
  ASSERT_EQ(activated_, 0);
  ASSERT_EQ(deactivated_, 0);
  ASSERT_EQ(closed_, sessionPut - sessionLimit);

  for (int i = 0; i < sessionPut; ++i) {
    txns[i] = p.getTransaction(this);
    if (i < sessionLimit) {
      ASSERT_TRUE(txns[i] != nullptr);
    } else {
      ASSERT_TRUE(txns[i] == nullptr);
    }
  }
  ASSERT_EQ(activated_, sessionLimit);

  // Clear the pool
  p.setMaxIdleSessions(0);
  // The txns are still active, so nothing should have been deactivated yet.
  ASSERT_EQ(deactivated_, 0);

  // Drop the transactions. All transactions on the sessions in the pool
  // must be completed before the pool can be destroyed
  for (int i = 0; i < sessionPut; ++i) {
    if (txns[i]) {
      txns[i]->sendAbort();
    }
  }
  evb_.loop();
  ASSERT_EQ(activated_, sessionLimit);
  ASSERT_EQ(deactivated_, sessionLimit);
  ASSERT_EQ(closed_, sessionPut);
}

TEST_F(SessionPoolFixture, ParallelPoolLists) {
  // Test where we put in 2 parallel sessions into the pool. Ensure that
  // the first one fills up before we start using the second.
  SessionPool p(this, 2);
  std::vector<HTTPTransaction*> txnsSess1;
  std::vector<HTTPTransaction*> txnsSess2;

  auto sess1 = makeParallelSession();
  auto sess2 = makeParallelSession();
  p.putSession(sess1);
  p.putSession(sess2);
  txnsSess1.push_back(CHECK_NOTNULL(p.getTransaction(this)));
  // Since these two sessions are equally old, it's ok for either one to
  // be selected to become "active"
  if (sess2->getNumOutgoingStreams() > sess1->getNumOutgoingStreams()) {
    std::swap(sess1, sess2);
  }
  ASSERT_EQ(sess1->getNumOutgoingStreams(), 1);
  ASSERT_EQ(sess2->getNumOutgoingStreams(), 0);
  ASSERT_EQ(activated_, 1);
  ASSERT_EQ(deactivated_, 0);
  ASSERT_EQ(closed_, 0);

  // sess1 should be filled completely before we add any transactions to sess2
  while (sess1->supportsMoreTransactions()) {
    txnsSess1.push_back(CHECK_NOTNULL(p.getTransaction(this)));
  }
  ASSERT_EQ(sess1->getNumOutgoingStreams(),
            sess1->getMaxConcurrentOutgoingStreams());
  ASSERT_EQ(sess2->getNumOutgoingStreams(), 0);
  ASSERT_EQ(activated_, 1);
  ASSERT_EQ(deactivated_, 0);
  ASSERT_EQ(closed_, 0);

  // Now fill sess2
  while (sess2->supportsMoreTransactions()) {
    txnsSess2.push_back(CHECK_NOTNULL(p.getTransaction(this)));
  }
  // The two sessions should be completely full
  ASSERT_EQ(sess1->getNumOutgoingStreams(),
            sess1->getMaxConcurrentOutgoingStreams());
  ASSERT_EQ(sess2->getNumOutgoingStreams(),
            sess2->getMaxConcurrentOutgoingStreams());
  ASSERT_EQ(activated_, 2);
  ASSERT_EQ(deactivated_, 0);
  ASSERT_EQ(closed_, 0);

  // Adding 1 more txn should fail since both sessions are full
  CHECK(nullptr == p.getTransaction(this));

  evb_.loop();
  ASSERT_EQ(sess1->getNumOutgoingStreams(),
            sess1->getMaxConcurrentOutgoingStreams());
  ASSERT_EQ(sess2->getNumOutgoingStreams(),
            sess2->getMaxConcurrentOutgoingStreams());
  ASSERT_EQ(activated_, 2);
  ASSERT_EQ(deactivated_, 0);
  ASSERT_EQ(closed_, 0);

  // Dropping all the txns on sess1 should "deactivate" that session
  for (auto& txn : txnsSess1) {
    txn->sendAbort();
  }
  ASSERT_EQ(activated_, 2);
  ASSERT_EQ(deactivated_, 1);
  ASSERT_EQ(closed_, 0);

  // Just for kicks, ensure that no session is scheduled to be closed
  // right now
  evb_.loop();
  ASSERT_EQ(closed_, 0);

  // Decrease the session pool limit to 0 and purge the idle sessions.
  // This should close sess1 (the only idle session at this time)
  p.setMaxIdleSessions(0);
  evb_.loop();
  ASSERT_EQ(closed_, 1);

  // Drop 1 txn from the completely full sess2, then get another txn. This
  // should succeed. This shouldn't "deactivate" sess2.
  txnsSess2.back()->sendAbort();
  txnsSess2.pop_back();
  ASSERT_EQ(deactivated_, 1);
  txnsSess2.push_back(CHECK_NOTNULL(p.getTransaction(this)));

  // Set the max pooled sessions to zero and drop all txns from
  // sess2. The session should be automatically closed when it hits
  // idle (i.e., no transactions open)
  for (auto& txn : txnsSess2) {
    ASSERT_EQ(closed_, 1);
    txn->sendAbort();
  }
  ASSERT_EQ(activated_, 2);
  ASSERT_EQ(deactivated_, 2);
  ASSERT_EQ(closed_, 2);
  evb_.loop();
}

TEST_F(SessionPoolFixture, OutstandingWrites) {
  auto codec = makeSerialCodec();
  EXPECT_CALL(*codec, generateHeader(_, _, _, _, _, _))
      .WillOnce(Invoke([](folly::IOBufQueue& writeBuf,
                          HTTPCodec::StreamID /*id*/,
                          const HTTPMessage& /*msg*/,
                          bool /*eom*/,
                          HTTPHeaderSize* size,
                          folly::Optional<HTTPHeaders>) {
        writeBuf.append("somedata");
        if (size) {
          size->uncompressed = 8;
        }
      }));
  auto sess = makeSession(std::move(codec));

  SessionPool p(this, 1);
  p.putSession(sess);
  ASSERT_FALSE(attached_);
  ASSERT_FALSE(sess->isClosing());
  auto txn = CHECK_NOTNULL(p.getTransaction(this));
  ASSERT_TRUE(attached_);
  txn->sendHeaders(HTTPMessage());
  txn->sendAbort();
  // Because the session has outstanding writes and is not parallel, the
  // session is not reusable, and so it will be drained and closed.
  ASSERT_TRUE(sess->isClosing());

  evb_.loop();
  // The session's writes are now drained
  ASSERT_FALSE(attached_);
  ASSERT_EQ(deactivated_, 1);
  ASSERT_EQ(closed_, 1);
}

TEST_F(SessionPoolFixture, OutstandingTransaction) {
  // Similar to the previous test, but even stricter. When the pool is
  // reset, there are outstanding transactions. So, the pool should stay
  // open longer to let the transaction finish.
  auto sess = makeSerialSession();
  HTTPTransaction* txn = nullptr;
  {
    SessionPool p(this, 1);
    p.putSession(sess);
    ASSERT_FALSE(attached_);
    txn = p.getTransaction(this);
    CHECK_NOTNULL(txn);
    ASSERT_TRUE(attached_);
    ASSERT_EQ(deactivated_, 0);
    ASSERT_EQ(closed_, 0);
    ASSERT_EQ(p.getNumSessions(), 1);
  }
  // Destroying the SessionPool starts draining all the sessions.
  // We get the stats for the deactivation and closing early since later
  // there is no guarantee the stats object will be around.
  ASSERT_EQ(deactivated_, 1);
  ASSERT_EQ(closed_, 1);

  // Dropping the transaction will let the session close
  ASSERT_TRUE(attached_);
  txn->sendAbort();
  ASSERT_FALSE(attached_);
}

TEST_F(SessionPoolFixture, DroppedRequestNotPooled) {
  // Let the session pool have 10 max
  SessionPool p(this, 10, std::chrono::seconds(4));
  auto codec = makeSerialCodec();
  // In this test, the codec starts off not busy, but then we do some
  // processing on it, and it should remain busy for the remainder of the
  // test since the response never arrives
  bool shouldBeBusy = false;
  EXPECT_CALL(*codec, isBusy()).WillRepeatedly(Invoke([&]() {
    return shouldBeBusy;
  }));
  auto sess = makeSession(std::move(codec));
  p.putSession(sess);
  auto txn1 = p.getTransaction(this);
  ASSERT_EQ(p.getNumIdleSessions(), 0);
  ASSERT_EQ(p.getNumActiveSessions(), 1);
  ASSERT_EQ(p.getNumSessions(), 1);

  HTTPMessage req;
  req.setMethod("GET");
  req.setURL<string>("/");
  req.setHTTPVersion(1, 1);
  txn1->sendHeaders(req);
  txn1->sendEOM();
  shouldBeBusy = true;
  ASSERT_TRUE(attached_);
  evb_.loop();
  // Writes have now succeeded. We should still be attached since we
  // terminated the loop before the timeouts fired
  CHECK(!timeout_);
  ASSERT_TRUE(attached_);

  // Now drop the txn before the response comes back
  ASSERT_EQ(p.getNumIdleSessions(), 0);
  ASSERT_EQ(p.getNumActiveSessions(), 1);
  ASSERT_EQ(p.getNumSessions(), 1);
  txn1->sendAbort();
  // The drop should have closed the session since the codec was busy when
  // the transaction count went to zero

  // We should fail to get another txn out of the pool
  ASSERT_EQ(p.getNumIdleSessions(), 0);
  ASSERT_EQ(p.getNumActiveSessions(), 0);
  ASSERT_EQ(p.getNumSessions(), 0);
  ASSERT_TRUE(nullptr == p.getTransaction(this));
  evb_.loop();
}

TEST_F(SessionPoolFixture, InsertIntoZeroSizePool) {
  // Let the session pool have 0 max sessions
  SessionPool p(this, 0, std::chrono::seconds(4));
  p.putSession(makeSerialSession());
  CHECK(nullptr == p.getTransaction(this));
  evb_.loop();
}

TEST_F(SessionPoolFixture, DrainSessionLater) {
  // Mark a session in the pool to drain, then make sure the pool works.
  SessionPool p(this, 10, std::chrono::seconds(4));
  auto session = makeParallelSession();
  p.putSession(session);

  auto txn1 = CHECK_NOTNULL(p.getTransaction(this));
  auto txn2 = CHECK_NOTNULL(p.getTransaction(this));
  session->drain();
  ASSERT_EQ(p.getNumSessions(), 1); // don't detect until getTransaction()
  CHECK(nullptr == p.getTransaction(this));
  ASSERT_EQ(p.getNumSessions(), 0);
  // now let the session be destroyed
  txn1->sendAbort();
  txn2->sendAbort();
}

TEST_F(SessionPoolFixture, InsertDrainedSession) {
  // Put a draining session into the pool. Make sure the pool ignores it.
  auto session = makeParallelSession();
  CHECK_NOTNULL(session->newTransaction(this));
  session->drain();

  auto cm = wangle::ConnectionManager::makeUnique(
      &evb_, std::chrono::milliseconds(1), nullptr);
  SessionPool p(this, 10, std::chrono::seconds(4), std::chrono::seconds(0));
  cm->addConnection(session);
  p.putSession(session);
  ASSERT_EQ(p.getNumSessions(), 0);
  CHECK(nullptr == p.getTransaction(this));

  // this session needs to be owned by the cm
  cm->dropAllConnections();
  EXPECT_EQ(numSessions_, 0);
}

TEST_F(SessionPoolFixture, CloseNotReusable) {
  // Make sure if a connection becomes not reusable, that it is removed
  // from the pool after the txn completes
  SessionPool p(this, 10, std::chrono::seconds(4));

  // Codec expectations
  bool reusable = true;
  auto codec = std::make_unique<NiceMock<MockHTTPCodec>>();
  EXPECT_CALL(*codec, getTransportDirection())
      .WillRepeatedly(Return(TransportDirection::UPSTREAM));
  EXPECT_CALL(*codec, createStream()).WillOnce(Return(1));
  EXPECT_CALL(*codec, isReusable()).WillRepeatedly(ReturnPointee(&reusable));
  EXPECT_CALL(*codec, supportsParallelRequests()).WillRepeatedly(Return(false));
  EXPECT_CALL(*codec, getProtocol())
      .WillRepeatedly(Return(CodecProtocol::HTTP_2));

  p.putSession(makeSession(std::move(codec)));
  ASSERT_EQ(p.getNumSessions(), 1);
  auto txn = CHECK_NOTNULL(p.getTransaction(this));
  reusable = false; // Mark the session as not reusable, e.g. if it got
                    // Connection: close
  txn->sendAbort();
  ASSERT_EQ(p.getNumSessions(), 0); // Non-reusable conn should be dropped
}

TEST_F(SessionPoolFixture, InsertOldSession) {
  // Put a session into the pool that is over max-age. Make sure the
  // pool ignores it.
  auto session = makeParallelSession();

  SessionPool p(
      this, 10, std::chrono::seconds(4), std::chrono::milliseconds(50));
  usleep(70000); // exceeds max jitter (currently 65ms)
  p.putSession(session);
  ASSERT_EQ(p.getNumSessions(), 0);
  CHECK(nullptr == p.getTransaction(this));
}

TEST_F(SessionPoolFixture, IdleTmeout) {
  // Put a session into the pool, let it timeout and ask for a new txn
  // pool ignores it.
  auto session = makeParallelSession();

  SessionPool p(
      this, 10, std::chrono::milliseconds(250), std::chrono::seconds(4));
  p.putSession(session);
  ASSERT_TRUE(p.getNumSessions() == 1);
  auto txn = CHECK_NOTNULL(p.getTransaction(this));
  txn->sendAbort();
  /* sleep override */ usleep(260000); // > 250ms
  CHECK(nullptr == p.getTransaction(this));
  ASSERT_EQ(p.getNumSessions(), 0);
}

TEST_F(SessionPoolFixture, AgeOut) {
  // Put a session into the pool, let it age out and ask for a new txn
  // pool ignores it.
  auto session = makeParallelSession();

  SessionPool p(
      this, 10, std::chrono::seconds(4), std::chrono::milliseconds(250));
  p.putSession(session);
  // possible flake if this process takes a dirtnap for >= 175ms
  ASSERT_TRUE(p.getNumSessions() == 1);
  auto txn = CHECK_NOTNULL(p.getTransaction(this));
  txn->sendAbort();
  usleep(350000); // over max jitter (325 ms)
  CHECK(nullptr == p.getTransaction(this));
  ASSERT_EQ(p.getNumSessions(), 0);
}

TEST_F(SessionPoolFixture, DrainOnShutdown) {
  // Verify that the session is drained if the socket is closing/closed
  auto session = makeParallelSession();

  SessionPool p(this, 10, std::chrono::milliseconds(4));
  p.putSession(session);
  ASSERT_EQ(p.getNumSessions(), 1);
  ASSERT_EQ(p.getNumIdleSessions(), 1);
  ASSERT_EQ(p.getNumActiveSessions(), 0);

  auto txn = p.getTransaction(this);
  ASSERT_TRUE(txn != nullptr);
  ASSERT_EQ(p.getNumSessions(), 1);
  ASSERT_EQ(p.getNumIdleSessions(), 0);
  ASSERT_EQ(p.getNumActiveSessions(), 1);
  ASSERT_EQ(p.getNumActiveNonFullSessions(), 1);
  ASSERT_EQ(session->getNumOutgoingStreams(), 1);

  // Manually close the socket
  session->getTransport()->closeNow();
  ASSERT_EQ(session->getConnectionCloseReason(),
            ConnectionCloseReason::kMAX_REASON);

  // New transaction creation fails and drains the session
  ASSERT_TRUE(p.getTransaction(this) == nullptr);
  ASSERT_EQ(session->getConnectionCloseReason(),
            ConnectionCloseReason::SHUTDOWN);
  ASSERT_EQ(p.getNumSessions(), 0);
  ASSERT_EQ(session->getNumOutgoingStreams(), 1);

  // Now let the session be destroyed
  txn->sendAbort();
}

class TestIdleController : public ServerIdleSessionController {
 public:
  // expose this method as public for tests.
  SessionPool* popBestIdlePool() {
    return ServerIdleSessionController::popBestIdlePool();
  }
};

TEST_F(SessionPoolFixture, MoveIdleSessionBetweenThreadsTest) {
  TestIdleController ctrl;
  HTTPUpstreamSession* session = nullptr;

  folly::Baton<> t1InitBaton, t2InitBaton, transferBaton;
  // Create two threads, each looping on their own event base
  std::thread t1([&] {
    folly::EventBaseManager::get()->setEventBase(&evb_, false);
    SessionPool p1(this,
                   10,
                   std::chrono::seconds(30),
                   std::chrono::milliseconds(0),
                   nullptr,
                   &ctrl);
    // Put an (idle) session on p1.
    session = makeParallelSession();
    p1.putSession(session);
    t1InitBaton.post();
    evb_.loopForever();
  });

  // Wait for t1 to start before starting t2
  // to ensure it is picked by popBestIdlePool()
  t1InitBaton.wait();

  folly::EventBase evb2;
  std::thread t2([&] {
    folly::EventBaseManager::get()->setEventBase(&evb2, false);
    SessionPool p2(this,
                   10,
                   std::chrono::seconds(30),
                   std::chrono::milliseconds(0),
                   nullptr,
                   &ctrl);
    t2InitBaton.post();
    evb2.loopForever();
  });

  t2InitBaton.wait();
  // Simulate thread2 asking thread1 for an idle session
  evb2.runInEventBaseThread([&] {
    ctrl.getIdleSession().via(&evb2).thenValue(
        [&](HTTPSessionBase* idleSession) {
          ASSERT_EQ(idleSession, session);
          // Not re-attaching it to thread2 so ctrl will be empty
          transferBaton.post();
        });
  });
  transferBaton.wait();
  EXPECT_EQ(ctrl.popBestIdlePool(), nullptr);

  session->drain();
  evb_.terminateLoopSoon();
  evb2.terminateLoopSoon();
  t1.join();
  t2.join();
}

TEST_F(SessionPoolFixture, PurgeAddedSessionTest) {
  TestIdleController ctrl;
  HTTPUpstreamSession* session = makeParallelSession();
  SessionPool p1(this,
                 1,
                 /*idleTimeout=*/std::chrono::milliseconds(0),
                 std::chrono::milliseconds(0),
                 nullptr,
                 &ctrl);

  // This is about to purge session immediately and should not crash.
  p1.putSession(session);
}

TEST_F(SessionPoolFixture, ServerIdleSessionControllerTest) {
  TestIdleController ctrl;
  SessionPool p1, p2;
  auto s1 = makeParallelSession(), s2 = makeParallelSession(),
       s3 = makeParallelSession();
  EXPECT_EQ(ctrl.popBestIdlePool(), nullptr);

  ctrl.addIdleSession(s1, &p1);
  EXPECT_EQ(ctrl.popBestIdlePool(), &p1);
  EXPECT_EQ(ctrl.popBestIdlePool(), nullptr);

  ctrl.addIdleSession(s3, &p2);
  ctrl.addIdleSession(s1, &p1);
  EXPECT_EQ(ctrl.popBestIdlePool(), &p2);
  EXPECT_EQ(ctrl.popBestIdlePool(), &p1);
  EXPECT_EQ(ctrl.popBestIdlePool(), nullptr);

  ctrl.addIdleSession(s1, &p1);
  ctrl.addIdleSession(s2, &p1);
  ctrl.removeIdleSession(s1);
  ctrl.addIdleSession(s3, &p2);
  EXPECT_EQ(ctrl.popBestIdlePool(), &p1);
  EXPECT_EQ(ctrl.popBestIdlePool(), &p2);
  EXPECT_EQ(ctrl.popBestIdlePool(), nullptr);

  s1->drain();
  s2->drain();
  s3->drain();
}

TEST_F(SessionPoolFixture, WritePausedSessionNotMarkedAsIdle) {
  auto codec = makeParallelCodec();
  EXPECT_CALL(*codec, generateHeader(_, _, _, _, _, _))
      .WillOnce(Invoke([](folly::IOBufQueue& writeBuf,
                          HTTPCodec::StreamID /*id*/,
                          const HTTPMessage& /*msg*/,
                          bool /*eom*/,
                          HTTPHeaderSize* size,
                          folly::Optional<HTTPHeaders>) {
        writeBuf.append("somedata");
        if (size) {
          size->uncompressed = 8;
        }
      }));
  auto session = makeSession(std::move(codec));

  TestIdleController ctrl;
  SessionPool p1(this,
                 10,
                 std::chrono::seconds(30),
                 std::chrono::milliseconds(0),
                 nullptr,
                 &ctrl);
  p1.putSession(session);

  auto txn = p1.getTransaction(this);
  ASSERT_NE(txn, (HTTPTransaction*)nullptr);
  TestAsyncTransport* transport =
      session->getTransport()->getUnderlyingTransport<TestAsyncTransport>();
  transport->pauseWrites();
  HTTPMessage req;
  req.setMethod("GET");
  req.setURL<string>("/");
  req.setHTTPVersion(1, 1);
  txn->sendHeaders(req);
  evb_.loopOnce();
  txn->sendAbort();

  transport->resumeWrites();
  evb_.loopOnce();

  // Session should not be marked as idle.
  EXPECT_EQ(ctrl.popBestIdlePool(), nullptr);
  EXPECT_EQ(p1.getNumSessions(), 1);
  EXPECT_EQ(p1.getNumIdleSessions(), 0);
  EXPECT_EQ(p1.getNumActiveSessions(), 1);
  EXPECT_EQ(p1.getNumActiveNonFullSessions(), 1);
}

TEST_F(SessionPoolFixture, ThreadIdleSessionControllerLimitsTotalIdle) {
  ThreadIdleSessionController controller(3);
  SessionPool p1(this,
                 2,
                 std::chrono::milliseconds(50),
                 std::chrono::milliseconds(50),
                 &controller);
  SessionPool p2(this,
                 2,
                 std::chrono::milliseconds(50),
                 std::chrono::milliseconds(50),
                 &controller);

  // Add two sessions on each pool.
  p1.putSession(makeSerialSession());
  p1.putSession(makeSerialSession());

  p2.putSession(makeSerialSession());
  p2.putSession(makeSerialSession());

  // Validate ThreadIdleSessionController limited it to 3 at most.
  EXPECT_EQ(controller.getTotalIdleSessions(), 3);

  auto txn = p1.getTransaction(this);
  ASSERT_TRUE(txn != nullptr);

  // Drop the transaction. All transactions on the sessions in the pool
  // must be completed before the pool can be destroyed
  txn->sendAbort();

  evb_.loop();
  ASSERT_EQ(closed_, 2);
  EXPECT_EQ(controller.getTotalIdleSessions(), 2);
  EXPECT_EQ(p1.getNumIdleSessions(), 0);
  EXPECT_EQ(p2.getNumIdleSessions(), 2);
}

TEST_F(SessionPoolFixture, ThreadIdleSessionControllerTrackSessionPoolChanges) {
  ThreadIdleSessionController controller(5);
  SessionPool p1(this,
                 2,
                 std::chrono::milliseconds(50),
                 std::chrono::milliseconds(50),
                 &controller);
  SessionPool p2(this,
                 2,
                 std::chrono::milliseconds(50),
                 std::chrono::milliseconds(50),
                 &controller);

  // Add two sessions on each pool.
  p1.putSession(makeSerialSession());
  p1.putSession(makeSerialSession());

  p2.putSession(makeSerialSession());
  p2.putSession(makeSerialSession());

  // Validate ThreadIdleSessionController sees all sessions.
  EXPECT_EQ(controller.getTotalIdleSessions(), 4);

  // Drop a number of sessions.
  p1.setMaxIdleSessions(1);
  p2.setMaxIdleSessions(1);

  EXPECT_EQ(controller.getTotalIdleSessions(), 2);
  EXPECT_EQ(p1.getNumIdleSessions(), 1);
  EXPECT_EQ(p2.getNumIdleSessions(), 1);
}

TEST_F(SessionPoolFixture, DescribeWithNullTransport) {
  MockHQSession session;
  MockSessionHolderCallback cb;
  SessionHolder sessionHolder(&session, &cb);

  // This should not crash.
  LOG(INFO) << sessionHolder;
  // This is to appease mock session destructor.
  session.setInfoCallback(nullptr);
}

// So we can have -v work
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
