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

#include <signal.h>

#include <boost/random.hpp>
#include <folly/portability/GTest.h>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/SocketPair.h>
#include <folly/io/async/test/Util.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp/async/TBinaryAsyncChannel.h>
#include <thrift/lib/cpp/async/TFramedAsyncChannel.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/protocol/TBinaryProtocol.h>

using namespace boost;
using namespace folly;
using namespace std::chrono;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

using apache::thrift::async::TAsyncChannel;
using apache::thrift::async::TBinaryAsyncChannel;
using apache::thrift::async::TFramedAsyncChannel;
using apache::thrift::protocol::TBinaryProtocolT;
using apache::thrift::transport::TBufferBase;
using apache::thrift::transport::TFramedTransport;
using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TTransportException;
using folly::AsyncSocket;
using folly::AsyncTimeout;
using folly::EventBase;

///////////////////////////////////////////////////////////////////////////
// Utility code
//////////////////////////////////////////////////////////////////////////

class ChannelCallback {
 public:
  ChannelCallback()
      : sendDone_(0),
        sendError_(0),
        recvDone_(0),
        recvError_(0),
        timestamp_(false) {}

  void send(
      const std::shared_ptr<TAsyncChannel>& channel, TMemoryBuffer* message) {
    // helper function to get the correct callbacks
    channel->sendMessage(
        std::bind(&ChannelCallback::sendDone, this),
        std::bind(&ChannelCallback::sendError, this),
        message);
  }

  void recv(
      const std::shared_ptr<TAsyncChannel>& channel, TMemoryBuffer* message) {
    // helper function to get the correct callbacks
    channel->recvMessage(
        std::bind(&ChannelCallback::recvDone, this),
        std::bind(&ChannelCallback::recvError, this),
        message);
  }

  void sendDone() {
    ++sendDone_;
    timestamp_.reset();
  }
  void sendError() {
    ++sendError_;
    timestamp_.reset();
  }

  void recvDone() {
    ++recvDone_;
    timestamp_.reset();
  }
  void recvError() {
    ++recvError_;
    timestamp_.reset();
  }

  uint32_t getSendDone() const { return sendDone_; }
  uint32_t getSendError() const { return sendError_; }
  uint32_t getRecvDone() const { return recvDone_; }
  uint32_t getRecvError() const { return recvError_; }

  const TimePoint& getTimestamp() const { return timestamp_; }

 private:
  uint32_t sendDone_;
  uint32_t sendError_;
  uint32_t recvDone_;
  uint32_t recvError_;
  TimePoint timestamp_;
};

class Message {
 public:
  explicit Message(uint32_t len, bool addFrame = false)
      : framed_(addFrame), buf_(new TMemoryBuffer(len + kPadding)) {
    init(len);
  }

  void init(uint32_t len) {
    string value;
    randomizeString(&value, len);

    // Generate a valid thrift TBinaryProtocol message,
    // containing a randomly generated string of the specified length
    std::shared_ptr<TBufferBase> transport;
    if (framed_) {
      transport.reset(new TFramedTransport(buf_));
    } else {
      transport = buf_;
    }
    TBinaryProtocolT<TBufferBase> prot(transport);

    prot.writeMessageBegin("foo", apache::thrift::protocol::T_CALL, 0);
    prot.writeStructBegin("bar");

    prot.writeFieldBegin("value", apache::thrift::protocol::T_STRING, 1);
    prot.writeString(value);
    prot.writeFieldEnd();

    prot.writeFieldStop();
    prot.writeStructEnd();
    prot.writeMessageEnd();

    transport->writeEnd();
    transport->flush();
  }

  void copyTo(TMemoryBuffer* membuf) {
    memcpy(membuf->getWritePtr(getLength()), getBuffer(), getLength());
    membuf->wroteBytes(getLength());
  }

  void checkEqual(TMemoryBuffer* membuf) const {
    // If we have a frame header, skip it when comparing
    uint32_t length = getLength();
    const uint8_t* myBuf = getBuffer();
    if (framed_) {
      length -= sizeof(uint32_t);
      myBuf += sizeof(uint32_t);
    }

    CHECK_EQ(membuf->available_read(), length);

    uint32_t borrowLen = length;
    const uint8_t* otherBuf = membuf->borrow(nullptr, &borrowLen);
    CHECK(otherBuf != nullptr);
    CHECK_EQ(borrowLen, length);

    CHECK_EQ(memcmp(otherBuf, myBuf, length), 0);
  }

  uint32_t getLength() const { return buf_->available_read(); }

  const uint8_t* getBuffer() const {
    uint32_t borrowLen = getLength();
    const uint8_t* buf = buf_->borrow(nullptr, &borrowLen);
    assert(buf != nullptr);
    return buf;
  }

