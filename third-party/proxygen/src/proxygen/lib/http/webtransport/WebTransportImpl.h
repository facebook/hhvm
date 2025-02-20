/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <quic/api/QuicCallbacks.h>

namespace proxygen {

class WebTransportImpl : public WebTransport {
 public:
  class TransportProvider {
   public:
    virtual ~TransportProvider() = default;

    virtual folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
    newWebTransportBidiStream() = 0;

    virtual folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
    newWebTransportUniStream() = 0;

    virtual folly::SemiFuture<folly::Unit> awaitUniStreamCredit() = 0;

    virtual folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() = 0;

    virtual folly::Expected<FCState, WebTransport::ErrorCode>
    sendWebTransportStreamData(HTTPCodec::StreamID /*id*/,
                               std::unique_ptr<folly::IOBuf> /*data*/,
                               bool /*eof*/,
                               DeliveryCallback* /* deliveryCallback */) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
    notifyPendingWriteOnStream(HTTPCodec::StreamID,
                               quic::StreamWriteCallback* wcb) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
        resetWebTransportEgress(HTTPCodec::StreamID /*id*/,
                                uint32_t /*errorCode*/) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
        setWebTransportStreamPriority(HTTPCodec::StreamID /*id*/,
                                      HTTPPriority /*pri*/) = 0;

    virtual folly::Expected<std::pair<std::unique_ptr<folly::IOBuf>, bool>,
                            WebTransport::ErrorCode>
        readWebTransportData(HTTPCodec::StreamID /*id*/, size_t /*max*/) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
    initiateReadOnBidiStream(HTTPCodec::StreamID /*id*/,
                             quic::StreamReadCallback* /*readCallback*/) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
        pauseWebTransportIngress(HTTPCodec::StreamID /*id*/) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
        resumeWebTransportIngress(HTTPCodec::StreamID /*id*/) = 0;

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
        stopReadingWebTransportIngress(
            HTTPCodec::StreamID /*id*/,
            folly::Optional<uint32_t> /*errorCode*/) = 0;
    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode> sendDatagram(
        std::unique_ptr<folly::IOBuf> /*datagram*/) = 0;

    virtual bool usesEncodedApplicationErrorCodes() = 0;
  };

  class SessionProvider {
   public:
    virtual ~SessionProvider() = default;

    virtual void refreshTimeout() {
    }

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode> closeSession(
        folly::Optional<uint32_t> /*error*/) = 0;
  };

  WebTransportImpl(TransportProvider& tp, SessionProvider& sp)
      : tp_(tp), sp_(sp) {
  }

  ~WebTransportImpl() override = default;

  void destroy();

  // WT API
  folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
  createUniStream() override {
    return newWebTransportUniStream();
  }

  folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
  createBidiStream() override {
    return newWebTransportBidiStream();
  }
  folly::SemiFuture<folly::Unit> awaitUniStreamCredit() override {
    return tp_.awaitUniStreamCredit();
  }
  folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() override {
    return tp_.awaitBidiStreamCredit();
  }
  folly::Expected<folly::SemiFuture<StreamData>, WebTransport::ErrorCode>
  readStreamData(uint64_t id) override {
    auto it = wtIngressStreams_.find(id);
    if (it == wtIngressStreams_.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::INVALID_STREAM_ID);
    }
    return it->second.readStreamData();
  }
  folly::Expected<folly::Unit, WebTransport::ErrorCode> stopSending(
      uint64_t id, uint32_t error) override {
    auto it = wtIngressStreams_.find(id);
    if (it == wtIngressStreams_.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::INVALID_STREAM_ID);
    }
    return it->second.stopSending(error);
  }
  folly::Expected<FCState, ErrorCode> writeStreamData(
      uint64_t id,
      std::unique_ptr<folly::IOBuf> data,
      bool fin,
      DeliveryCallback* deliveryCallback) override {
    auto it = wtEgressStreams_.find(id);
    if (it == wtEgressStreams_.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::INVALID_STREAM_ID);
    }
    return it->second.writeStreamData(std::move(data), fin, deliveryCallback);
  }
  folly::Expected<folly::Unit, WebTransport::ErrorCode> resetStream(
      uint64_t id, uint32_t error) override {
    auto it = wtEgressStreams_.find(id);
    if (it == wtEgressStreams_.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::INVALID_STREAM_ID);
    }
    return it->second.resetStream(error);
  }
  folly::Expected<folly::Unit, WebTransport::ErrorCode> setPriority(
      uint64_t id, uint8_t level, uint64_t order, bool incremental) override {
    auto it = wtEgressStreams_.find(id);
    if (it == wtEgressStreams_.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::INVALID_STREAM_ID);
    }
    return it->second.setPriority(level, order, incremental);
  }
  folly::Expected<folly::SemiFuture<folly::Unit>, ErrorCode> awaitWritable(
      uint64_t streamId) override {
    auto it = wtEgressStreams_.find(streamId);
    if (it == wtEgressStreams_.end()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::INVALID_STREAM_ID);
    }
    return it->second.awaitWritable();
  }
  folly::Expected<folly::Unit, WebTransport::ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) override {
    // This can bypass the size and state machine checks in
    // HTTPTransaction::sendDatagram
    if (!tp_.sendDatagram(std::move(datagram))) {
      return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
    }
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> closeSession(
      folly::Optional<uint32_t> error) override {
    return sp_.closeSession(error);
  }

  class StreamWriteHandle
      : public WebTransport::StreamWriteHandle
      , public quic::StreamWriteCallback {
   public:
    StreamWriteHandle(WebTransportImpl& tp, HTTPCodec::StreamID id)
        : impl_(tp), id_(id) {
    }

    ~StreamWriteHandle() override {
      cancellationSource_.requestCancellation();
    }

    folly::CancellationToken getCancelToken() override {
      return cancellationSource_.getToken();
    }

    uint64_t getID() override {
      return id_;
    }

    folly::Expected<FCState, WebTransport::ErrorCode> writeStreamData(
        std::unique_ptr<folly::IOBuf> data,
        bool fin,
        DeliveryCallback* deliveryCallback) override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode> resetStream(
        uint32_t errorCode) override {
      return impl_.resetWebTransportEgress(id_, errorCode);
    }

    folly::Expected<folly::Unit, WebTransport::ErrorCode> setPriority(
        uint8_t level, uint64_t order, bool incremental) override {
      return impl_.tp_.setWebTransportStreamPriority(
          getID(), {level, incremental, order});
    }
    folly::Expected<folly::SemiFuture<folly::Unit>, ErrorCode> awaitWritable()
        override;

    void onStopSending(uint32_t errorCode);

    // TODO: what happens to promise_ if this stream is reset or the
    // conn closes

   private:
    void onStreamWriteReady(quic::StreamId id, uint64_t) noexcept override;

    WebTransportImpl& impl_;
    HTTPCodec::StreamID id_;
    folly::Optional<folly::Promise<folly::Unit>> writePromise_;
    folly::CancellationSource cancellationSource_;
  };

  class StreamReadHandle
      : public WebTransport::StreamReadHandle
      , public quic::StreamReadCallback {
   public:
    StreamReadHandle(WebTransportImpl& impl, HTTPCodec::StreamID id)
        : impl_(impl), id_(id) {
    }

    ~StreamReadHandle() override = default;

    uint64_t getID() override {
      return id_;
    }

    folly::CancellationToken getCancelToken() override {
      return cancellationSource_.getToken();
    }

    folly::SemiFuture<WebTransport::StreamData> readStreamData() override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode> stopSending(
        uint32_t error) override {
      return impl_.stopReadingWebTransportIngress(id_, error);
    }

    FCState dataAvailable(std::unique_ptr<folly::IOBuf> data, bool eof);
    void deliverReadError(const folly::exception_wrapper& ex);
    [[nodiscard]] bool open() const {
      return !eof_ && !error_;
    }

    // quic::StreamReadCallback overrides
    void readAvailable(quic::StreamId id) noexcept override;
    void readError(quic::StreamId id, quic::QuicError error) noexcept override;

   private:
    WebTransportImpl& impl_;
    HTTPCodec::StreamID id_;
    folly::Optional<folly::Promise<WebTransport::StreamData>> readPromise_;
    folly::IOBufQueue buf_{folly::IOBufQueue::cacheChainLength()};
    bool eof_{false};
    folly::Optional<folly::exception_wrapper> error_;
    folly::CancellationSource cancellationSource_;
  };

 private:
  folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
  sendWebTransportStreamData(HTTPCodec::StreamID id,
                             std::unique_ptr<folly::IOBuf> data,
                             bool eof,
                             DeliveryCallback* deliveryCallback);

  folly::Expected<folly::Unit, WebTransport::ErrorCode> resetWebTransportEgress(
      HTTPCodec::StreamID id, uint32_t errorCode);

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  stopReadingWebTransportIngress(HTTPCodec::StreamID id,
                                 folly::Optional<uint32_t> errorCode);

  folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
  newWebTransportBidiStream();

  folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
  newWebTransportUniStream();

  TransportProvider& tp_;
  SessionProvider& sp_;
  using WTEgressStreamMap = std::map<HTTPCodec::StreamID, StreamWriteHandle>;
  using WTIngressStreamMap = std::map<HTTPCodec::StreamID, StreamReadHandle>;
  WTEgressStreamMap wtEgressStreams_;
  WTIngressStreamMap wtIngressStreams_;

 public:
  // Calls from Transport
  struct BidiStreamHandle {
    StreamReadHandle* readHandle;
    StreamWriteHandle* writeHandle;
  };
  BidiStreamHandle onWebTransportBidiStream(HTTPCodec::StreamID id);

  WebTransportImpl::StreamReadHandle* onWebTransportUniStream(
      HTTPCodec::StreamID id);

  void onWebTransportStopSending(HTTPCodec::StreamID id, uint32_t errorCode);
};

} // namespace proxygen
