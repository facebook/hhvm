/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/HqWtSession.h>

namespace {
using namespace proxygen;
using namespace proxygen::detail;

struct H3CapsuleCodecCb final : public WebTransportCapsuleCodec::Callback {
  H3WtSession& wtSess;
  explicit H3CapsuleCodecCb(H3WtSession& wtSess) : wtSess(wtSess) {
  }

  void onWTMaxDataCapsule(WTMaxDataCapsule c) noexcept override {
    VLOG(4) << __func__;
    wtSess.onConnMaxData({.maxData = c.maximumData});
  }
  void onWTMaxStreamsBidiCapsule(WTMaxStreamsCapsule c) noexcept override {
    VLOG(4) << __func__;
    wtSess.onMaxStreams(WtStreamManager::MaxStreamsBidi{c.maximumStreams});
  }
  void onWTMaxStreamsUniCapsule(WTMaxStreamsCapsule c) noexcept override {
    VLOG(4) << __func__;
    wtSess.onMaxStreams(WtStreamManager::MaxStreamsUni{c.maximumStreams});
  }
  void onDrainWTSessionCapsule(
      DrainWebTransportSessionCapsule) noexcept override {
    VLOG(4) << __func__;
    wtSess.onDrainSession({});
  }
  void onCloseWTSessionCapsule(
      CloseWebTransportSessionCapsule c) noexcept override {
    VLOG(4) << __func__;
    wtSess.onCloseSession(WtStreamManager::CloseSession{
        c.applicationErrorCode, std::move(c.applicationErrorMessage)});
  }
  void onConnectionError(CapsuleCodec::ErrorCode error) noexcept override {
    VLOG(4) << __func__;
    onCloseWTSessionCapsule({uint8_t(error), "onConnectionError"});
  }

  // ignored callbacks
  void onWTMaxStreamDataCapsule(WTMaxStreamDataCapsule) noexcept override {
  }
  void onWTResetStreamCapsule(WTResetStreamCapsule) noexcept override {
  }
  void onWTStopSendingCapsule(WTStopSendingCapsule) noexcept override {
  }
  void onWTStreamCapsule(WTStreamCapsule) noexcept override {
  }
  void onWTStreamDataBlockedCapsule(
      WTStreamDataBlockedCapsule) noexcept override {
  }
  void onWTStreamsBlockedBidiCapsule(
      WTStreamsBlockedCapsule) noexcept override {
  }
  void onWTStreamsBlockedUniCapsule(WTStreamsBlockedCapsule) noexcept override {
  }
  void onPaddingCapsule(PaddingCapsule) noexcept override {
  }
  void onDatagramCapsule(DatagramCapsule) noexcept override {
  }
  void onWTDataBlockedCapsule(WTDataBlockedCapsule) noexcept override {
  }
};

}; // namespace

namespace proxygen::detail {

struct WtReadLooper : public WtLooper {
  WtReadLooper(folly::EventBase* evb,
               WebTransportTxnHandler& wtTxnHandler,
               HqWtSession& wtSess) noexcept
      : WtLooper(evb, Type::Read),
        wtSess_(wtSess),
        wtTxnHandler_(wtTxnHandler),
        codecCb_(wtSess.getH3WtSession()),
        codec_(&codecCb_, CodecVersion::H3) {
  }
  ~WtReadLooper() override = default;

