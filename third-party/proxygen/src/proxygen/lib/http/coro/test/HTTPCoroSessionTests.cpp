/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/test/HTTPCoroSessionTests.h"
#include <folly/logging/xlog.h>

#include <proxygen/lib/http/codec/HTTPCodecFactory.h>

using namespace testing;
using namespace proxygen;

namespace proxygen::coro::test {

std::string paramsToTestName(const testing::TestParamInfo<TestParams> &info) {
  switch (info.param.codecProtocol) {
    case CodecProtocol::HTTP_1_1:
      return "h1";
    case CodecProtocol::HTTP_2:
      return "h2";
    case CodecProtocol::HQ:
      return "h3";
    default:
      XLOG(FATAL);
      return "";
  }
}

void HTTPCoroSessionTest::initCodec() {
  if (isHQ()) {
    auto codec = std::make_unique<hq::HQMultiCodec>(oppositeDirection());
    multiCodec_ = codec.get();
    multiCodec_->setQPACKEncoderMaxDataFn(
        []() -> uint64_t { return std::numeric_limits<uint64_t>::max(); });
    auto headerTableSize = (GetParam().useDynamicTable) ? 4096 : 0;
    auto maxBlocking = 100;
    multiCodec_->getEgressSettings()->setSetting(
        proxygen::SettingsId::HEADER_TABLE_SIZE, headerTableSize);
    multiCodec_->getEgressSettings()->setSetting(
        proxygen::SettingsId::_HQ_QPACK_BLOCKED_STREAMS, maxBlocking);
    // Apply the egress settings to the simulated codec
    multiCodec_->getQPACKCodec().setDecoderHeaderTableMaxSize(headerTableSize);
    multiCodec_->getQPACKCodec().setMaxBlocking(maxBlocking);
    peerCodec_ = std::move(codec);
  } else {
    peerCodec_ = HTTPCodecFactory::getCodec(GetParam().codecProtocol,
                                            oppositeDirection());
  }
  auto settings = peerCodec_->getEgressSettings();
  if (direction_ == TransportDirection::DOWNSTREAM) {
    setTestCodecSetting(settings, SettingsId::ENABLE_PUSH, 1);
  }
  setTestCodecSetting(settings, SettingsId::INITIAL_WINDOW_SIZE, 65535);
  setTestCodecSetting(settings, SettingsId::MAX_CONCURRENT_STREAMS, 10);
  peerCodec_->setCallback(&callbacks_);
}

void HTTPCoroSessionTest::setUp(std::shared_ptr<HTTPHandler> handler) {
  wangle::TransportInfo tinfo;
  tinfo.appProtocol = std::make_shared<std::string>("blarf");

  if (isHQ()) {
    muxTransport_ =
        std::make_unique<TestCoroMultiplexTransport>(&evb_, direction_);
    transport_ = muxTransport_.get();
    // Other MQSD server setup
    auto codec = std::make_unique<hq::HQMultiCodec>(direction_);
    auto headerTableSize = (GetParam().useDynamicTable) ? 4096 : 0;
    codec->getEgressSettings()->setSetting(SettingsId::HEADER_TABLE_SIZE,
                                           headerTableSize);
    // Apply the SETTINGS to the simulated codec before the first request
    multiCodec_->getQPACKCodec().setEncoderHeaderTableSize(
        codec->getEgressSettings()
            ->getSetting(SettingsId::HEADER_TABLE_SIZE)
            ->value);
    multiCodec_->getQPACKCodec().setMaxVulnerable(
        codec->getEgressSettings()
            ->getSetting(SettingsId::_HQ_QPACK_BLOCKED_STREAMS)
            ->value);
    codec_ = codec.get();
    if (handler) {
      session_ =
          HTTPCoroSession::makeDownstreamCoroSession(muxTransport_->getSocket(),
                                                     handler,
                                                     std::move(codec),
                                                     std::move(tinfo));
    } else {
      session_ = HTTPCoroSession::makeUpstreamCoroSession(
          muxTransport_->getSocket(), std::move(codec), std::move(tinfo));
    }
  } else {
    auto transport =
        std::make_unique<TestUniplexTransport>(&evb_, &transportState_);
    transport_ = transport.get();
    auto codec =
        HTTPCodecFactory::getCodec(GetParam().codecProtocol, direction_);
    codec_ = codec.get();
    if (handler) {
      session_ = HTTPCoroSession::makeDownstreamCoroSession(
          std::move(transport), handler, std::move(codec), std::move(tinfo));
    } else {
      session_ = HTTPCoroSession::makeUpstreamCoroSession(
          std::move(transport), std::move(codec), std::move(tinfo));
    }
  }
  peerCodec_->generateConnectionPreface(writeBuf_);
  peerCodec_->generateSettings(writeBuf_);

  transport_->addReadEvent(writeBuf_.move(), false);

  session_->setConnectionReadTimeout(std::chrono::seconds(1));
  session_->setStreamReadTimeout(std::chrono::seconds(1));
  session_->setWriteTimeout(std::chrono::seconds(1));
  session_->setSessionStats(&fakeSessionStats_);
  session_->addLifecycleObserver(&lifecycleObs_);
  // mimic EventBase::add() when ::inRunningEventBase == false
  evb_.runInEventBaseThreadAlwaysEnqueue([this]() {
    folly::coro::co_withCancellation(cancellationSource_.getToken(),
                                     session_->run())
        .startInlineUnsafe();
  });
}

void HTTPCoroSessionTest::resetStream(HTTPCodec::StreamID id,
                                      ErrorCode code,
                                      bool stopSending) {
  if (isHQ()) {
    auto h3Code =
        HTTPErrorCode2HTTP3ErrorCode(ErrorCode2HTTPErrorCode(code), false);
    muxTransport_->socketDriver_.addReadError(id, h3Code);
    auto sock = muxTransport_->socketDriver_.getSocket();
    // If it's a downstream test, then the framework is a client
    bool isClient = direction_ == TransportDirection::DOWNSTREAM;
    if (sock->isBidirectionalStream(id) ||
        sock->isClientStream(id) == isClient) {
      muxTransport_->socketDriver_.addReadError(id, h3Code);
    }
    if ((stopSending && sock->isBidirectionalStream(id)) ||
        sock->isClientStream(id) != isClient) {
      muxTransport_->socketDriver_.addStopSending(id, h3Code);
    }
  } else {
    peerCodec_->generateRstStream(writeBuf_, id, code);
    transport_->addReadEvent(id, writeBuf_.move(), false);
  }
}

void HTTPCoroSessionTest::generateGoaway(HTTPCodec::StreamID id,
                                         ErrorCode code) {
  if (isHQ()) {
    if (direction_ == TransportDirection::DOWNSTREAM) {
      // Downstream test, peer codec is upstream
      // push ID, min unseen + 1
      id += 1;
    } else {
      // stream ID, min unseen + 4
      id += 4;
    }
  }
  peerCodec_->generateGoaway(writeBuf_, id, code);
  transport_->addReadEvent(writeBuf_.move(), false);
}

void HTTPCoroSessionTest::generateGoaway() {
  peerCodec_->generateGoaway(writeBuf_);
  transport_->addReadEvent(writeBuf_.move(), false);
}

void HTTPCoroSessionTest::windowUpdate(HTTPCodec::StreamID id, uint32_t delta) {
  if (isHQ()) {
    auto fc =
        muxTransport_->socketDriver_.getSocket()->getStreamFlowControl(id);
    muxTransport_->socketDriver_.setStreamFlowControlWindow(
        id, fc->sendWindowAvailable + delta);
  } else {
    peerCodec_->generateWindowUpdate(writeBuf_, id, delta);
    transport_->addReadEvent(id, writeBuf_.move(), false);
  }
}

void HTTPCoroSessionTest::windowUpdate(uint32_t delta) {
  if (isHQ()) {
    auto fc =
        muxTransport_->socketDriver_.getSocket()->getConnectionFlowControl();
    muxTransport_->socketDriver_.setConnectionFlowControlWindow(
        fc->sendWindowAvailable + delta);
  } else {
    peerCodec_->generateWindowUpdate(writeBuf_, 0, delta);
    transport_->addReadEvent(writeBuf_.move(), false);
  }
}

void HTTPCoroSessionTest::expectStreamAbort(HTTPCodec::StreamID id,
                                            ErrorCode code) {
  if (isHQ()) {
    EXPECT_EQ(muxTransport_->socketDriver_.streams_[id].writeState,
              quic::MockQuicSocketDriver::ERROR);
    EXPECT_EQ(
        muxTransport_->socketDriver_.streams_[id].error,
        HTTPErrorCode2HTTP3ErrorCode(ErrorCode2HTTPErrorCode(code), false));
  } else {
    EXPECT_CALL(callbacks_, onAbort(id, code));
  }
}

void HTTPCoroSessionTest::expectGoaway(HTTPCodec::StreamID id, ErrorCode code) {
  if (isHQ()) {
    id = ((id / 2) + 1) * 4;
    EXPECT_CALL(callbacks_, onGoaway(id, _, _));
  } else {
    EXPECT_CALL(callbacks_, onGoaway(id, code, _));
  }
}

} // namespace proxygen::coro::test
// Tests to write
// Response before request
// messageBegin gets dup stream
// onHeadersComplete for missing stream
// receiving body conn flow control error
// trailers stream not found
// onError: stream 0 or no codec status
// onAbort stream not found
// flow control errors
// responseSource provided for egress complete stream
// handler returned nullptr but not egress complete
// error reading response headers
// response headers only
// response body error
// response body egress complete
// resetStream with non-empty buffer queue
// read error
// read timeout
// bytesParsed == 0
// timeout waiting for write event
// write cancelled
// write timeoout
// write error
// write error cleanup
// writable stream is missing
// egress trailers
// read/ingress first, egress last (normal server path)
// read/egress first, ingress last (normal client path)
// ingress/egress first, read last (server receive reset stream)
// Info callbacks for settings ack, ingress error and flow controlw window full

// TODO: push promise with push disabled, still decode push promise?
// HTTP/1.0 response with no content-length - have to send a FIN to terminate