 private:
  void randomizeString(string* ret, uint32_t len) {
    ret->resize(len);

    // Randomly initialize the string
    // TODO: we don't currently seed the RNG.
    boost::mt19937 rng;
    for (uint32_t n = 0; n < len; ++n) {
      // The RNG gives us more than 1 byte of randomness, but
      // we don't currently take advantage of it.  It's easier for now
      // just to proceed 1 byte at a time.
      (*ret)[n] = rng();
    }
  }

  static const uint32_t kPadding = 256; // extra room for serialization overhead

  bool framed_;
  std::shared_ptr<TMemoryBuffer> buf_;
};

struct ChunkInfo {
  ChunkInfo(int b, milliseconds d) : bytes(b), delayMS(d) {}

  int bytes;
  milliseconds delayMS;
};

/*
 * Allows ChunkSchedules to be easily specified in the code.  The elements are
 * ChunkInfo objects {bytes, delay}.
 *
 * The elements must end with a chunk that has a non-positive bytes value.
 * A negative bytes value means to send the rest of the data, a value of 0
 * means to close the socket.
 */
using ChunkSchedule = vector<ChunkInfo>;

class ChunkSender : private AsyncTransport::WriteCallback,
                    private AsyncTimeout {
 public:
  ChunkSender(
      EventBase* evb,
      AsyncSocket* socket,
      const Message* msg,
      const ChunkSchedule& schedule)
      : AsyncTimeout(evb),
        bufOffset_(0),
        scheduleIndex_(0),
        currentChunkLen_(0),
        error_(false),
        socket_(socket),
        message_(msg),
        schedule_(schedule) {}

  void start() { scheduleNext(); }

  const ChunkSchedule* getSchedule() const { return &schedule_; }

  bool error() const { return error_; }

 private:
  void scheduleNext() {
    assert(scheduleIndex_ < schedule_.size());
    const ChunkInfo& info = schedule_[scheduleIndex_];
    if (info.delayMS.count() <= 0) {
      sendNow();
    } else {
      scheduleTimeout(info.delayMS);
    }
  }

  void sendNow() {
    assert(scheduleIndex_ < schedule_.size());
    const ChunkInfo& info = schedule_[scheduleIndex_];

    assert(bufOffset_ <= message_->getLength());

    uint32_t len;
    if (info.bytes == 0) {
      // close the socket
      socket_->close();
      return;
    } else if (info.bytes < 0) {
      // write the rest of the data
      if (bufOffset_ >= message_->getLength()) {
        // nothing more to write
        return;
      }
      len = message_->getLength() - bufOffset_;
    } else {
      len = info.bytes;
      if (len + bufOffset_ > message_->getLength()) {
        // bug in the test code: ChunkSchedule lists more data than available
        ADD_FAILURE() << "bad ChunkSchedule";

        len = message_->getLength() - bufOffset_;
        if (len == 0) {
          ++scheduleIndex_;
          if (scheduleIndex_ < schedule_.size()) {
            scheduleNext();
          }
          return;
        }
      }
    }

    currentChunkLen_ = len;
    const uint8_t* buf = message_->getBuffer() + bufOffset_;
    socket_->write(this, buf, len);
  }

  void writeSuccess() noexcept override {
    bufOffset_ += currentChunkLen_;

    ++scheduleIndex_;
    if (scheduleIndex_ < schedule_.size()) {
      scheduleNext();
    }
  }

  void writeErr(size_t /* bytesWritten */, const AsyncSocketException&) noexcept
      override {
    error_ = true;
  }

  void timeoutExpired() noexcept override { sendNow(); }

  uint32_t bufOffset_;
  uint32_t scheduleIndex_;
  uint32_t currentChunkLen_;
  bool error_;
  AsyncSocket* socket_;
  const Message* message_;
  ChunkSchedule schedule_;
};

class MultiMessageSize : public vector<int> {
 public:
  MultiMessageSize(int len, ...) {
    push_back(len);

    va_list ap;
    va_start(ap, len);

    while (true) {
      int b = va_arg(ap, int);
      if (b <= 0) {
        break;
      } else {
        push_back(b);
      }
    }

    va_end(ap);
  }
};

