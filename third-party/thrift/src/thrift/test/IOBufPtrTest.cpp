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

#include <condition_variable>
#include <mutex>
#include <thread>

#include <glog/logging.h>

#include <folly/Memory.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/test/gen-cpp2/IOBufPtrTestService.h>

namespace thrift {
namespace test {
namespace iobufptr {

class IOBufPtrTestService
    : public apache::thrift::ServiceHandler<IOBufPtrTestService> {
 public:
  void async_tm_combine(
      std::unique_ptr<
          apache::thrift::HandlerCallback<std::unique_ptr<IOBufPtr>>> callback,
      std::unique_ptr<Request> req) override;
};

void IOBufPtrTestService::async_tm_combine(
    std::unique_ptr<apache::thrift::HandlerCallback<std::unique_ptr<IOBufPtr>>>
        callback,
    std::unique_ptr<Request> req) {
  folly::IOBufQueue queue;
  queue.append("(");
  queue.append(std::move(*req->one()));
  queue.append(")+(");
  queue.append(std::move(*req->two()));
  queue.append(")+(");
  queue.append(req->three()->clone());
  queue.append(")");
  callback->result(std::make_unique<IOBufPtr>(queue.move()));
}

class IOBufPtrTest : public ::testing::Test {
 protected:
  IOBufPtrTest();
  ~IOBufPtrTest() override;

  const folly::SocketAddress& getServerAddress() const {
    return server_.getAddress();
  }

  folly::EventBase* getEventBase() const {
    return server_.getEventBaseManager()->getEventBase();
  }

  IOBufPtrTestServiceAsyncClient* client() const { return client_.get(); }

 private:
  void serverThreadLoop();

  std::thread serverThread_;
  std::mutex mutex_;
  std::condition_variable startedCond_;
  folly::EventBase* serverEventBase_;
  apache::thrift::ThriftServer server_;
  std::unique_ptr<IOBufPtrTestServiceAsyncClient> client_;
};

IOBufPtrTest::IOBufPtrTest() : serverEventBase_(nullptr) {
  serverThread_ = std::thread([this] { this->serverThreadLoop(); });
  std::unique_lock<std::mutex> lock(mutex_);
  while (!serverEventBase_) {
    startedCond_.wait(lock);
  }

  auto socket =
      folly::AsyncSocket::newSocket(getEventBase(), getServerAddress());

  auto channel =
      apache::thrift::HeaderClientChannel::newChannel(std::move(socket));
  client_ =
      std::make_unique<IOBufPtrTestServiceAsyncClient>(std::move(channel));
}

IOBufPtrTest::~IOBufPtrTest() {
  serverEventBase_->terminateLoopSoon();
  serverThread_.join();
}

void IOBufPtrTest::serverThreadLoop() {
  server_.setPort(0); // pick one
  server_.setInterface(std::make_unique<IOBufPtrTestService>());
  server_.setup();
  SCOPE_EXIT { server_.cleanUp(); };
  {
    std::unique_lock<std::mutex> lock(mutex_);
    serverEventBase_ = server_.getEventBaseManager()->getEventBase();
    startedCond_.notify_one();
  }
  serverEventBase_->loopForever();
}

TEST_F(IOBufPtrTest, Simple) {
  {
    Request req;
    IOBufPtr resp;
    client()->sync_combine(resp, req);
    EXPECT_EQ("()+(hello)+()", resp->moveToFbString());
  }
  {
    Request req;
    *req.one() = folly::IOBuf::wrapBuffer("meow", 4);
    *req.two() = folly::IOBuf::wrapBuffer("woof", 4);
    EXPECT_TRUE(
        apache::thrift::StringTraits<std::unique_ptr<folly::IOBuf>>::isEqual(
            *req.one(), *req.one()));
    EXPECT_FALSE(
        apache::thrift::StringTraits<std::unique_ptr<folly::IOBuf>>::isEqual(
            *req.one(), *req.two()));
    *req.three() = folly::IOBuf(folly::IOBuf::WRAP_BUFFER, "oink", 4);
    IOBufPtr resp;
    client()->sync_combine(resp, req);
    EXPECT_EQ("(meow)+(woof)+(oink)", resp->moveToFbString());
  }
}

TEST(IOBufPtrUnionTest, Union) {
  Union foo;
  foo.foo_ref() = folly::IOBuf::wrapBufferAsValue("meow", 4);
  Union bar = std::move(foo);
  EXPECT_EQ(bar.foo_ref()->moveToFbString(), "meow");
}

} // namespace iobufptr
} // namespace test
} // namespace thrift
