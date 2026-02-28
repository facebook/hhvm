/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/SocketAddress.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GTest.h>
#include <proxygen/facebook/lib/http/sink/HTTPConnectSink.h>

#include <memory>

#include "proxygen/facebook/revproxy/test/MockHTTPTransactionHandler.h"

namespace proxygen {
using namespace testing;
using namespace folly::test;

// Subclass which tracks deletion
class TestHTTPConnectSink : public HTTPConnectSink {
 public:
  using HTTPConnectSink::HTTPConnectSink;
  ~TestHTTPConnectSink() override {
    if (isDeleted != nullptr) {
      *isDeleted = true;
    }
  }
  std::shared_ptr<bool> isDeleted;
};

class HTTPConnectSinkTest : public Test {
 public:
  void SetUp() override {
    evb_ = folly::EventBaseManager::get()->getEventBase();
    mockSocket_ = new MockAsyncTransport();
    mockHandler_ = new MockHTTPTransactionHandler();
    sink_ = std::make_unique<TestHTTPConnectSink>(
        folly::AsyncTransport::UniquePtr(mockSocket_), mockHandler_);
    sinkDeleted_ = std::make_shared<bool>(false);
    sink_->isDeleted = sinkDeleted_;
  }

  void TearDown() override {
    sink_.reset();
    delete mockHandler_;
    // Don't delete mockSocket_ because it's owned by sink_
  }

 protected:
  folly::EventBase *evb_;
  MockAsyncTransport *mockSocket_;
  MockHTTPTransactionHandler *mockHandler_;

  std::shared_ptr<bool> sinkDeleted_;

