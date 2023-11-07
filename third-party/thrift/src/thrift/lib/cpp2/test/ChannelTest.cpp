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

#include <folly/portability/GTest.h>

#include <folly/SocketAddress.h>
#include <folly/experimental/TestUtil.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/SocketPair.h>
#include <folly/io/async/test/TestSSLServer.h>
#include <folly/lang/Bits.h>
#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp2/async/Cpp2Channel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderServerChannel.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>

using namespace apache::thrift;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;
using folly::IOBuf;
using folly::IOBufQueue;
using folly::test::find_resource;
using std::make_unique;
using std::unique_ptr;

unique_ptr<IOBuf> makeTestBufImpl(size_t len) {
  unique_ptr<IOBuf> buf = IOBuf::create(len);
  buf->IOBuf::append(len);
  memset(buf->writableData(), char(0x80), len);
  return LegacySerializedRequest(
             T_COMPACT_PROTOCOL, "test", SerializedRequest(std::move(buf)))
      .buffer;
}

unique_ptr<IOBuf> makeTestBuf(size_t len) {
  for (auto requestLen = len; requestLen > 0; --requestLen) {
    auto buf = makeTestBufImpl(requestLen);
    if (buf->computeChainDataLength() == len) {
      return buf;
    }
  }
  LOG(FATAL) << "Can't generate valid legacy request of given length: " << len;
}

SerializedRequest makeTestSerializedRequest(size_t len) {
  for (auto requestLen = len; requestLen > 0; --requestLen) {
    unique_ptr<IOBuf> buf = IOBuf::create(requestLen);
    buf->IOBuf::append(requestLen);
    memset(buf->writableData(), char(0x80), requestLen);
    if (LegacySerializedRequest(
            T_COMPACT_PROTOCOL, "test", SerializedRequest(buf->clone()))
            .buffer->computeChainDataLength() == len) {
      return SerializedRequest(std::move(buf));
    }
  }
  LOG(FATAL) << "Can't generate valid serialized request of given length: "
             << len;
}

size_t lengthWithEnvelope(const ClientReceiveState& state) {
  return LegacySerializedResponse(
             state.protocolId(),
             0,
             state.messageType(),
             "test",
             SerializedResponse(state.serializedResponse().buffer->clone()))
      .buffer->computeChainDataLength();
}

class EventBaseAborter : public folly::AsyncTimeout {
 public:
  EventBaseAborter(folly::EventBase* eventBase, uint32_t timeoutMS)
      : folly::AsyncTimeout(
            eventBase, folly::AsyncTimeout::InternalEnum::INTERNAL),
        eventBase_(eventBase) {
    scheduleTimeout(timeoutMS);
  }

  void timeoutExpired() noexcept override {
    ADD_FAILURE();
    eventBase_->terminateLoopSoon();
  }

 private:
  folly::EventBase* eventBase_;
};

// Creates/unwraps a framed message (LEN(MSG) | MSG)
class TestFramingHandler : public FramingHandler {
 public:
  std::tuple<unique_ptr<IOBuf>, size_t, unique_ptr<THeader>> removeFrame(
      IOBufQueue* q) override {
    assert(q);
    queue_.append(*q);
    if (!queue_.front() || queue_.front()->empty()) {
      return make_tuple(std::unique_ptr<IOBuf>(), 0, nullptr);
    }

    uint32_t len = queue_.front()->computeChainDataLength();

    if (len < 4) {
      size_t remaining = 4 - len;
      return make_tuple(unique_ptr<IOBuf>(), remaining, nullptr);
    }

    folly::io::Cursor c(queue_.front());
    uint32_t msgLen = c.readBE<uint32_t>();
    if (len - 4 < msgLen) {
      size_t remaining = msgLen - (len - 4);
      return make_tuple(unique_ptr<IOBuf>(), remaining, nullptr);
    }

    queue_.trimStart(4);
    return make_tuple(queue_.split(msgLen), 0, nullptr);
  }

  unique_ptr<IOBuf> addFrame(unique_ptr<IOBuf> buf, THeader*) override {
    assert(buf);
    unique_ptr<IOBuf> framing;

    if (buf->headroom() > 4) {
      buf->prepend(4);
      framing = std::move(buf);
    } else {
      framing = IOBuf::create(4);
      framing->append(4);
      framing->appendChain(std::move(buf));
    }
    folly::io::RWPrivateCursor c(framing.get());
    c.writeBE<uint32_t>(framing->computeChainDataLength() - 4);

    return framing;
  }

 private:
  IOBufQueue queue_;
};

