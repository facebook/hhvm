/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBaseManager.h>
#include <limits>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/QPACKDecoderCodec.h>
#include <proxygen/lib/http/codec/QPACKEncoderCodec.h>
#include <proxygen/lib/http/session/HQDownstreamSession.h>
#include <proxygen/lib/http/session/HQUpstreamSession.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>

#define ALPN_HQ (alpn.find("h3") == 0)

namespace {
constexpr unsigned int kTransactionTimeout = 500;
constexpr unsigned int kConnectTimeout = 500;
constexpr size_t kQPACKTestDecoderMaxTableSize = 2048;
constexpr std::size_t kUnlimited = std::numeric_limits<std::size_t>::max();
const proxygen::hq::PushId kUnknownPushId =
    std::numeric_limits<uint64_t>::max();
constexpr proxygen::hq::PushId kInitialPushId = 12345;
constexpr uint64_t kPushIdIncrement = 1;
constexpr uint64_t kDefaultUnidirStreamCredit = 3;
} // namespace

struct TestParams {
  std::string alpn_;
  bool createQPACKStreams_{true};
  bool shouldSendSettings_{true};
  uint64_t unidirectionalStreamsCredit{kDefaultUnidirStreamCredit};
  std::size_t numBytesOnPushStream{kUnlimited};
  bool expectOnTransportReady{true};
  bool datagrams_{false};
  bool webTransport_{false};
  bool checkUniridStreamCallbacks{true};
};

std::string prBodyScriptToName(const std::vector<uint8_t>& bodyScript);

size_t encodeQuicIntegerWithAtLeast(uint64_t value,
                                    uint8_t atLeast,
                                    folly::io::QueueAppender& appender);

std::string paramsToTestName(const testing::TestParamInfo<TestParams>& info);

size_t generateStreamPreface(folly::IOBufQueue& writeBuf,
                             proxygen::hq::UnidirectionalStreamType type);

folly::Optional<std::pair<proxygen::hq::UnidirectionalStreamType, size_t>>
parseStreamPreface(folly::io::Cursor& cursor, std::string alpn);

void parseReadData(proxygen::hq::HQUnidirectionalCodec* codec,
                   folly::IOBufQueue& readBuf,
                   std::unique_ptr<folly::IOBuf> buf);

void createControlStream(quic::MockQuicSocketDriver* socketDriver,
                         quic::StreamId id,
                         proxygen::hq::UnidirectionalStreamType streamType);

