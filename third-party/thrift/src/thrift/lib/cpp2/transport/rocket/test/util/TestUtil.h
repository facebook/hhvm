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

#include <folly/portability/GFlags.h>
#include <folly/portability/GMock.h>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeServerObserver.h>
#include <thrift/lib/cpp2/transport/core/testutil/MockCallback.h>
#include <thrift/lib/cpp2/transport/core/testutil/TAsyncSocketIntercepted.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/ThriftRocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestServiceMock.h>
#include <thrift/lib/cpp2/transport/util/ConnectionManager.h>

namespace apache {
namespace thrift {

using namespace apache::thrift;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;
using namespace testing;
using namespace testutil::testservice;
using testutil::testservice::Message;

// Event handler to attach to the Thrift server so we know when it is
// ready to serve and also so we can determine the port it is
// listening on.
class TestEventHandler : public server::TServerEventHandler {
 public:
  // This is a callback that is called when the Thrift server has
  // initialized and is ready to serve RPCs.
  void preServe(const folly::SocketAddress* address) override {
    port_ = address->getPort();
    baton_.post();
  }

  int32_t waitForPortAssignment() {
    baton_.wait();
    return port_;
  }

 private:
  folly::Baton<> baton_;
  int32_t port_;
};

class TestSetup : public testing::Test {
 protected:
  virtual std::unique_ptr<ThriftServer> createServer(
      std::shared_ptr<AsyncProcessorFactory>,
      uint16_t& port,
      int maxRequests = 0,
      std::string transport = "rocket");

  RequestChannel::Ptr connectToServer(
      uint16_t port,
      folly::Function<void()> onDetachable = nullptr,
      folly::Function<void(TAsyncSocketIntercepted&)> socketSetup = nullptr);

  void setNumIOThreads(int num) { numIOThreads_ = num; }
  void setNumWorkerThreads(int num) { numWorkerThreads_ = num; }
  void setQueueTimeout(std::chrono::milliseconds timeout) {
    queueTimeout_ = timeout;
  }
  void setIdleTimeout(std::chrono::milliseconds timeout) {
    idleTimeout_ = timeout;
  }
  void setTaskExpireTime(std::chrono::milliseconds time) {
    taskExpireTime_ = time;
  }
  void setStreamExpireTime(std::chrono::milliseconds time) {
    streamExpireTime_ = time;
  }

 protected:
  std::shared_ptr<FakeServerObserver> observer_;

  int numIOThreads_{10};
  int numWorkerThreads_{10};
  std::optional<std::chrono::milliseconds> queueTimeout_;
  std::optional<std::chrono::milliseconds> idleTimeout_;
  std::optional<std::chrono::milliseconds> taskExpireTime_;
  std::optional<std::chrono::milliseconds> streamExpireTime_;

  folly::ScopedEventBaseThread evbThread_;
  folly::ScopedEventBaseThread executor_;
  std::shared_ptr<folly::IOExecutor> ioThread_{
      std::make_shared<folly::ScopedEventBaseThread>()};
};

class SlowWritingSocket : public folly::AsyncSocket {
 public:
  SlowWritingSocket(folly::EventBase* evb, const folly::SocketAddress& address)
      : folly::AsyncSocket(evb, address) {}

  void delayWritingAfterFirstNBytes(size_t nbytes) {
    LOG(INFO) << "delayWritingAfterFirstNBytes";
    ASSERT_TRUE(bufferedWrites_.empty())
        << "Can only be called on socket without buffered writes";
    ASSERT_EQ(
        std::numeric_limits<size_t>::max(),
        bytesRemainingBeforeDelayingWrites_);

    bytesRemainingBeforeDelayingWrites_ = nbytes;
  }

  void flushBufferedWrites() {
    while (!bufferedWrites_.empty()) {
      auto bufferedWrite = std::move(bufferedWrites_.front());
      bufferedWrites_.pop_front();
      folly::AsyncSocket::writeChain(
          bufferedWrite.callback, std::move(bufferedWrite.iobuf));
    }
  }

  void errorOutBufferedWrites(
      folly::Optional<size_t> failRequestWithNBytesWritten) {
    while (!bufferedWrites_.empty()) {
      auto bufferedWrite = std::move(bufferedWrites_.front());
      bufferedWrites_.pop_front();
      bufferedWrite.callback->writeErr(
          failRequestWithNBytesWritten ? *failRequestWithNBytesWritten
                                       : bufferedWrite.bytesWritten,
          folly::AsyncSocketException(
              folly::AsyncSocketException::INTERRUPTED, "Write failed"));
    }
  }

  void writeChain(
      WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    ASSERT_EQ(folly::WriteFlags::NONE, flags) << "Write flags not supported";

    std::unique_ptr<folly::IOBuf> writeNow;
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    queue.append(std::move(buf));
    if (bytesRemainingBeforeDelayingWrites_ != 0) {
      writeNow = queue.splitAtMost(bytesRemainingBeforeDelayingWrites_);
      bytesRemainingBeforeDelayingWrites_ -= writeNow->computeChainDataLength();
    }

    if (!queue.empty()) {
      bufferedWrites_.emplace_back(
          queue.move(),
          callback,
          writeNow ? writeNow->computeChainDataLength() : 0);
      if (writeNow) {
        folly::AsyncSocket::writeChain(nullptr, std::move(writeNow), flags);
      }
    } else if (!writeNow->empty()) {
      folly::AsyncSocket::writeChain(callback, std::move(writeNow), flags);
    }
  }

 private:
  struct BufferedWrite {
    BufferedWrite(
        std::unique_ptr<folly::IOBuf> _fbthrift_iobuf,
        WriteCallback* _callback,
        size_t _bytesWritten)
        : iobuf(std::move(_fbthrift_iobuf)),
          callback(_callback),
          bytesWritten(_bytesWritten) {}

    std::unique_ptr<folly::IOBuf> iobuf;
    WriteCallback* callback;
    size_t bytesWritten;
  };

  std::deque<BufferedWrite> bufferedWrites_;
  size_t bytesRemainingBeforeDelayingWrites_{
      std::numeric_limits<size_t>::max()};
};
} // namespace thrift
} // namespace apache