class MultiMessageSenderReceiver : private AsyncTransport::WriteCallback,
                                   private AsyncTimeout {
 public:
  MultiMessageSenderReceiver(
      EventBase* evb,
      AsyncSocket* socket,
      const MultiMessageSize& multiMessage,
      bool framed,
      uint32_t writeTimes,
      bool queued = false,
      milliseconds delayMS = milliseconds(2))
      : AsyncTimeout(evb),
        writeError_(false),
        readError_(false),
        socket_(socket),
        queued_(queued),
        delayMS_(delayMS) {
    uint32_t totalSize = 0;
    for (vector<int>::const_iterator it = multiMessage.begin();
         it != multiMessage.end();
         it++) {
      Message message(*it, framed);
      writeMessages_.push_back(message);
      writeMemoryBuffer_.write(message.getBuffer(), message.getLength());
      totalSize += message.getLength();
    }

    assert(writeMessages_.size() > 0);
    writeSize_ = totalSize / writeTimes;
  }

  void initialize(const std::shared_ptr<TAsyncChannel>& channel) {
    int n_msgs = writeMessages_.size();
    uint32_t n_recvs = (queued_) ? n_msgs : 1;
    for (uint32_t i = 0; i < n_recvs; i++) {
      channel->recvMessage(
          std::bind(&MultiMessageSenderReceiver::recvDone, this),
          std::bind(&MultiMessageSenderReceiver::recvError, this),
          &readMemoryBuffer_);
    }
    scheduleNext();
    recvChannel_ = channel;
  }

  bool getReadError() const { return readError_; }

  bool getWriteError() const { return writeError_; }

  vector<std::shared_ptr<TMemoryBuffer>>& getReadBuffers() {
    return readBuffers_;
  }

  vector<Message>& getWriteMessages() { return writeMessages_; }

  void recvDone() {
    uint8_t* request;
    uint32_t requestLen;
    readMemoryBuffer_.extractReadBuffer(&request, &requestLen);
    if (requestLen > 0) {
      std::shared_ptr<TMemoryBuffer> recvBuffer(new TMemoryBuffer(
          request, requestLen, TMemoryBuffer::TAKE_OWNERSHIP));
      readBuffers_.push_back(recvBuffer);
    } else if (request) {
      delete request;
    }

    // Read another message if we haven't read all of the messages yet
    if (!queued_ && readBuffers_.size() < writeMessages_.size()) {
      recvChannel_->recvMessage(
          std::bind(&MultiMessageSenderReceiver::recvDone, this),
          std::bind(&MultiMessageSenderReceiver::recvError, this),
          &readMemoryBuffer_);
    }
  }

  void recvError() { readError_ = true; }

  void writeSuccess() noexcept override {
    uint32_t sentSize =
        std::min(writeSize_, writeMemoryBuffer_.available_read());
    writeMemoryBuffer_.consume(sentSize);
    if (writeMemoryBuffer_.available_read() > 0) {
      scheduleNext();
    }
  }

  void writeErr(size_t /* bytesWritten */, const AsyncSocketException&) noexcept
      override {
    writeError_ = true;
  }

 private:
  void scheduleNext() {
    if (delayMS_.count() <= 0) {
      send();
    } else {
      scheduleTimeout(delayMS_);
    }
  }

  void timeoutExpired() noexcept override { send(); }

  void send() {
    uint32_t availableSize = writeMemoryBuffer_.available_read();
    const uint8_t* sendBufPtr =
        writeMemoryBuffer_.borrow(nullptr, &availableSize);
    uint32_t sendSize = std::min(writeSize_, availableSize);
    if (sendSize > 0) {
      socket_->write(this, sendBufPtr, sendSize);
    }
  }

  bool writeError_;
  bool readError_;
  AsyncSocket* socket_;
  vector<Message> writeMessages_;
  vector<std::shared_ptr<TMemoryBuffer>> readBuffers_;
  TMemoryBuffer writeMemoryBuffer_;
  TMemoryBuffer readMemoryBuffer_;
  uint32_t writeSize_;
  bool queued_;
  milliseconds delayMS_;
  std::shared_ptr<TAsyncChannel> recvChannel_;
};

class EventBaseAborter : public AsyncTimeout {
 public:
  EventBaseAborter(EventBase* eventBase, uint32_t timeoutMS)
      : AsyncTimeout(eventBase, AsyncTimeout::InternalEnum::INTERNAL),
        eventBase_(eventBase) {
    scheduleTimeout(timeoutMS);
  }

  void timeoutExpired() noexcept override {
    ADD_FAILURE() << "test timed out";
    eventBase_->terminateLoopSoon();
  }

 private:
  EventBase* eventBase_;
};

template <typename ChannelT>
class SocketPairTest {
 public:
  SocketPairTest() : eventBase_(), socketPair_() {
    auto socket0 = AsyncSocket::newSocket(
        &eventBase_, socketPair_.extractNetworkSocket0());
    socket0_ = socket0.get();

    auto socket1 = AsyncSocket::newSocket(
        &eventBase_, socketPair_.extractNetworkSocket1());
    socket1_ = socket1.get();

    channel0_ = ChannelT::newChannel(std::move(socket0));
    channel1_ = ChannelT::newChannel(std::move(socket1));
  }
  virtual ~SocketPairTest() {}

  void loop(uint32_t timeoutMS = 3000) {
    EventBaseAborter aborter(&eventBase_, timeoutMS);
    eventBase_.loop();
  }

  virtual void run() { runWithTimeout(3000); }

  virtual void runWithTimeout(uint32_t timeoutMS) {
    preLoop();
    loop(timeoutMS);
    postLoop();
  }

  virtual void preLoop() {}
  virtual void postLoop() {}

 protected:
  EventBase eventBase_;
  SocketPair socketPair_;
  AsyncSocket* socket0_;
  AsyncSocket* socket1_;
  std::shared_ptr<ChannelT> channel0_;
  std::shared_ptr<ChannelT> channel1_;
};