template <typename Channel>
unique_ptr<Channel, folly::DelayedDestruction::Destructor> createChannel(
    folly::AsyncTransport::UniquePtr transport) {
  return Channel::newChannel(std::move(transport));
}

template <>
unique_ptr<Cpp2Channel, folly::DelayedDestruction::Destructor> createChannel(
    folly::AsyncTransport::UniquePtr transport) {
  return Cpp2Channel::newChannel(
      std::move(transport), make_unique<TestFramingHandler>());
}

template <>
HeaderClientChannel::LegacyPtr createChannel(
    folly::AsyncTransport::UniquePtr transport) {
  return HeaderClientChannel::newChannel(
      HeaderClientChannel::WithoutRocketUpgrade{}, std::move(transport));
}

template <typename Channel1, typename Channel2>
class SocketPairTest {
 public:
  struct Config {
    bool ssl{false};
  };

  SocketPairTest(Config config = Config()) : eventBase_() {
    folly::SocketPair socketPair;

    folly::AsyncSocket::UniquePtr socket0, socket1;
    if (config.ssl) {
      auto clientCtx = std::make_shared<folly::SSLContext>();
      auto serverCtx = std::make_shared<folly::SSLContext>();
      getctx(clientCtx, serverCtx);
      socket0 = TAsyncSSLSocket::newSocket(
          clientCtx, &eventBase_, socketPair.extractNetworkSocket0(), false);
      socket1 = TAsyncSSLSocket::newSocket(
          serverCtx, &eventBase_, socketPair.extractNetworkSocket1(), true);
      dynamic_cast<folly::AsyncSSLSocket*>(socket0.get())->sslConn(nullptr);
      dynamic_cast<folly::AsyncSSLSocket*>(socket1.get())->sslAccept(nullptr);
    } else {
      socket0 = folly::AsyncSocket::newSocket(
          &eventBase_, socketPair.extractNetworkSocket0());
      socket1 = folly::AsyncSocket::newSocket(
          &eventBase_, socketPair.extractNetworkSocket1());
    }
    socket0_ = socket0.get();
    socket1_ = socket1.get();

    channel0_ = createChannel<Channel1>(std::move(socket0));
    channel1_ = createChannel<Channel2>(std::move(socket1));
  }
  virtual ~SocketPairTest() {}

  void loop(uint32_t timeoutMS) {
    EventBaseAborter aborter(&eventBase_, timeoutMS);
    eventBase_.loop();
  }

  void run() { runWithTimeout(); }

  void getctx(
      std::shared_ptr<folly::SSLContext> clientCtx,
      std::shared_ptr<folly::SSLContext> serverCtx) {
    clientCtx->ciphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

    serverCtx->ciphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    serverCtx->loadCertificate(find_resource(folly::test::kTestCert).c_str());
    serverCtx->loadPrivateKey(find_resource(folly::test::kTestKey).c_str());
  }

  int getFd0() { return socket0_->getNetworkSocket().toFd(); }

  int getFd1() { return socket1_->getNetworkSocket().toFd(); }

  folly::AsyncSocket* getSocket0() { return socket0_; }

  folly::AsyncSocket* getSocket1() { return socket1_; }

  void runWithTimeout(uint32_t timeoutMS = 6000) {
    preLoop();
    loop(timeoutMS);
    postLoop();
  }

  virtual void preLoop() {}
  virtual void postLoop() {}

 protected:
  folly::EventBase eventBase_;
  folly::AsyncSocket* socket0_;
  folly::AsyncSocket* socket1_;
  unique_ptr<Channel1, folly::DelayedDestruction::Destructor> channel0_;
  unique_ptr<Channel2, folly::DelayedDestruction::Destructor> channel1_;
};

class MessageCallback : public MessageChannel::SendCallback,
                        public MessageChannel::RecvCallback {
 public:
  MessageCallback()
      : sent_(0),
        recv_(0),
        sendError_(0),
        recvError_(0),
        recvEOF_(0),
        recvBytes_(0) {}

  void sendQueued() override {}

  void messageSent() override { sent_++; }
  void messageSendError(folly::exception_wrapper&&) override { sendError_++; }

  void messageReceived(
      unique_ptr<IOBuf>&& buf, unique_ptr<THeader>&&) override {
    recv_++;
    recvBytes_ += buf->computeChainDataLength();
  }
  void messageChannelEOF() override { recvEOF_++; }
  void messageReceiveErrorWrapped(folly::exception_wrapper&&) override {
    sendError_++;
  }

  uint32_t sent_;
  uint32_t recv_;
  uint32_t sendError_;
  uint32_t recvError_;
  uint32_t recvEOF_;
  size_t recvBytes_;
};