  std::unique_ptr<TestHTTPConnectSink> sink_;
};

using WriteCB = folly::AsyncTransport::WriteCallback;

TEST_F(HTTPConnectSinkTest, test_egress_backpressure) {
  // Test that egress backpressure is propagated to the handler

  // Mock underlying write to just store the callbacks for later
  std::vector<WriteCB *> writeCallbacks;
  EXPECT_CALL(*mockSocket_, writeChain)
      .WillRepeatedly(Invoke([&writeCallbacks](WriteCB *cb, auto &&, auto &&) {
        writeCallbacks.push_back(cb);
      }));

  // Send bytes, which should trigger egress pause
  EXPECT_CALL(*mockHandler_, onEgressPaused);
  sink_->sendBody(folly::IOBuf::copyBuffer("Is anybody there?"));
  EXPECT_TRUE(sink_->isEgressPaused());

  // Write success to just one callback, which should resume egress
  EXPECT_CALL(*mockHandler_, onEgressResumed).WillOnce(Return());
  writeCallbacks[0]->writeSuccess();
  EXPECT_FALSE(sink_->isEgressPaused());

  // Send another body, which should trigger egress pausing again
  EXPECT_CALL(*mockHandler_, onEgressPaused).WillOnce(Return());
  sink_->sendBody(folly::IOBuf::copyBuffer("Oh, it's just me?"));

  // Write success to both remaining callbacks, which should resume egress again
  EXPECT_CALL(*mockHandler_, onEgressResumed).WillOnce(Return());
  writeCallbacks[1]->writeSuccess();

  // Re-mock the write callback to just call writeSuccess immediately
  EXPECT_CALL(*mockSocket_, writeChain)
      .WillRepeatedly(
          Invoke([](WriteCB *cb, auto &&, auto &&) { cb->writeSuccess(); }));

  // Egress shouldn't pause and resume, which we're checking w/ WillOnce
  sink_->sendBody(folly::IOBuf::copyBuffer("We made it!"));
}

TEST_F(HTTPConnectSinkTest, test_ingress_control) {
  folly::AsyncTransport::ReadCallback *read_cb = sink_.get();
  EXPECT_CALL(*mockSocket_, setReadCB).WillRepeatedly(Invoke([&](auto cb) {
    read_cb = cb;
  }));
  EXPECT_CALL(*mockSocket_, getReadCallback).WillRepeatedly(Invoke([&]() {
    return read_cb;
  }));
  EXPECT_FALSE(sink_->isIngressPaused());
  sink_->pauseIngress();
  EXPECT_EQ(read_cb, nullptr);
  EXPECT_TRUE(sink_->isIngressPaused());
  sink_->resumeIngress();
  EXPECT_EQ(read_cb, sink_.get());
}

TEST_F(HTTPConnectSinkTest, test_bytes_read) {
  // Test that read bytes are propagated to the handler
  std::string bytes = "Shoelace is a cute dog!";
  size_t num_bytes = bytes.size();

  // get an IOBuf to write to
  void *buf = nullptr;
  size_t siz = 0;
  sink_->getReadBuffer(&buf, &siz);
  memcpy(buf, bytes.c_str(), num_bytes);

  EXPECT_CALL(*mockHandler_, onBody)
      .WillOnce(Invoke([&bytes](auto &&written_bytes) {
        EXPECT_EQ(written_bytes->moveToFbString().toStdString(), bytes);
      }));
  sink_->readDataAvailable(num_bytes);
}

// Run all callbacks on evb until the specified time has passed
void sleep_evb(folly::EventBase *evb, std::chrono::milliseconds timeout) {
  bool done = false;
  evb->scheduleAt([&done]() { done = true; }, evb->now() + timeout);
  while (!done) {
    evb->loopOnce();
  }
};

TEST_F(HTTPConnectSinkTest, test_idle_timeout) {
  EXPECT_CALL(*mockSocket_, getEventBase).WillRepeatedly(Invoke([&]() {
    return evb_;
  }));
  sink_->setIdleTimeout(std::chrono::milliseconds(10));

  // Wait 8ms; the timeout shouldn't have expired
  sleep_evb(evb_, std::chrono::milliseconds(8));

  // Test that read bytes are propagated to the handler
  std::string bytes = "Shoelace is a cute dog!";
  size_t num_bytes = bytes.size();

  // get an IOBuf to write to
  void *buf = nullptr;
  size_t siz = 0;
  sink_->getReadBuffer(&buf, &siz);
  memcpy(buf, bytes.c_str(), num_bytes);

  EXPECT_CALL(*mockHandler_, onBody)
      .WillOnce(Invoke([&bytes](auto &&written_bytes) {
        EXPECT_EQ(written_bytes->moveToFbString().toStdString(), bytes);
      }));
  sink_->readDataAvailable(num_bytes);

  // Wait another 8ms; the timeout was reset, so this still shouldn't expire
  sleep_evb(evb_, std::chrono::milliseconds(8));

  // Ok, now, expect callbacks which happen during timeout expiration
  EXPECT_CALL(*mockHandler_, onError).WillOnce(Return());
  EXPECT_CALL(*mockHandler_, detachTransaction).WillOnce(Return());
  EXPECT_CALL(*mockSocket_, closeNow).WillOnce(Return());
  sleep_evb(evb_, std::chrono::milliseconds(8));
}

TEST_F(HTTPConnectSinkTest, test_delayed_destruction) {
  // Mock underlying write to just store the callbacks for later
  std::vector<WriteCB *> writeCallbacks;
  EXPECT_CALL(*mockSocket_, writeChain)
      .WillRepeatedly(Invoke([&writeCallbacks](WriteCB *cb, auto &&, auto &&) {
        writeCallbacks.push_back(cb);
      }));

  sink_->sendBody(folly::IOBuf::copyBuffer("hello world"));

  EXPECT_CALL(*mockSocket_, closeWithReset).WillOnce(Return());
  sink_->detachAndAbortIfIncomplete(std::move(sink_));
  // There's an outstanding write, so the sink should not be deleted yet
  EXPECT_EQ(*sinkDeleted_, false);

  // The sink should be destroyed after the final write
  writeCallbacks[0]->writeSuccess();
  EXPECT_EQ(*sinkDeleted_, true);
}

TEST_F(HTTPConnectSinkTest, test_destruction_on_abort) {
  EXPECT_CALL(*mockSocket_, writeChain)
      .WillRepeatedly(
          Invoke([](WriteCB *cb, auto &&, auto &&) { cb->writeSuccess(); }));
  sink_->sendBody(folly::IOBuf::copyBuffer("hello world"));

  EXPECT_CALL(*mockSocket_, closeWithReset).WillOnce(Return());
  sink_->detachAndAbortIfIncomplete(std::move(sink_));

  // The sink should be destroyed
  EXPECT_EQ(*sinkDeleted_, true);
}

TEST_F(HTTPConnectSinkTest, test_ingress_eom) {
  // expect handler onEOM
  EXPECT_CALL(*mockHandler_, onEOM).WillOnce(Return());
  sink_->readEOF();

  // now that we've read an upstream EOM, if we send a downstream EOM,
  // the handler will detach
  EXPECT_CALL(*mockHandler_, detachTransaction).WillOnce(Return());
  EXPECT_CALL(*mockSocket_, shutdownWrite).WillOnce(Return());
  sink_->sendEOM();
  EXPECT_TRUE(sink_->isEgressEOMSeen());
}

TEST_F(HTTPConnectSinkTest, test_send_abort) {
  EXPECT_CALL(*mockHandler_, detachTransaction).WillOnce(Return());
  EXPECT_CALL(*mockSocket_, closeWithReset).WillOnce(Return());
  sink_->sendAbort();
}

TEST_F(HTTPConnectSinkTest, test_read_error) {
  EXPECT_CALL(*mockHandler_, onError).WillOnce(Return());
  EXPECT_CALL(*mockHandler_, detachTransaction).WillOnce(Return());
  sink_->readErr(folly::AsyncSocketException(
      folly::AsyncSocketException::AsyncSocketExceptionType::UNKNOWN, "test"));
}

TEST_F(HTTPConnectSinkTest, test_read_error_with_abort) {
  EXPECT_CALL(*mockHandler_, onError).WillOnce(Invoke([&](auto &&) {
    sink_->detachAndAbortIfIncomplete(std::move(sink_));
  }));
  EXPECT_CALL(*mockHandler_, detachTransaction).Times(0);
  sink_->readErr(folly::AsyncSocketException(
      folly::AsyncSocketException::AsyncSocketExceptionType::UNKNOWN, "test"));
}

TEST_F(HTTPConnectSinkTest, test_write_error) {
  EXPECT_CALL(*mockSocket_, writeChain)
      .WillRepeatedly(Invoke([](WriteCB *cb, auto &&, auto &&) {
        cb->writeErr(
            0,
            folly::AsyncSocketException(
                folly::AsyncSocketException::AsyncSocketExceptionType::UNKNOWN,
                "test"));
      }));
  EXPECT_CALL(*mockHandler_, onError).WillOnce(Return());
  EXPECT_CALL(*mockHandler_, detachTransaction).WillOnce(Return());
  sink_->sendBody(folly::IOBuf::copyBuffer("hello world"));
}

TEST_F(HTTPConnectSinkTest, test_write_error_with_abort) {
  EXPECT_CALL(*mockSocket_, writeChain)
      .WillRepeatedly(Invoke([](WriteCB *cb, auto &&, auto &&) {
        cb->writeErr(
            0,
            folly::AsyncSocketException(
                folly::AsyncSocketException::AsyncSocketExceptionType::UNKNOWN,
                "test"));
      }));
  EXPECT_CALL(*mockHandler_, onError).WillOnce(Invoke([&](auto &&) {
    sink_->detachAndAbortIfIncomplete(std::move(sink_));
  }));
  EXPECT_CALL(*mockHandler_, detachTransaction).Times(0);
  sink_->sendBody(folly::IOBuf::copyBuffer("hello world"));
}

} // namespace proxygen