template <typename ChannelT>
class NeedsFrame {};

template <>
class NeedsFrame<TFramedAsyncChannel> {
 public:
  static bool value() { return true; }
};

template <>
class NeedsFrame<TBinaryAsyncChannel> {
 public:
  static bool value() { return false; }
};

///////////////////////////////////////////////////////////////////////////
// Test cases
//////////////////////////////////////////////////////////////////////////

template <typename ChannelT>
class SendRecvTest : public SocketPairTest<ChannelT> {
 public:
  explicit SendRecvTest(uint32_t msgLen) : msg_(msgLen) {}

  void preLoop() override {
    msg_.copyTo(&sendBuf_);
    sendCallback_.send(this->channel0_, &sendBuf_);
    recvCallback_.recv(this->channel1_, &recvBuf_);
  }

  void postLoop() override {
    CHECK_EQ(sendCallback_.getSendError(), 0);
    CHECK_EQ(sendCallback_.getSendDone(), 1);
    CHECK_EQ(recvCallback_.getRecvError(), 0);
    CHECK_EQ(recvCallback_.getRecvDone(), 1);
    msg_.checkEqual(&recvBuf_);
  }

 private:
  Message msg_;
  TMemoryBuffer sendBuf_;
  TMemoryBuffer recvBuf_;
  ChannelCallback sendCallback_;
  ChannelCallback recvCallback_;
};

TEST(TAsyncChannelTest, TestSendRecvFramed) {
  SendRecvTest<TFramedAsyncChannel>(1).run();
  SendRecvTest<TFramedAsyncChannel>(100).run();
  SendRecvTest<TFramedAsyncChannel>(1024 * 1024).run();
}

TEST(TAsyncChannelTest, TestSendRecvBinary) {
  SendRecvTest<TBinaryAsyncChannel>(1).run();
  SendRecvTest<TBinaryAsyncChannel>(100).run();
  SendRecvTest<TBinaryAsyncChannel>(1024 * 1024).run();
}

template <typename ChannelT>
class MultiSendRecvTest : public SocketPairTest<ChannelT> {
 public:
  MultiSendRecvTest(
      const MultiMessageSize& multiMessage,
      uint32_t writeTimes,
      bool queued = false,
      milliseconds delayMS = milliseconds(0))
      : multiMessageSenderReceiver_(
            &this->eventBase_,
            this->socket0_,
            multiMessage,
            NeedsFrame<ChannelT>::value(),
            writeTimes,
            queued,
            delayMS) {}

  void preLoop() override {
    multiMessageSenderReceiver_.initialize(this->channel1_);
  }

  void postLoop() override {
    CHECK_EQ(multiMessageSenderReceiver_.getReadError(), false);
    CHECK_EQ(multiMessageSenderReceiver_.getWriteError(), false);

    vector<std::shared_ptr<TMemoryBuffer>>& readBuffers =
        multiMessageSenderReceiver_.getReadBuffers();
    vector<Message>& writeMessages =
        multiMessageSenderReceiver_.getWriteMessages();
    CHECK_EQ(readBuffers.size(), writeMessages.size());
    for (size_t i = 0; i < writeMessages.size(); i++) {
      writeMessages[i].checkEqual(readBuffers[i].get());
    }
  }

 private:
  MultiMessageSenderReceiver multiMessageSenderReceiver_;
};

TEST(TAsyncChannelTest, TestMultiSendRecvBinary) {
  typedef MultiSendRecvTest<TBinaryAsyncChannel> MultiSendRecvBinaryTest;

  // size below 1024 for each message
  MultiMessageSize sizes(911, 911, 911, -1);

  // each time send one whole message below 1024
  MultiSendRecvBinaryTest(sizes, 3, 0).run();
  MultiSendRecvBinaryTest(sizes, 3, 2).run();

  // send all messages for one time
  MultiSendRecvBinaryTest(sizes, 1, 0).run();

  // each time send one and half message.
  MultiSendRecvBinaryTest(sizes, 2).run();
  MultiSendRecvBinaryTest(sizes, 2, 2).run();

  // size above 1024 for each message
  MultiMessageSize bigSizes(1911 * 1911, 1911 * 1911, 1911 * 1911, -1);

  // each time send one whole message above 1024
  MultiSendRecvBinaryTest(bigSizes, 3, 0).run();
  MultiSendRecvBinaryTest(bigSizes, 3, 2).run();

  // send all messages for one time
  MultiSendRecvBinaryTest(bigSizes, 1, 0).run();

  // each time send one and half message
  MultiSendRecvBinaryTest(bigSizes, 2).run();
  MultiSendRecvBinaryTest(bigSizes, 2, 2).run();
}