class HQSessionTest
    : public testing::TestWithParam<TestParams>
    , public quic::MockQuicSocketDriver::LocalAppCallback
    , public proxygen::hq::HQUnidirectionalCodec::Callback {

 public:
  void SetUp() override {
    folly::EventBaseManager::get()->clearEventBase();
    proxygen::HTTPSession::setDefaultWriteBufferLimit(65536);
    proxygen::HTTP2PriorityQueue::setNodeLifetime(std::chrono::milliseconds(2));
    EXPECT_CALL(infoCb_, onTransactionAttached(testing::_))
        .WillRepeatedly([this]() { onTransactionSymmetricCounter++; });
    EXPECT_CALL(infoCb_, onTransactionAttached(testing::_))
        .WillRepeatedly([this]() { onTransactionSymmetricCounter--; });
  }

  void TearDown() override {
    EXPECT_EQ(onTransactionSymmetricCounter, 0);
  }

 protected:
  explicit HQSessionTest(
      proxygen::TransportDirection direction,
      folly::Optional<TestParams> overrideParams = folly::none)
      : direction_(direction),
        overrideParams_(overrideParams),
        qpackEncoderCodec_(qpackCodec_, *this),
        qpackDecoderCodec_(qpackCodec_, *this),
        controllerContainer_(GetParam())

  {
    if (direction_ == proxygen::TransportDirection::DOWNSTREAM) {
      hqSession_ = new proxygen::HQDownstreamSession(
          std::chrono::milliseconds(kTransactionTimeout),
          &controllerContainer_.mockController,
          proxygen::mockTransportInfo,
          nullptr);
      nextUnidirectionalStreamId_ = 2;
    } else if (direction_ == proxygen::TransportDirection::UPSTREAM) {
      hqSession_ = new proxygen::HQUpstreamSession(
          std::chrono::milliseconds(kTransactionTimeout),
          std::chrono::milliseconds(kConnectTimeout),
          &controllerContainer_.mockController,
          proxygen::mockTransportInfo,
          nullptr);
      nextUnidirectionalStreamId_ = 3;
    } else {
      LOG(FATAL) << "wrong TransportEnum";
    }

    if (GetParam().datagrams_) {
      egressSettings_.setSetting(proxygen::SettingsId::_HQ_DATAGRAM, 1);
    }
    if (GetParam().webTransport_) {
      egressSettings_.setSetting(proxygen::SettingsId::_HQ_DATAGRAM, 1);
      egressSettings_.setSetting(proxygen::SettingsId::ENABLE_CONNECT_PROTOCOL,
                                 1);
      egressSettings_.setSetting(proxygen::SettingsId::ENABLE_WEBTRANSPORT, 1);
    }

    egressControlCodec_ = std::make_unique<proxygen::hq::HQControlCodec>(
        nextUnidirectionalStreamId_,
        direction_ == proxygen::TransportDirection::DOWNSTREAM
            ? proxygen::TransportDirection::UPSTREAM
            : proxygen::TransportDirection::DOWNSTREAM,
        proxygen::hq::StreamDirection::EGRESS,
        egressSettings_);
    socketDriver_ = std::make_unique<quic::MockQuicSocketDriver>(
        &eventBase_,
        hqSession_,
        hqSession_,
        direction_ == proxygen::TransportDirection::DOWNSTREAM
            ? quic::MockQuicSocketDriver::TransportEnum::SERVER
            : quic::MockQuicSocketDriver::TransportEnum::CLIENT,
        getProtocolString());

    hqSession_->setSocket(socketDriver_->getSocket());

    hqSession_->setEgressSettings(egressSettings_.getAllSettings());
    qpackCodec_.setEncoderHeaderTableSize(1024);
    qpackCodec_.setDecoderHeaderTableMaxSize(kQPACKTestDecoderMaxTableSize);
    hqSession_->setInfoCallback(&infoCb_);

    socketDriver_->setMaxUniStreams(GetParam().unidirectionalStreamsCredit);

    EXPECT_CALL(infoCb_, onRead(testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber());

    size_t ctrlStreamCount = 1;
    size_t qpackStreamCount = (GetParam().createQPACKStreams_) ? 2 : 0;
    numCtrlStreams_ = ctrlStreamCount + qpackStreamCount;
    socketDriver_->setLocalAppCallback(this);

    if (GetParam().checkUniridStreamCallbacks &&
        GetParam().unidirectionalStreamsCredit >= numCtrlStreams_ &&
        GetParam().alpn_.find("h3") == 0) {
      auto dirModifier =
          (direction_ == proxygen::TransportDirection::DOWNSTREAM) ? 0 : 1;
      EXPECT_CALL(infoCb_, onWrite(testing::_, testing::_))
          .Times(testing::AtLeast(numCtrlStreams_));
      for (auto i = 0; i < numCtrlStreams_; i++) {
        folly::Optional<proxygen::HTTPCodec::StreamID> expectedStreamID =
            i * 4 + 2 + dirModifier;
        EXPECT_CALL(infoCb_, onRead(testing::_, testing::_, expectedStreamID))
            .Times(testing::AtLeast(1));
      }
    }

    quic::QuicSocket::TransportInfo transportInfo;
    transportInfo.srtt = std::chrono::microseconds(100);
    transportInfo.congestionWindow = 1500;

    EXPECT_CALL(*socketDriver_->getSocket(), getTransportInfo())
        .WillRepeatedly(testing::Return(transportInfo));
  }

  bool createControlStreams() {
    // NOTE: this is NOT the stream credit advertised by the peer.
    // this is the number of uni streams that we allow the peer to open. if that
    // is not enough for the control streams, onTransportReady drops the
    // connection, so don't try to create or write to new streams.
    if (GetParam().unidirectionalStreamsCredit < numCtrlStreams_) {
      return false;
    }
    if (GetParam().alpn_.find("h3") != 0) {
      // this function can be called when alpn negotiation failed
      return false;
    }
    connControlStreamId_ = nextUnidirectionalStreamId();
    createControlStream(socketDriver_.get(),
                        connControlStreamId_,
                        proxygen::hq::UnidirectionalStreamType::CONTROL);
    if (GetParam().createQPACKStreams_) {
      createControlStream(
          socketDriver_.get(),
          nextUnidirectionalStreamId(),
          proxygen::hq::UnidirectionalStreamType::QPACK_ENCODER);
      createControlStream(
          socketDriver_.get(),
          nextUnidirectionalStreamId(),
          proxygen::hq::UnidirectionalStreamType::QPACK_DECODER);
    }
    if (GetParam().shouldSendSettings_) {
      sendSettings();
    }
    return true;
  }

  void sendSettings() {
    folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
    egressControlCodec_->generateSettings(writeBuf);
    socketDriver_->addReadEvent(
        connControlStreamId_, writeBuf.move(), std::chrono::milliseconds(0));
  }

  const std::string getProtocolString() const {
    if (GetParam().alpn_ == "h3") {
      return proxygen::kH3;
    }
    return GetParam().alpn_;
  }

  void readCallback(quic::StreamId id,
                    std::unique_ptr<folly::IOBuf> buf) override {
  }

  void unidirectionalReadCallback(quic::StreamId id,
                                  std::unique_ptr<folly::IOBuf> buf) override {
    // check for control streams
    if (buf->empty()) {
      return;
    }

    auto it = controlStreams_.find(id);
    if (it == controlStreams_.end()) {
      folly::io::Cursor cursor(buf.get());
      auto preface = parseStreamPreface(cursor, getProtocolString());
      CHECK(preface) << "Preface can not be parsed protocolString="
                     << getProtocolString();
      switch (preface->first) {
        case proxygen::hq::UnidirectionalStreamType::CONTROL:
          ingressControlCodec_ = std::make_unique<proxygen::hq::HQControlCodec>(
              id,
              proxygen::TransportDirection::UPSTREAM,
              proxygen::hq::StreamDirection::INGRESS,
              ingressSettings_,
              preface->first);
          ingressControlCodec_->setCallback(&httpCallbacks_);
          break;
        case proxygen::hq::UnidirectionalStreamType::QPACK_ENCODER:
        case proxygen::hq::UnidirectionalStreamType::QPACK_DECODER:
          break;
        case proxygen::hq::UnidirectionalStreamType::PUSH: {
          auto pushIt = pushes_.find(id);
          if (pushIt == pushes_.end()) {
            auto pushId = quic::decodeQuicInteger(cursor);
            if (pushId) {
              pushes_.emplace(id, pushId->first);
            }
          }
        }
          return;
        case proxygen::hq::UnidirectionalStreamType::WEBTRANSPORT:
          return;
        default:
          CHECK(false) << "Unknown stream preface=" << preface->first;
      }
      socketDriver_->sock_->setControlStream(id);
      auto res = controlStreams_.emplace(id, preface->first);
      it = res.first;
      buf->trimStart(preface->second);
      if (buf->empty()) {
        return;
      }
    }

    switch (it->second) {
      case proxygen::hq::UnidirectionalStreamType::CONTROL:
        parseReadData(
            ingressControlCodec_.get(), ingressControlBuf_, std::move(buf));
        break;
      case proxygen::hq::UnidirectionalStreamType::QPACK_ENCODER:
        parseReadData(&qpackEncoderCodec_, encoderReadBuf_, std::move(buf));
        break;
      case proxygen::hq::UnidirectionalStreamType::QPACK_DECODER:
        parseReadData(&qpackDecoderCodec_, decoderReadBuf_, std::move(buf));
        break;
      case proxygen::hq::UnidirectionalStreamType::PUSH:
        VLOG(4) << "Ingress push streams should not go through "
                << "the unidirectional read path";
        break;
      default:
        CHECK(false) << "Unknown stream type=" << it->second;
    }
  }

  void onError(proxygen::HTTPCodec::StreamID streamID,
               const proxygen::HTTPException& error,
               bool /*newTxn*/) override {
    LOG(FATAL) << __func__ << " streamID=" << streamID
               << " error=" << error.what();
  }

  quic::StreamId nextUnidirectionalStreamId() {
    auto id = nextUnidirectionalStreamId_;
    nextUnidirectionalStreamId_ += 4;
    return id;
  }

  struct MockControllerContainer {
    explicit MockControllerContainer(TestParams params) {
      EXPECT_CALL(mockController, getHeaderIndexingStrategy())
          .WillRepeatedly(testing::Return(
              proxygen::HeaderIndexingStrategy::getDefaultInstance()));
      testing::InSequence s;
      EXPECT_CALL(mockController, attachSession(testing::_));
      if (params.expectOnTransportReady) {
        EXPECT_CALL(mockController, onTransportReady(testing::_));
      }
      EXPECT_CALL(mockController, detachSession(testing::_));
    }
    testing::StrictMock<proxygen::MockController> mockController;
  };

  testing::StrictMock<proxygen::MockController>& getMockController() {
    return controllerContainer_.mockController;
  }

 public:
  quic::MockQuicSocketDriver* getSocketDriver() {
    return socketDriver_.get();
  }

  proxygen::HQSession* getSession() {
    return hqSession_;
  }

  void setSessionDestroyCallback(
      folly::Function<void(const proxygen::HTTPSessionBase&)> cb) {
    EXPECT_CALL(infoCb_, onDestroy(testing::_))
        .WillOnce(testing::Invoke(
            [&](const proxygen::HTTPSessionBase&) { cb(*hqSession_); }));
  }

  const TestParams& GetParam() const {
    if (overrideParams_) {
      return *overrideParams_;
    } else {
      const testing::TestWithParam<TestParams>* base = this;
      return base->GetParam();
    }
  }

  std::unique_ptr<folly::IOBuf> getH3Datagram(
      uint64_t streamId, std::unique_ptr<folly::IOBuf> datagram) {
    // Prepend the H3 Datagram header to the datagram payload
    // HTTP/3 Datagram {
    //   Quarter Stream ID (i),
    //   [Context ID (i)],
    //   HTTP/3 Datagram Payload (..),
    // }
    quic::Buf headerBuf = quic::Buf(folly::IOBuf::create(0));
    quic::BufAppender appender(headerBuf.get(),
                               proxygen::kMaxDatagramHeaderSize);
    auto streamIdRes = quic::encodeQuicInteger(
        streamId / 4, [&](auto val) { appender.writeBE(val); });
    if (streamIdRes.hasError()) {
      return nullptr;
    }
    // Always use context-id = 0 for now
    auto ctxIdRes =
        quic::encodeQuicInteger(0, [&](auto val) { appender.writeBE(val); });
    if (ctxIdRes.hasError()) {
      return nullptr;
    }
    quic::BufQueue queue(std::move(headerBuf));
    queue.append(std::move(datagram));
    return queue.move();
  }

 protected:
  proxygen::TransportDirection direction_;
  folly::Optional<TestParams> overrideParams_;
  // Unidirectional Stream Codecs used for Ingress Only
  proxygen::hq::QPACKEncoderCodec qpackEncoderCodec_;
  proxygen::hq::QPACKDecoderCodec qpackDecoderCodec_;
  // Read/WriteBufs for QPACKCodec, one for the encoder, one for the decoder
  folly::IOBufQueue encoderReadBuf_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue decoderReadBuf_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue encoderWriteBuf_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue decoderWriteBuf_{folly::IOBufQueue::cacheChainLength()};

  folly::EventBase eventBase_;
  proxygen::HQSession* hqSession_;
  MockControllerContainer controllerContainer_;
  std::unique_ptr<quic::MockQuicSocketDriver> socketDriver_;
  // One QPACKCodec per session, handles both encoder and decoder
  proxygen::QPACKCodec qpackCodec_;
  std::map<quic::StreamId, proxygen::hq::UnidirectionalStreamType>
      controlStreams_;
  // Ingress Control Stream
  std::unique_ptr<proxygen::hq::HQControlCodec> ingressControlCodec_;
  folly::IOBufQueue ingressControlBuf_{folly::IOBufQueue::cacheChainLength()};
  proxygen::HTTPSettings egressSettings_{
      {proxygen::SettingsId::HEADER_TABLE_SIZE, kQPACKTestDecoderMaxTableSize},
      {proxygen::SettingsId::MAX_HEADER_LIST_SIZE, 655335},
      {proxygen::SettingsId::_HQ_QPACK_BLOCKED_STREAMS, 100}};
  proxygen::HTTPSettings ingressSettings_;
  proxygen::FakeHTTPCodecCallback httpCallbacks_;
  uint8_t numCtrlStreams_{0};
  quic::StreamId connControlStreamId_;
  testing::NiceMock<proxygen::MockHTTPSessionInfoCallback> infoCb_;
  quic::StreamId nextUnidirectionalStreamId_;
  // Egress Control Stream
  std::unique_ptr<proxygen::hq::HQControlCodec> egressControlCodec_;
  folly::F14FastMap<quic::StreamId, proxygen::hq::PushId> pushes_;
  uint64_t onTransactionSymmetricCounter{0};
};