class TestRequestCallback : public RequestClientCallback, public CloseCallback {
 public:
  explicit TestRequestCallback(bool oneWay = false) : oneWay_(oneWay) {}

  void onResponse(ClientReceiveState&& state) noexcept override {
    if (!oneWay_) {
      reply_++;
      replyBytes_ += lengthWithEnvelope(state);
    }
    delete this;
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    EXPECT_TRUE(ex);
    replyError_++;
    delete this;
  }

  void channelClosed() override { closed_ = true; }

  static void reset() {
    closed_ = false;
    reply_ = 0;
    replyBytes_ = 0;
    replyError_ = 0;
  }
  static bool closed_;
  static uint32_t reply_;
  static uint32_t replyBytes_;
  static uint32_t replyError_;

 protected:
  const bool oneWay_;
};

bool TestRequestCallback::closed_ = false;
uint32_t TestRequestCallback::reply_ = 0;
uint32_t TestRequestCallback::replyBytes_ = 0;
uint32_t TestRequestCallback::replyError_ = 0;

class ResponseCallback : public HeaderServerChannel::Callback {
 public:
  ResponseCallback()
      : serverClosed_(false), oneway_(0), request_(0), requestBytes_(0) {}

  void requestReceived(
      unique_ptr<HeaderServerChannel::HeaderRequest>&& req) override {
    request_++;
    requestBytes_ += req->getBuf()->computeChainDataLength();
    if (req->isOneway()) {
      oneway_++;
    } else {
      req->sendReply(ResponsePayload::create(req->extractBuf()));
    }
  }

  void channelClosed(folly::exception_wrapper&&) override {
    serverClosed_ = true;
  }

  bool serverClosed_;
  uint32_t oneway_;
  uint32_t request_;
  uint32_t requestBytes_;
};

class MessageTest : public SocketPairTest<Cpp2Channel, Cpp2Channel>,
                    public MessageCallback {
 public:
  explicit MessageTest(size_t len, Config socketConfig = Config())
      : SocketPairTest(socketConfig), len_(len), header_(new THeader) {}

  void preLoop() override {
    channel0_->sendMessage(&sendCallback_, makeTestBuf(len_), header_.get());
    channel1_->setReceiveCallback(this);
  }

  void postLoop() override {
    EXPECT_EQ(sendCallback_.sendError_, 0);
    EXPECT_EQ(recvError_, 0);
    EXPECT_EQ(recvEOF_, 0);
    EXPECT_EQ(recv_, 1);
    EXPECT_EQ(sendCallback_.sent_, 1);
    EXPECT_EQ(recvBytes_, len_);
  }

  void messageReceived(
      unique_ptr<IOBuf>&& buf, unique_ptr<THeader>&& header) override {
    MessageCallback::messageReceived(std::move(buf), std::move(header));
    channel1_->setReceiveCallback(nullptr);
  }

 private:
  size_t len_;
  unique_ptr<THeader> header_;
  MessageCallback sendCallback_;
};

TEST(Channel, Cpp2Channel) {
  MessageTest(10).run();
  MessageTest(100).run();
  MessageTest(1024 * 1024).run();
}

TEST(Channel, Cpp2ChannelSSL) {
  MessageTest::Config socketConfig;
  socketConfig.ssl = true;
  MessageTest(10, socketConfig).run();
  MessageTest(100, socketConfig).run();
  MessageTest(1024 * 1024, socketConfig).run();
}

class MessageCloseTest : public SocketPairTest<Cpp2Channel, Cpp2Channel>,
                         public MessageCallback {
 public:
  MessageCloseTest() : header_(new THeader) {}

  void preLoop() override {
    channel0_->sendMessage(
        &sendCallback_, makeTestBuf(1024 * 1024), header_.get());
    // Close the other socket after delay
    this->eventBase_.runInLoop(
        std::bind(&folly::AsyncSocket::close, this->socket1_));
    channel1_->setReceiveCallback(this);
  }

  void postLoop() override {
    EXPECT_EQ(sendCallback_.sendError_, 1);
    EXPECT_EQ(recvError_, 0);
    EXPECT_EQ(recvEOF_, 1);
    EXPECT_EQ(recv_, 0);
    EXPECT_EQ(sendCallback_.sent_, 0);
  }

  void messageChannelEOF() override {
    MessageCallback::messageChannelEOF();
    channel1_->setReceiveCallback(nullptr);
  }

 private:
  MessageCallback sendCallback_;
  unique_ptr<THeader> header_;
};