TEST(TAsyncChannelTest, TestMultiSendRecvBinaryQueued) {
  typedef MultiSendRecvTest<TBinaryAsyncChannel> MultiSendRecvBinaryTest;

  // size below 1024 for each message
  MultiMessageSize sizes(911, 911, 911, -1);

  // each time send one whole message below 1024
  MultiSendRecvBinaryTest(sizes, 3, true, milliseconds(0)).run();
  MultiSendRecvBinaryTest(sizes, 3, true, milliseconds(2)).run();

  // send all messages for one time
  MultiSendRecvBinaryTest(sizes, 1, true, milliseconds(0)).run();

  // each time send one and half message.
  MultiSendRecvBinaryTest(sizes, 2, true).run();
  MultiSendRecvBinaryTest(sizes, 2, true, milliseconds(2)).run();

  // size above 1024 for each message
  MultiMessageSize bigSizes(1911 * 1911, 1911 * 1911, 1911 * 1911, -1);

  // each time send one whole message above 1024
  MultiSendRecvBinaryTest(bigSizes, 3, true, milliseconds(0)).run();
  MultiSendRecvBinaryTest(bigSizes, 3, true, milliseconds(2)).run();

  // send all messages for one time
  MultiSendRecvBinaryTest(bigSizes, 1, true, milliseconds(0)).run();

  // each time send one and half message
  MultiSendRecvBinaryTest(bigSizes, 2, true).run();
  MultiSendRecvBinaryTest(bigSizes, 2, true, milliseconds(2)).run();
}

TEST(TAsyncChannelTest, TestMultiSendRecvFramed) {
  typedef MultiSendRecvTest<TFramedAsyncChannel> MultiSendRecvFramedTest;

  // size below 1024 for each message
  MultiMessageSize sizes(911, 911, 911, -1);

  // each time send one whole message below 1024
  MultiSendRecvFramedTest(sizes, 3, 0).run();
  MultiSendRecvFramedTest(sizes, 3, 2).run();

  // send all messages for one time
  MultiSendRecvFramedTest(sizes, 1, 0).run();

  // each time send one and half message.
  MultiSendRecvFramedTest(sizes, 2).run();
  MultiSendRecvFramedTest(sizes, 2, 2).run();

  // size above 1024 for each message
  MultiMessageSize bigSizes(1911 * 1911, 1911 * 1911, 1911 * 1911, -1);

  // each time send one whole message above 1024
  MultiSendRecvFramedTest(bigSizes, 3, 0).run();
  MultiSendRecvFramedTest(bigSizes, 3, 2).run();

  // send all messages for one time
  MultiSendRecvFramedTest(bigSizes, 1, 0).run();

  // each time send one and half message
  MultiSendRecvFramedTest(bigSizes, 2).run();
  MultiSendRecvFramedTest(bigSizes, 2, 2).run();
}

TEST(TAsyncChannelTest, TestMultiSendRecvFramedQueued) {
  typedef MultiSendRecvTest<TFramedAsyncChannel> MultiSendRecvBinaryTest;

  // size below 1024 for each message
  MultiMessageSize sizes(911, 911, 911, -1);

  // each time send one whole message below 1024
  MultiSendRecvBinaryTest(sizes, 3, true, milliseconds(0)).run();
  MultiSendRecvBinaryTest(sizes, 3, true, milliseconds(2)).run();

  // send all messages for one time
  MultiSendRecvBinaryTest(sizes, 1, true, milliseconds(0)).run();

  // each time send one and half message.
  MultiSendRecvBinaryTest(sizes, 2, true).run();
  MultiSendRecvBinaryTest(sizes, 2, true, milliseconds(2)).run();

  // size above 1024 for each message
  MultiMessageSize bigSizes(1911 * 1911, 1911 * 1911, 1911 * 1911, -1);

  // each time send one whole message above 1024
  MultiSendRecvBinaryTest(bigSizes, 3, true, milliseconds(0)).run();
  MultiSendRecvBinaryTest(bigSizes, 3, true, milliseconds(2)).run();

  // send all messages for one time
  MultiSendRecvBinaryTest(bigSizes, 1, true, milliseconds(0)).run();

  // each time send one and half message
  MultiSendRecvBinaryTest(bigSizes, 2, true).run();
  MultiSendRecvBinaryTest(bigSizes, 2, true, milliseconds(2)).run();
}

const int kRecvDelay = 200;
const int kTimeout = 50;

template <typename ChannelT>
class TimeoutQueuedTest : public SocketPairTest<ChannelT> {
 public:
  explicit TimeoutQueuedTest(uint32_t n_msgs = 3)
      : n_msgs_(n_msgs), start_(false), msg_(911) {}

  void preLoop() override {
    this->channel1_->setRecvTimeout(kRecvDelay * n_msgs_ + kTimeout);

    for (size_t i = 0; i < n_msgs_; i++) {
      // queue some reads 200ms apart
      this->eventBase_.tryRunAfterDelay(
          std::bind(&TimeoutQueuedTest<ChannelT>::recvMe, this),
          kRecvDelay * i);
    }

    this->eventBase_.tryRunAfterDelay(
        std::bind(&TimeoutQueuedTest<ChannelT>::sendMe, this),
        kRecvDelay * n_msgs_);
  }

