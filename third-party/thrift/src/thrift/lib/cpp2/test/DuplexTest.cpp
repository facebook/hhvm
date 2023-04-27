/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/DuplexChannel.h>
#include <thrift/lib/cpp2/async/FutureRequest.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DuplexClient.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DuplexService.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace folly;
using std::shared_ptr;
using std::unique_ptr;

class DuplexClientInterface
    : public apache::thrift::ServiceHandler<DuplexClient> {
 public:
  DuplexClientInterface(int32_t first, int32_t count, bool& success)
      : firstIndex_(first), indexesSeen_(count, false), success_(success) {}

  void async_tm_update(
      unique_ptr<HandlerCallback<int32_t>> callback,
      int32_t currentIndex) override {
    // The order we see the values for currentIndex is not defined, but we do
    // expect to see each possible value once.
    std::lock_guard<std::mutex> g(m_);
    EXPECT_FALSE(indexesSeen_[currentIndex - firstIndex_]);
    indexesSeen_[currentIndex - firstIndex_] = true;
    EventBase* eb = callback->getEventBase();
    callback->result(currentIndex);
    if (std::find(indexesSeen_.begin(), indexesSeen_.end(), false) ==
        indexesSeen_.end()) {
      success_ = true;
      eb->runInEventBaseThread([eb] { eb->terminateLoopSoon(); });
    }
  }

 private:
  int32_t firstIndex_;
  std::vector<bool> indexesSeen_;
  std::mutex m_;
  bool& success_;
};

class Updater {
 public:
  Updater(
      shared_ptr<DuplexClientAsyncClient> client,
      EventBase* eb,
      int32_t startIndex,
      int32_t numUpdates,
      int32_t interval)
      : client_(client),
        eb_(eb),
        startIndex_(startIndex),
        numUpdates_(numUpdates),
        interval_(interval) {}

  void update() {
    int32_t si = startIndex_;
    client_->update(
        [si](ClientReceiveState&& state) {
          EXPECT_FALSE(state.isException());
          int32_t res = DuplexClientAsyncClient::recv_update(state);
          EXPECT_EQ(res, si);
        },
        startIndex_);
    startIndex_++;
    numUpdates_--;
    if (numUpdates_ > 0) {
      Updater updater(*this);
      eb_->tryRunAfterDelay(
          [updater]() mutable { updater.update(); }, interval_);
    }
  }

 private:
  shared_ptr<DuplexClientAsyncClient> client_;
  EventBase* eb_;
  int32_t startIndex_;
  int32_t numUpdates_;
  int32_t interval_;
};

class DuplexServiceInterface
    : public apache::thrift::ServiceHandler<DuplexService> {
  void async_tm_registerForUpdates(
      unique_ptr<HandlerCallback<bool>> callback,
      int32_t startIndex,
      int32_t numUpdates,
      int32_t interval) override {
    auto ctx = callback->getConnectionContext()->getConnectionContext();
    CHECK(ctx != nullptr);
    auto client = ctx->getDuplexClient<DuplexClientAsyncClient>();
    auto eb = callback->getEventBase();
    CHECK(eb != nullptr);
    if (numUpdates > 0) {
      Updater updater(client, eb, startIndex, numUpdates, interval);
      eb->runInEventBaseThread([updater]() mutable { updater.update(); });
    };
    callback->result(true);
  }

  void async_tm_regularMethod(
      unique_ptr<HandlerCallback<int32_t>> callback, int32_t val) override {
    callback->result(val * 2);
  }
};

TEST(Duplex, DuplexTest) {
  enum { START = 1, COUNT = 10, INTERVAL = 5 };
  apache::thrift::TestThriftServerFactory<DuplexServiceInterface> factory;
  factory.duplex(true);
  ScopedServerThread sst(factory.create());
  EventBase base;

  std::shared_ptr<AsyncSocket> socket(
      AsyncSocket::newSocket(&base, *sst.getAddress()));

  auto duplexChannel =
      std::make_shared<DuplexChannel>(DuplexChannel::Who::CLIENT, socket);
  DuplexServiceAsyncClient client(duplexChannel->getClientChannel());

  bool success = false;
  ThriftServer clients_server(duplexChannel->getServerChannel());
  clients_server.setInterface(
      std::make_shared<DuplexClientInterface>(START, COUNT, success));
  clients_server.serve();

  client.registerForUpdates(
      [](ClientReceiveState&& state) {
        EXPECT_FALSE(state.isException());
        bool res = DuplexServiceAsyncClient::recv_registerForUpdates(state);
        EXPECT_TRUE(res);
      },
      START,
      COUNT,
      INTERVAL);

  // fail on time out
  base.tryRunAfterDelay([] { ADD_FAILURE(); }, 5000);

  base.loopForever();

  EXPECT_TRUE(success);
}

void testNonHeader(CLIENT_TYPE type) {
  apache::thrift::TestThriftServerFactory<DuplexServiceInterface> factory;
  factory.duplex(true);
  ScopedServerThread sst(factory.create());
  EventBase base;

  std::shared_ptr<AsyncSocket> socket(
      AsyncSocket::newSocket(&base, *sst.getAddress()));

  HeaderClientChannel::Options options;
  options.setClientType(type);
  auto duplexChannel = std::make_shared<DuplexChannel>(
      DuplexChannel::Who::CLIENT, socket, std::move(options));

  DuplexServiceAsyncClient client(duplexChannel->getClientChannel());

  int res = client.sync_regularMethod(5);
  EXPECT_EQ(10, res);
}

TEST(Duplex, TestFramed) {
  testNonHeader(THRIFT_FRAMED_DEPRECATED);
}

TEST(Duplex, TestUnframed) {
  testNonHeader(THRIFT_UNFRAMED_DEPRECATED);
}