TEST(Channel, MessageCloseTest) {
  MessageCloseTest().run();
}

class MessageEOFTest : public SocketPairTest<Cpp2Channel, Cpp2Channel>,
                       public MessageCallback {
 public:
  MessageEOFTest() : header_(new THeader) {}

  void preLoop() override {
    channel0_->setReceiveCallback(this);
    channel1_->getTransport()->shutdownWrite();
    channel0_->sendMessage(this, makeTestBuf(1024 * 1024), header_.get());
  }

  void postLoop() override {
    EXPECT_EQ(sendError_, 1);
    EXPECT_EQ(recvError_, 0);
    EXPECT_EQ(recvEOF_, 1);
    EXPECT_EQ(recv_, 0);
    EXPECT_EQ(sent_, 0);
  }

 private:
  unique_ptr<THeader> header_;
};

TEST(Channel, MessageEOFTest) {
  MessageEOFTest().run();
}

class HeaderChannelTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit HeaderChannelTest(size_t len, Config socketConfig = Config())
      : SocketPairTest(socketConfig), len_(len) {}

  class Callback : public TestRequestCallback {
   public:
    Callback(HeaderChannelTest* c, bool oneWay)
        : TestRequestCallback(oneWay), c_(c) {}
    void onResponse(ClientReceiveState&& state) noexcept override {
      if (!oneWay_) {
        c_->channel1_->setCallback(nullptr);
      }
      TestRequestCallback::onResponse(std::move(state));
    }

   private:
    HeaderChannelTest* c_;
  };

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->setCallback(this);
    channel0_->setCloseCallback(this);
    RpcOptions options;
    channel0_->sendRequestNoResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new Callback(this, true)));
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new Callback(this, false)));
    channel0_->setCloseCallback(nullptr);
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 1);
    EXPECT_EQ(replyError_, 0);
    EXPECT_EQ(replyBytes_, len_);
    EXPECT_EQ(closed_, false);
    EXPECT_EQ(serverClosed_, false);
    EXPECT_EQ(request_, 2);
    EXPECT_EQ(requestBytes_, len_ * 2);
    EXPECT_EQ(oneway_, 1);
    channel1_->setCallback(nullptr);
  }

 private:
  size_t len_;
};

TEST(Channel, HeaderChannelTest) {
  HeaderChannelTest(10).run();
  HeaderChannelTest(100).run();
  HeaderChannelTest(1024 * 1024).run();
}

TEST(Channel, HeaderChannelTestSSL) {
  HeaderChannelTest::Config socketConfig;
  socketConfig.ssl = true;
  HeaderChannelTest(10, socketConfig).run();
  HeaderChannelTest(100, socketConfig).run();
  HeaderChannelTest(1024 * 1024, socketConfig).run();
}

class HeaderChannelClosedTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel> {
  //   , public TestRequestCallback
  //   , public ResponseCallback {
 public:
  explicit HeaderChannelClosedTest() {}

  class Callback : public RequestClientCallback {
   public:
    explicit Callback(HeaderChannelClosedTest* c) : c_(c) {}

    ~Callback() override { c_->callbackDtor_ = true; }

    void onResponse(ClientReceiveState&&) noexcept override {
      FAIL() << "should not recv reply from closed channel";
    }

    void onResponseError(folly::exception_wrapper ew) noexcept override {
      EXPECT_TRUE(ew.with_exception([this](const TTransportException& e) {
        EXPECT_EQ(e.getType(), TTransportException::END_OF_FILE);
        c_->gotError_ = true;
      }));
      delete this;
    }

   private:
    HeaderChannelClosedTest* c_;
  };

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->getTransport()->shutdownWrite();
    RpcOptions options;
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(42),
        std::make_unique<THeader>(),
        RequestClientCallback::Ptr(new Callback(this)));
  }

  void postLoop() override {
    EXPECT_TRUE(gotError_);
    EXPECT_TRUE(callbackDtor_);
  }

 private:
  uint32_t seqId_;
  bool gotError_ = true;
  bool callbackDtor_ = false;
};

TEST(Channel, HeaderChannelClosedTest) {
  HeaderChannelClosedTest().run();
}

class InOrderTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit InOrderTest(Config socketConfig = Config())
      : SocketPairTest(socketConfig), len_(10) {}

  class Callback : public TestRequestCallback {
   public:
    explicit Callback(InOrderTest* c) : c_(c) {}
    void onResponse(ClientReceiveState&& state) noexcept override {
      if (reply_ == 1) {
        c_->channel1_->setCallback(nullptr);
        // Verify that they came back in the same order
        EXPECT_EQ(lengthWithEnvelope(state), c_->len_ + 1);
      }
      TestRequestCallback::onResponse(std::move(state));
    }

    void requestReceived(ResponseChannelRequest::UniquePtr rcr) {
      auto req = dynamic_cast<HeaderServerChannel::HeaderRequest*>(rcr.get());
      c_->request_++;
      c_->requestBytes_ += req->getBuf()->computeChainDataLength();
      if (c_->firstbuf_) {
        req->sendReply(ResponsePayload::create(req->extractBuf()));
        auto firstbuf = dynamic_cast<HeaderServerChannel::HeaderRequest*>(
            c_->firstbuf_.get());
        c_->firstbuf_->sendReply(
            ResponsePayload::create(firstbuf->extractBuf()));
      } else {
        c_->firstbuf_ = std::move(rcr);
      }
    }

   private:
    InOrderTest* c_;
  };

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->setCallback(this);
    RpcOptions options;
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new Callback(this)));
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_ + 1),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new Callback(this)));
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 2);
    EXPECT_EQ(replyError_, 0);
    EXPECT_EQ(replyBytes_, 2 * len_ + 1);
    EXPECT_EQ(closed_, false);
    EXPECT_EQ(serverClosed_, false);
    EXPECT_EQ(request_, 2);
    EXPECT_EQ(requestBytes_, 2 * len_ + 1);
    EXPECT_EQ(oneway_, 0);
  }

 private:
  ResponseChannelRequest::UniquePtr firstbuf_;
  size_t len_;
};

TEST(Channel, InOrderTest) {
  InOrderTest().run();
}

TEST(Channel, InOrderTestSSL) {
  InOrderTest::Config socketConfig;
  socketConfig.ssl = true;
  InOrderTest(socketConfig).run();
}

class BadSeqIdTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit BadSeqIdTest(size_t len, Config socketConfig = Config())
      : SocketPairTest(socketConfig), len_(len) {}

  class Callback : public TestRequestCallback {
   public:
    Callback(BadSeqIdTest* c, bool oneWay)
        : TestRequestCallback(oneWay), c_(c) {}

    void onResponseError(folly::exception_wrapper ew) noexcept override {
      c_->channel1_->setCallback(nullptr);
      TestRequestCallback::onResponseError(std::move(ew));
    }

   private:
    BadSeqIdTest* c_;
  };

  void requestReceived(
      unique_ptr<HeaderServerChannel::HeaderRequest>&& req) override {
    request_++;
    requestBytes_ += req->getBuf()->computeChainDataLength();
    if (req->isOneway()) {
      oneway_++;
      return;
    }
    unique_ptr<THeader> header(new THeader);
    header->setSequenceNumber(-1);
    HeaderServerChannel::HeaderRequest r(
        channel1_.get(), req->extractBuf(), std::move(header), {});
    r.sendReply(ResponsePayload::create(r.extractBuf()));
  }

  void preLoop() override {
    TestRequestCallback::reset();
    channel0_->setTimeout(1000);
    channel1_->setCallback(this);
    RpcOptions options;
    channel0_->sendRequestNoResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new Callback(this, true)));
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new Callback(this, false)));
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 0);
    EXPECT_EQ(replyError_, 1);
    EXPECT_EQ(replyBytes_, 0);
    EXPECT_EQ(closed_, false);
    EXPECT_EQ(serverClosed_, false);
    EXPECT_EQ(request_, 2);
    EXPECT_EQ(requestBytes_, len_ * 2);
    EXPECT_EQ(oneway_, 1);
  }

 private:
  size_t len_;
};

TEST(Channel, BadSeqIdTest) {
  BadSeqIdTest(10).run();
}

TEST(Channel, BadSeqIdTestSSL) {
  BadSeqIdTest::Config socketConfig;
  socketConfig.ssl = true;
  BadSeqIdTest(10, socketConfig).run();
}

class TimeoutTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit TimeoutTest(uint32_t timeout, Config socketConfig = Config())
      : SocketPairTest(socketConfig), timeout_(timeout), len_(10) {}

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->setCallback(this);
    channel0_->setTimeout(timeout_);
    channel0_->setCloseCallback(this);
    RpcOptions options;
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new TestRequestCallback()));
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new TestRequestCallback()));
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 0);
    EXPECT_EQ(replyError_, 2);
    EXPECT_EQ(replyBytes_, 0);
    EXPECT_EQ(closed_, false); // client timeouts do not close connection
    EXPECT_EQ(serverClosed_, false);
    EXPECT_EQ(request_, 2);
    EXPECT_EQ(requestBytes_, len_ * 2);
    EXPECT_EQ(oneway_, 0);
    channel0_->setCloseCallback(nullptr);
    channel1_->setCallback(nullptr);
  }

  void requestReceived(
      unique_ptr<HeaderServerChannel::HeaderRequest>&& req) override {
    request_++;
    requestBytes_ += req->getBuf()->computeChainDataLength();
    // Don't respond, let it time out
    // TestRequestCallback::replyReceived(std::move(buf));
    channel1_->getEventBase()->tryRunAfterDelay(
        [&]() {
          channel1_->setCallback(nullptr);
          channel0_->setCloseCallback(nullptr);
        },
        timeout_ * 2); // enough time for server socket to close also
  }

 private:
  uint32_t timeout_;
  size_t len_;
};

TEST(Channel, TimeoutTest) {
  TimeoutTest(25).run();
  TimeoutTest(100).run();
  TimeoutTest(250).run();
}

TEST(Channel, TimeoutTestSSL) {
  TimeoutTest::Config socketConfig;
  socketConfig.ssl = true;
  TimeoutTest(25, socketConfig).run();
  TimeoutTest(100, socketConfig).run();
  TimeoutTest(250, socketConfig).run();
}

// Test client per-call timeout options
class OptionsTimeoutTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit OptionsTimeoutTest(Config socketConfig = Config())
      : SocketPairTest(socketConfig), len_(10) {}

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->setCallback(this);
    channel0_->setTimeout(1000);
    RpcOptions options;
    options.setTimeout(std::chrono::milliseconds(25));
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(len_),
        std::unique_ptr<THeader>(new THeader),
        RequestClientCallback::Ptr(new TestRequestCallback()));
    // Verify the timeout worked within 10ms
    channel0_->getEventBase()->tryRunAfterDelay(
        [&]() { EXPECT_EQ(replyError_, 1); }, 35);
    // Verify that subsequent successful requests don't delay timeout
    channel0_->getEventBase()->tryRunAfterDelay(
        [&]() {
          RpcOptions options;
          channel0_->sendRequestResponse(
              options,
              "test",
              makeTestSerializedRequest(len_),
              std::unique_ptr<THeader>(new THeader),
              RequestClientCallback::Ptr(new TestRequestCallback()));
        },
        20);
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 1);
    EXPECT_EQ(replyError_, 1);
    EXPECT_EQ(replyBytes_, len_);
    EXPECT_EQ(closed_, false); // client timeouts do not close connection
    EXPECT_EQ(serverClosed_, false);
    EXPECT_EQ(request_, 2);
    EXPECT_EQ(requestBytes_, len_ * 2);
    EXPECT_EQ(oneway_, 0);
  }

  void requestReceived(
      unique_ptr<HeaderServerChannel::HeaderRequest>&& req) override {
    if (request_ == 0) {
      request_++;
      requestBytes_ += req->getBuf()->computeChainDataLength();
    } else {
      ResponseCallback::requestReceived(std::move(req));
      channel1_->setCallback(nullptr);
    }
  }

 private:
  size_t len_;
};

TEST(Channel, OptionsTimeoutTest) {
  OptionsTimeoutTest().run();
}

TEST(Channel, OptionsTimeoutTestSSL) {
  OptionsTimeoutTest::Config socketConfig;
  socketConfig.ssl = true;
  OptionsTimeoutTest(socketConfig).run();
}

class ClientCloseTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit ClientCloseTest(bool halfClose) : halfClose_(halfClose) {}

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->setCallback(this);
    channel0_->setCloseCallback(this);
    if (halfClose_) {
      channel1_->getEventBase()->tryRunAfterDelay(
          [&]() { channel1_->getTransport()->shutdownWrite(); }, 10);
    } else {
      channel1_->getEventBase()->tryRunAfterDelay(
          [&]() { channel1_->getTransport()->close(); }, 10);
    }
    channel1_->getEventBase()->tryRunAfterDelay(
        [&]() { channel1_->setCallback(nullptr); }, 20);
    channel0_->getEventBase()->tryRunAfterDelay(
        [&]() { channel0_->setCloseCallback(nullptr); }, 20);
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 0);
    EXPECT_EQ(replyError_, 0);
    EXPECT_EQ(replyBytes_, 0);
    EXPECT_EQ(closed_, true);
    EXPECT_EQ(serverClosed_, !halfClose_);
    EXPECT_EQ(request_, 0);
    EXPECT_EQ(requestBytes_, 0);
    EXPECT_EQ(oneway_, 0);
  }

 private:
  bool halfClose_;
};