  void sendMe() {
    // Send one message to test that the timeout for queued reads
    // adjusts based time recv was called.  Also tests that all queued
    // readers receive errors on the first timeout
    msg_.copyTo(&sendBuf_);
    sendCallback_.send(this->channel0_, &sendBuf_);
    start_.reset();
  }

  void recvMe() { recvCallback_.recv(this->channel1_, &readMemoryBuffer_); }

  void postLoop() override {
    CHECK_EQ(recvCallback_.getRecvError(), 2);
    CHECK_EQ(recvCallback_.getRecvDone(), 1);

    T_CHECK_TIMEOUT(
        start_,
        recvCallback_.getTimestamp(),
        milliseconds(n_msgs_ * kRecvDelay + kTimeout));
  }

 private:
  uint32_t n_msgs_;
  TimePoint start_;
  Message msg_;
  TMemoryBuffer sendBuf_;
  TMemoryBuffer readMemoryBuffer_;
  ChannelCallback sendCallback_;
  ChannelCallback recvCallback_;
};

TEST(TAsyncChannelTest, TestTimeoutQueued) {
  TimeoutQueuedTest<TFramedAsyncChannel>().run();
  TimeoutQueuedTest<TBinaryAsyncChannel>().run();
}

template <typename ChannelT>
class RecvChunksTest : public SocketPairTest<ChannelT> {
 public:
  explicit RecvChunksTest(
      const char* file,
      int line,
      const ChunkSchedule& schedule,
      milliseconds timeout = milliseconds(0),
      uint32_t msgLen = 1024 * 1024)
      : file_(file),
        line_(line),
        start_(false),
        timeout_(timeout),
        msg_(msgLen, NeedsFrame<ChannelT>::value()),
        sender_(&this->eventBase_, this->socket0_, &msg_, schedule) {}

  void preLoop() override {
    if (timeout_ > milliseconds(0)) {
      this->channel1_->setRecvTimeout(timeout_.count());
    }
    start_.reset();
    recvCallback_.recv(this->channel1_, &recvBuf_);
    sender_.start();
  }

  void postLoop() override {
    bool expectTimeout = false;
    milliseconds expectedMS = milliseconds(0);
    milliseconds tolerance = milliseconds(0);
    uint32_t expectedBytes = 0;
    for (ChunkSchedule::const_iterator it = sender_.getSchedule()->begin();
         it != sender_.getSchedule()->end();
         ++it) {
      // Allow 3ms of processing overhead for every scheduled event.
      tolerance += milliseconds(3);

      if (milliseconds(0) < timeout_ && timeout_ < it->delayMS) {
        // We expect to time out waiting for this chunk of data
        expectedMS += timeout_;
        expectTimeout = true;
        break;
      }

      expectedMS += it->delayMS;
      if (it->bytes < 0) {
        // The full message should be written
        expectedBytes = msg_.getLength();
      } else {
        expectedBytes += it->bytes;
      }
    }

    // Unframed transports require many more read callbacks to fully read the
    // data.  Add extra tolerance for the overhead in this case.
    //
    // The number of calls is roughly log(size/4096) / log(1.5)
    // (Since the code starts with an initial buffer size of 4096, and grows by
    // a factor of 1.5 each time it reallocates.)  Add extra 2 milliseconds of
    // tolerance for every expected call.
    if (!NeedsFrame<ChannelT>::value() && expectedBytes > 4096) {
      double numCalls = log(expectedBytes / 4096) / log(1.5);
      printf("expected %f calls for %u bytes\n", numCalls, expectedBytes);
      tolerance += milliseconds(static_cast<int64_t>(numCalls)) * 2;
    }

    if (expectTimeout) {
      // We should time out after expectedMS
      LOG(INFO) << "RecvChunksTest: testing for timeout in " << file_ << ":"
                << line_;
      EXPECT_EQ(sender_.error(), true);
      EXPECT_EQ(recvCallback_.getRecvError(), 1);
      EXPECT_EQ(recvCallback_.getRecvDone(), 0);
      EXPECT_EQ(this->channel1_->timedOut(), true);
      EXPECT_EQ(this->channel1_->error(), true);
      EXPECT_EQ(this->channel1_->good(), false);

      T_CHECK_TIMEOUT(
          start_, recvCallback_.getTimestamp(), expectedMS, tolerance);
    } else if (expectedBytes == 0) {
      // We should get EOF after expectedMS, before any data was ever sent
      //
      // This seems like a weird special case.  TAsyncChannel calls the normal
      // callback in this case, even though no message was received.  Maybe we
      // should consider changing this TAsyncChannel behavior?
      LOG(INFO) << "RecvChunksTest: testing for EOF with no data in " << file_
                << ":" << line_;
      EXPECT_EQ(sender_.error(), false);
      EXPECT_EQ(recvCallback_.getRecvError(), 0);
      EXPECT_EQ(recvCallback_.getRecvDone(), 1);
      EXPECT_EQ(this->channel1_->timedOut(), false);
      EXPECT_EQ(this->channel1_->error(), false);
      EXPECT_EQ(this->channel1_->good(), false);
      EXPECT_EQ(recvBuf_.available_read(), 0);

      T_CHECK_TIMEOUT(
          start_, recvCallback_.getTimestamp(), expectedMS, tolerance);
    } else if (expectedBytes < msg_.getLength()) {
      // We should get EOF after expectedMS
      LOG(INFO) << "RecvChunksTest: testing for EOF in " << file_ << ":"
                << line_;
      EXPECT_EQ(sender_.error(), false);
      EXPECT_EQ(recvCallback_.getRecvError(), 1);
      EXPECT_EQ(recvCallback_.getRecvDone(), 0);
      EXPECT_EQ(this->channel1_->timedOut(), false);
      EXPECT_EQ(this->channel1_->error(), false);
      EXPECT_EQ(this->channel1_->good(), false);

      T_CHECK_TIMEOUT(
          start_, recvCallback_.getTimestamp(), expectedMS, tolerance);
    } else {
      // We expect success after expectedMS
      LOG(INFO) << "RecvChunksTest: testing for success in " << file_ << ":"
                << line_;
      EXPECT_EQ(sender_.error(), false);
      EXPECT_EQ(recvCallback_.getRecvError(), 0);
      EXPECT_EQ(recvCallback_.getRecvDone(), 1);
      EXPECT_EQ(this->channel1_->timedOut(), false);
      EXPECT_EQ(this->channel1_->error(), false);
      EXPECT_EQ(this->channel1_->good(), true);
      msg_.checkEqual(&recvBuf_);

      T_CHECK_TIMEOUT(
          start_, recvCallback_.getTimestamp(), expectedMS, tolerance);
    }
  }