 private:
  void runLoopCallback() noexcept override;
  HqWtSession& wtSess_;
  WebTransportTxnHandler& wtTxnHandler_;
  H3CapsuleCodecCb codecCb_;
  WebTransportCapsuleCodec codec_;
};

void WtReadLooper::runLoopCallback() noexcept {
  auto [buf, eom] = wtTxnHandler_.moveBufferedIngress();
  VLOG(4) << "WtReadLooper buf=" << buf.get() << "; eom=" << eom
          << "; ex=" << wtTxnHandler_.ex_;
  codec_.onIngress(std::move(buf), eom);
  const bool ingressDone = eom || wtTxnHandler_.ex_;
  if (ingressDone) {
    wtSess_.getH3WtSession().onCloseSession(
        {WebTransport::kSessionGone, "rx ingress eom"});
    wtSess_.readsDone();
  }
}

struct WtWriteLooper
    : public WtLooper
    , public H3ConnectStreamCallback {
  WtWriteLooper(folly::EventBase* evb,
                WebTransportTxnHandler& wtTxnHandler,
                WtHttpSession& wtSess) noexcept
      : WtLooper(evb, Type::Write),
        H3ConnectStreamCallback(buf_),
        wtTxnHandler_(wtTxnHandler),
        wtSess_(wtSess) {
  }
  ~WtWriteLooper() override = default;

 private:
  // invoked when we need to egress an event on the underlying CONNECT stream
  void onEvent(detail::WtStreamManager::Event&&) noexcept override;
  void runLoopCallback() noexcept override;

  WebTransportTxnHandler& wtTxnHandler_;
  WtHttpSession& wtSess_;
  folly::IOBufQueue buf_{folly::IOBufQueue::cacheChainLength()};
};

void WtWriteLooper::onEvent(detail::WtStreamManager::Event&& event) noexcept {
  H3ConnectStreamCallback::onEvent(std::move(event));
  schedule();
}

void WtWriteLooper::runLoopCallback() noexcept {
  // serialize all control frames
  auto* txn = wtTxnHandler_.txn_;
  const bool canWrite = !wtSess_.writesDone_ && txn;
  VLOG(4) << "WtWriteLooper canWrite=" << canWrite;
  if (!canWrite) {
    return;
  }
  txn->sendBody(buf_.move());
  // if we've visited CloseSession, send eom & mark writes done
  if (visitor.sessionClosed) {
    VLOG(4) << "WtWriteLooper sessionClosed";
    txn->sendEOM();
    wtSess_.writesDone();
  }
}

HqWtSession::HqWtSession(WtLooper& readLooper,
                         WtLooper& writeLooper,
                         std::shared_ptr<QuicSocket> quicSocket,
                         WebTransportHandler::Ptr wtHandler,
                         WtStreamManager::WtConfig wtConfig,
                         uint64_t connectStreamId,
                         H3ConnectStreamCallback& cb) noexcept
    : WtHttpSession(readLooper, writeLooper),
      wtSess_(std::move(quicSocket),
              std::move(wtHandler),
              wtConfig,
              connectStreamId,
              cb) {
}

void HqWtSession::onHttpError(const HTTPException& err) noexcept {
  VLOG(4) << __func__ << "; err=" << err.describe();
  wtSess_.onCloseSession({WebTransport::kInternalError, err.describe()});
  WtHttpSession::onHttpError(err);
}

void HqWtSession::onDone() noexcept {
  VLOG(4) << __func__;
  wtSess_.closeSession(folly::none);
}

void HqWtSession::init(Ptr self,
                       HttpWtClientCallbackPtr wtClientCallback) noexcept {
  this->self = std::shared_ptr<WebTransport>(self, &wtSess_);
  txnHandler_.wtClientCb_ = std::move(wtClientCallback);
  wtSess_.onWtSession(this->self);
}

/*static*/ auto HqWtSession::make(folly::EventBase* evb,
                                  detail::WtStreamManager::WtConfig wtConfig,
                                  WebTransportHandler::Ptr wtHandler,
                                  std::shared_ptr<quic::QuicSocket> quicSocket,
                                  uint64_t connectStreamId) -> Ptr {
  struct Agg {
    Agg(folly::EventBase* evb,
        std::shared_ptr<QuicSocket> quicSocket,
        WebTransportHandler::Ptr wtHandler,
        WtStreamManager::WtConfig wtConfig,
        uint64_t connectStreamId) noexcept
        : hqWtSession(readLooper,
                      writeLooper,
                      std::move(quicSocket),
                      std::move(wtHandler),
                      wtConfig,
                      connectStreamId,
                      writeLooper),
          readLooper(evb, hqWtSession.txnHandler_, hqWtSession),
          writeLooper(evb, hqWtSession.txnHandler_, hqWtSession) {
    }
    ~Agg() noexcept = default;
    HqWtSession hqWtSession;
    WtReadLooper readLooper;
    WtWriteLooper writeLooper;
  };

  auto agg = std::make_shared<Agg>(evb,
                                   std::move(quicSocket),
                                   std::move(wtHandler),
                                   wtConfig,
                                   connectStreamId);
  // alias
  return std::shared_ptr<HqWtSession>(agg, &agg->hqWtSession);
}

}; // namespace proxygen::detail