TEST(Channel, ClientCloseTest) {
  ClientCloseTest(true).run();
  ClientCloseTest(false).run();
}

class ServerCloseTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public ResponseCallback {
 public:
  explicit ServerCloseTest(bool halfClose) : halfClose_(halfClose) {}

  void preLoop() override {
    TestRequestCallback::reset();
    channel1_->setCallback(this);
    channel0_->setCloseCallback(this);
    if (halfClose_) {
      channel0_->getEventBase()->tryRunAfterDelay(
          [&]() { channel0_->getTransport()->shutdownWrite(); }, 10);
    } else {
      channel0_->getEventBase()->tryRunAfterDelay(
          [&]() { channel0_->getTransport()->close(); }, 10);
    }
    channel1_->getEventBase()->tryRunAfterDelay(
        [&]() { channel1_->setCallback(nullptr); }, 20);
    channel0_->getEventBase()->tryRunAfterDelay(
        [&]() { channel0_->setCloseCallback(nullptr); }, 20);
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 0);
    EXPECT_EQ(replyError_, 0);
    EXPECT_EQ(replyBytes_, 0);
    EXPECT_EQ(closed_, !halfClose_);
    EXPECT_EQ(serverClosed_, true);
    EXPECT_EQ(request_, 0);
    EXPECT_EQ(requestBytes_, 0);
    EXPECT_EQ(oneway_, 0);
  }

 private:
  bool halfClose_;
};

TEST(Channel, ServerCloseTest) {
  ServerCloseTest(true).run();
  ServerCloseTest(false).run();
}

class ClientCloseOnErrorTest;
class InvalidResponseCallback : public HeaderServerChannel::Callback {
 public:
  explicit InvalidResponseCallback(ClientCloseOnErrorTest* self)
      : self_(self), request_(0), requestBytes_(0) {}

  // configuration
  InvalidResponseCallback& closeSocketInResponse(bool value) {
    closeSocketInResponse_ = value;
    return *this;
  }

  void requestReceived(
      unique_ptr<HeaderServerChannel::HeaderRequest>&& req) override;
  void channelClosed(folly::exception_wrapper&&) override {}

 protected:
  ClientCloseOnErrorTest* self_;
  uint32_t request_;
  uint32_t requestBytes_;

  bool closeSocketInResponse_ = false;
};

class ClientCloseOnErrorTest
    : public SocketPairTest<HeaderClientChannel, HeaderServerChannel>,
      public TestRequestCallback,
      public InvalidResponseCallback {
 public:
  explicit ClientCloseOnErrorTest() : InvalidResponseCallback(this) {}

  // configuration
  ClientCloseOnErrorTest& forcePendingSend(bool value) {
    forcePendingSend_ = value;
    return *this;
  }

  ClientCloseOnErrorTest& closeSocketInResponse(bool value) {
    InvalidResponseCallback::closeSocketInResponse(value);
    return *this;
  }

  class Callback : public TestRequestCallback {
   public:
    explicit Callback(ClientCloseOnErrorTest* c) : c_(c) {}

    void onResponseError(folly::exception_wrapper ew) noexcept override {
      // force closing the channel on error
      c_->channel0_->closeNow();
      TestRequestCallback::onResponseError(std::move(ew));
    }

   private:
    ClientCloseOnErrorTest* c_;
  };

  void preLoop() override {
    TestRequestCallback::reset();

    reqSize_ = 30;
    uint32_t ss = sizeof(reqSize_);
    if (forcePendingSend_) {
      // make request size big enough to not fit into kernel buffer
      getsockopt(getFd1(), SOL_SOCKET, SO_RCVBUF, &reqSize_, &ss);
      reqSize_++;
    }

    channel1_->setCallback(this);
    RpcOptions options;
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(10),
        std::make_unique<THeader>(),
        RequestClientCallback::Ptr(new Callback(this)));
    channel0_->sendRequestResponse(
        options,
        "test",
        makeTestSerializedRequest(reqSize_),
        std::make_unique<THeader>(),
        RequestClientCallback::Ptr(new Callback(this)));
  }

  void postLoop() override {
    EXPECT_EQ(reply_, 0);
    EXPECT_EQ(replyError_, 2);
    EXPECT_EQ(replyBytes_, 0);
    EXPECT_EQ(request_, (forcePendingSend_ ? 1 : 2));
    EXPECT_EQ(requestBytes_, 10 + (forcePendingSend_ ? 0 : reqSize_));
    channel1_->setCallback(nullptr);
  }

 private:
  bool forcePendingSend_ = false;
  int32_t reqSize_;
};