 private:
  std::string file_;
  int line_;
  TimePoint start_;
  milliseconds timeout_;
  Message msg_;
  ChunkSender sender_;
  TMemoryBuffer recvBuf_;
  ChannelCallback recvCallback_;
};

#define RECV_FRAME_TEST(...) \
  RecvChunksTest<TFramedAsyncChannel>(__FILE__, __LINE__, __VA_ARGS__)

TEST(TAsyncChannelTest, TestRecvFrameChunks) {
  // The frame header is 4 bytes.  Test sending each byte separately,
  // 5ms apart, followed by the body.
  ChunkSchedule s1{
      {1, milliseconds(5)},
      {1, milliseconds(5)},
      {1, milliseconds(5)},
      {1, milliseconds(5)},
      {100, milliseconds(10)},
      {-1, milliseconds(10)},
  };
  // Test reading the whole message
  RECV_FRAME_TEST(s1).run();
  // Setting the timeout to 15ms should still succeed--the code only times out
  // if no data is received for the specified period
  RECV_FRAME_TEST(s1, milliseconds(15)).run();

  // Test timing out before any data is sent
  RECV_FRAME_TEST(ChunkSchedule{{-1, milliseconds(50)}}, milliseconds(20))
      .run();
  // Test timing out after part of the frame header is sent
  RECV_FRAME_TEST(
      ChunkSchedule{{2, milliseconds(10)}, {-1, milliseconds(50)}},
      milliseconds(20))
      .run();
  // Test timing out after the frame header is sent
  RECV_FRAME_TEST(
      ChunkSchedule{{4, milliseconds(10)}, {-1, milliseconds(50)}},
      milliseconds(20))
      .run();
  // Test timing out after part of the body is snet
  RECV_FRAME_TEST(
      ChunkSchedule{{100, milliseconds(10)}, {-1, milliseconds(50)}},
      milliseconds(20))
      .run();

  // Test closing the connection before any data is sent
  RECV_FRAME_TEST(ChunkSchedule{{0, milliseconds(5)}}).run();
  // Test closing the connection after part of the frame header is sent
  RECV_FRAME_TEST(ChunkSchedule{{2, milliseconds(10)}, {0, milliseconds(5)}})
      .run();
  // Test closing the connection after the frame header is sent
  RECV_FRAME_TEST(ChunkSchedule{{4, milliseconds(10)}, {0, milliseconds(5)}})
      .run();
  // Test closing the connection after part of the body is snet
  RECV_FRAME_TEST(ChunkSchedule{{100, milliseconds(10)}, {0, milliseconds(5)}})
      .run();

  // Some various other schedules
  RECV_FRAME_TEST(ChunkSchedule{
                      {1, milliseconds(10)},
                      {1, milliseconds(10)},
                      {100, milliseconds(10)},
                      {-1, milliseconds(5)},
                  })
      .run();
}

#define RECV_BINARY_TEST(...) \
  RecvChunksTest<TBinaryAsyncChannel>(__FILE__, __LINE__, __VA_ARGS__)