void InvalidResponseCallback::requestReceived(
    unique_ptr<HeaderServerChannel::HeaderRequest>&& req) {
  request_++;
  requestBytes_ += req->getBuf()->computeChainDataLength();
  if (closeSocketInResponse_) {
    self_->getSocket1()->shutdownWrite();
  } else {
    write(self_->getFd1(), "SSH-", 4);
  }
}

TEST(Channel, ClientCloseOnErrorTest) {
  ClientCloseOnErrorTest()
      .forcePendingSend(false)
      .closeSocketInResponse(true)
      .run();
  ClientCloseOnErrorTest()
      .forcePendingSend(false)
      .closeSocketInResponse(false)
      .run();
  ClientCloseOnErrorTest()
      .forcePendingSend(true)
      .closeSocketInResponse(true)
      .run();
  ClientCloseOnErrorTest()
      .forcePendingSend(true)
      .closeSocketInResponse(false)
      .run();
}

class DestroyAsyncTransport : public folly::AsyncTransport {
 public:
  DestroyAsyncTransport() : cb_(nullptr) {}
  void setReadCB(folly::AsyncTransport::ReadCallback* callback) override {
    cb_ = callback;
  }
  ReadCallback* getReadCallback() const override {
    return dynamic_cast<ReadCallback*>(cb_);
  }
  void write(
      folly::AsyncTransport::WriteCallback*,
      const void*,
      size_t,
      folly::WriteFlags) override {}
  void writev(
      folly::AsyncTransport::WriteCallback*,
      const iovec*,
      size_t,
      folly::WriteFlags) override {}
  void writeChain(
      folly::AsyncTransport::WriteCallback*,
      std::unique_ptr<folly::IOBuf>&&,
      folly::WriteFlags) override {}
  void close() override {}
  void closeNow() override {}
  void shutdownWrite() override {}
  void shutdownWriteNow() override {}
  bool good() const override { return true; }
  bool readable() const override { return false; }
  bool connecting() const override { return false; }
  bool error() const override { return false; }
  void attachEventBase(folly::EventBase*) override {}
  void detachEventBase() override {}
  bool isDetachable() const override { return true; }
  folly::EventBase* getEventBase() const override { return nullptr; }
  void setSendTimeout(uint32_t /* ms */) override {}
  uint32_t getSendTimeout() const override { return 0; }
  void getLocalAddress(folly::SocketAddress*) const override {}
  void getPeerAddress(folly::SocketAddress*) const override {}
  size_t getAppBytesWritten() const override { return 0; }
  size_t getRawBytesWritten() const override { return 0; }
  size_t getAppBytesReceived() const override { return 0; }
  size_t getRawBytesReceived() const override { return 0; }
  void setEorTracking(bool /* track */) override {}
  bool isEorTrackingEnabled() const override { return false; }

  void invokeEOF() { cb_->readEOF(); }

 private:
  folly::AsyncTransport::ReadCallback* cb_;
};

class DestroyRecvCallback : public MessageChannel::RecvCallback {
 public:
  typedef std::unique_ptr<Cpp2Channel, folly::DelayedDestruction::Destructor>
      ChannelPointer;
  explicit DestroyRecvCallback(ChannelPointer&& channel)
      : channel_(std::move(channel)), invocations_(0) {
    channel_->setReceiveCallback(this);
  }
  void messageReceived(
      std::unique_ptr<folly::IOBuf>&&,
      std::unique_ptr<apache::thrift::transport::THeader>&&) override {}
  void messageChannelEOF() override {
    EXPECT_EQ(invocations_, 0);
    invocations_++;
    channel_.reset();
  }
  void messageReceiveErrorWrapped(folly::exception_wrapper&&) override {}

 private:
  ChannelPointer channel_;
  int invocations_;
};

TEST(Channel, DestroyInEOF) {
  auto t = new DestroyAsyncTransport();
  auto transport = folly::AsyncTransport::UniquePtr(t);
  auto channel = createChannel<Cpp2Channel>(std::move(transport));
  DestroyRecvCallback drc(std::move(channel));
  t->invokeEOF();
}

class NullCloseCallback : public CloseCallback {
 public:
  void channelClosed() override {}
};