TEST(TAsyncChannelTest, TestRecvBinaryChunks) {
  // Test sending the first four bytes byte separately,
  // 5ms apart, followed by the rest of the message.
  ChunkSchedule s1{
      {1, milliseconds(5)},
      {1, milliseconds(5)},
      {1, milliseconds(5)},
      {1, milliseconds(5)},
      {100, milliseconds(10)},
      {-1, milliseconds(10)},
  };
  // Test reading the whole message
  RECV_BINARY_TEST(s1).run();
  // Setting the timeout to 15ms should still succeed--the code only times out
  // if no data is received for the specified period
  RECV_BINARY_TEST(s1, milliseconds(15)).run();

  // Test timing out before any data is sent
  RECV_BINARY_TEST(ChunkSchedule{{-1, milliseconds(50)}}, milliseconds(20))
      .run();
  // Test timing out after part of the frame header is sent
  RECV_BINARY_TEST(
      ChunkSchedule{{2, milliseconds(10)}, {-1, milliseconds(50)}},
      milliseconds(20))
      .run();
  // Test timing out after the frame header is sent
  RECV_BINARY_TEST(
      ChunkSchedule{{4, milliseconds(10)}, {-1, milliseconds(50)}},
      milliseconds(20))
      .run();
  // Test timing out after part of the body is snet
  RECV_BINARY_TEST(
      ChunkSchedule{{100, milliseconds(10)}, {-1, milliseconds(50)}},
      milliseconds(20))
      .run();

  // Test closing the connection before any data is sent
  RECV_BINARY_TEST(ChunkSchedule{{0, milliseconds(5)}}).run();
  // Test closing the connection after sending 4 bytes
  RECV_BINARY_TEST(ChunkSchedule{{2, milliseconds(10)}, {0, milliseconds(5)}})
      .run();
  // Test closing the connection after sending 100 bytes
  RECV_BINARY_TEST(ChunkSchedule{{100, milliseconds(10)}, {0, milliseconds(5)}})
      .run();

  // Some various other schedules
  RECV_BINARY_TEST(ChunkSchedule{
                       {1, milliseconds(10)},
                       {1, milliseconds(10)},
                       {100, milliseconds(10)},
                       {-1, milliseconds(5)},
                   })
      .run();
}

template <typename ChannelT>
class SendTimeoutTest : public SocketPairTest<ChannelT> {
 public:
  explicit SendTimeoutTest(milliseconds timeout)
      : timeout_(timeout), start_(false), msg_(1024 * 1024) {}

  void preLoop() override {
    this->socket0_->setSendTimeout(timeout_.count());
    msg_.copyTo(&sendBuf_);
    sendCallback_.send(this->channel0_, &sendBuf_);
    // don't receive on the other socket

    start_.reset();
  }

  void postLoop() override {
    CHECK_EQ(sendCallback_.getSendError(), 1);
    CHECK_EQ(sendCallback_.getSendDone(), 0);
    T_CHECK_TIMEOUT(start_, sendCallback_.getTimestamp(), timeout_);
  }

 private:
  milliseconds timeout_;
  TimePoint start_;
  Message msg_;
  TMemoryBuffer sendBuf_;
  ChannelCallback sendCallback_;
};

TEST(TAsyncChannelTest, TestSendTimeoutFramed) {
  SendTimeoutTest<TFramedAsyncChannel>(milliseconds(25)).run();
  SendTimeoutTest<TFramedAsyncChannel>(milliseconds(100)).run();
  SendTimeoutTest<TFramedAsyncChannel>(milliseconds(250)).run();
}

TEST(TAsyncChannelTest, TestSendTimeoutBinary) {
  SendTimeoutTest<TBinaryAsyncChannel>(milliseconds(25)).run();
  SendTimeoutTest<TBinaryAsyncChannel>(milliseconds(100)).run();
  SendTimeoutTest<TBinaryAsyncChannel>(milliseconds(250)).run();
}

template <typename ChannelT>
class SendClosedTest : public SocketPairTest<ChannelT> {
 public:
  explicit SendClosedTest(milliseconds closeTimeout = milliseconds(5))
      : closeTimeout_(closeTimeout), start_(false), msg_(1024 * 1024) {}

  void preLoop() override {
    msg_.copyTo(&sendBuf_);
    sendCallback_.send(this->channel0_, &sendBuf_);

    // Close the other socket after 25ms
    this->eventBase_.tryRunAfterDelay(
        std::bind(&AsyncSocket::close, this->socket1_), closeTimeout_.count());

    start_.reset();
  }

  void postLoop() override {
    CHECK_EQ(sendCallback_.getSendError(), 1);
    CHECK_EQ(sendCallback_.getSendDone(), 0);
    T_CHECK_TIMEOUT(start_, sendCallback_.getTimestamp(), closeTimeout_);
  }

 private:
  milliseconds closeTimeout_;
  TimePoint start_;
  Message msg_;
  TMemoryBuffer sendBuf_;
  ChannelCallback sendCallback_;
};

TEST(TAsyncChannelTest, TestSendClosed) {
  SendClosedTest<TFramedAsyncChannel>().run();
  SendClosedTest<TBinaryAsyncChannel>().run();
}
