/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <folly/Conv.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <functional>
#include <mutex>
#include <proxygen/lib/http/HTTPException.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <random>
#include <vector>

#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/Random.h>
#include <folly/ThreadLocal.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/httpserver/samples/hq/HQServer.h>
#include <proxygen/httpserver/samples/hq/devious/DeviousBaton.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/utils/SafePathUtils.h>

namespace quic::samples {

/**
 * The Dispatcher object is responsible for spawning
 * new request handlers, based on the path.
 */
struct HandlerParams {
  std::string protocol;
  uint16_t port;
  std::string httpVersion;

  HandlerParams(std::string pro, uint16_t po, std::string h)
      : protocol(std::move(pro)), port(po), httpVersion(std::move(h)) {
  }
};

class Dispatcher {
 public:
  explicit Dispatcher(HandlerParams params) : params_(std::move(params)) {
  }

  proxygen::HTTPTransactionHandler* getRequestHandler(
      proxygen::HTTPMessage* /* msg */);

  HandlerParams params_;
};

using random_bytes_engine =
    std::independent_bits_engine<std::default_random_engine,
                                 CHAR_BIT,
                                 unsigned char>;

class BaseSampleHandler : public proxygen::HTTPTransactionHandler {
 public:
  BaseSampleHandler() = delete;

  explicit BaseSampleHandler(const HandlerParams& params) : params_(params) {
  }

  void setTransaction(proxygen::HTTPTransaction* txn) noexcept override {
    txn_ = txn;
  }

  void detachTransaction() noexcept override {
    delete this;
  }

  void onChunkHeader(size_t /*length*/) noexcept override {
  }

  void onChunkComplete() noexcept override {
  }

  void onTrailers(
      std::unique_ptr<proxygen::HTTPHeaders> /*trailers*/) noexcept override {
  }

  void onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept override {
  }

  void onEgressPaused() noexcept override {
  }

  void onEgressResumed() noexcept override {
  }

  void maybeAddAltSvcHeader(proxygen::HTTPMessage& msg) const {
    if (params_.protocol.empty() || params_.port == 0) {
      return;
    }
    msg.getHeaders().add(
        proxygen::HTTP_HEADER_ALT_SVC,
        fmt::format("{}=\":{}\"; ma=3600", params_.protocol, params_.port));
  }

  // clang-format off
  static const std::string& getH1QFooter() {
    static const std::string footer(
" __    __  .___________.___________..______      ___ ___       ___    ______\n"
"|  |  |  | |           |           ||   _  \\    /  // _ \\     / _ \\  |      \\\n"
"|  |__|  | `---|  |----`---|  |----`|  |_)  |  /  /| | | |   | (_) | `----)  |\n"
"|   __   |     |  |        |  |     |   ___/  /  / | | | |    \\__, |     /  /\n"
"|  |  |  |     |  |        |  |     |  |     /  /  | |_| |  __  / /     |__|\n"
"|__|  |__|     |__|        |__|     | _|    /__/    \\___/  (__)/_/       __\n"
"                                                                        (__)\n"
"\n"
"\n"
"____    __    ____  __    __       ___   .___________.\n"
"\\   \\  /  \\  /   / |  |  |  |     /   \\  |           |\n"
" \\   \\/    \\/   /  |  |__|  |    /  ^  \\ `---|  |----`\n"
"  \\            /   |   __   |   /  /_\\  \\    |  |\n"
"   \\    /\\    /    |  |  |  |  /  _____  \\   |  |\n"
"    \\__/  \\__/     |__|  |__| /__/     \\__\\  |__|\n"
"\n"
"____    ____  _______     ___      .______\n"
"\\   \\  /   / |   ____|   /   \\     |   _  \\\n"
" \\   \\/   /  |  |__     /  ^  \\    |  |_)  |\n"
"  \\_    _/   |   __|   /  /_\\  \\   |      /\n"
"    |  |     |  |____ /  _____  \\  |  |\\  \\----.\n"
"    |__|     |_______/__/     \\__\\ | _| `._____|\n"
"\n"
" __       _______.    __  .___________.______\n"
"|  |     /       |   |  | |           |      \\\n"
"|  |    |   (----`   |  | `---|  |----`----)  |\n"
"|  |     \\   \\       |  |     |  |        /  /\n"
"|  | .----)   |      |  |     |  |       |__|\n"
"|__| |_______/       |__|     |__|        __\n"
"                                         (__)\n"
    );
    // clang-format on
    return footer;
  }

  static uint32_t getQueryParamAsNumber(
      std::unique_ptr<proxygen::HTTPMessage>& msg,
      const std::string& name,
      uint32_t defValue) noexcept {
    return folly::tryTo<uint32_t>(msg->getQueryParam(name)).value_or(defValue);
  }

 protected:
  [[nodiscard]] const std::string& getHttpVersion() const {
    return params_.httpVersion;
  }

  proxygen::HTTPMessage createHttpResponse(uint16_t status,
                                           std::string_view message) {
    proxygen::HTTPMessage resp;
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(status);
    resp.setStatusMessage(message);
    return resp;
  }

  proxygen::HTTPTransaction* txn_{nullptr};
  const HandlerParams& params_;
};

/*
** A handler which returns chunked responses spread over time
** Generally used to simulat live video downloads where every frame
** is delivered at 1/30 s (or similar) cadance.
** Query parameters used:
**  - keyFrame - size in bytes of the first chunk in the response
**  - frame - size in bytes of all other chunks
**  - segment - total time of the response in milliseconds.
*/
class ChunkedHandler
    : public BaseSampleHandler
    , folly::DelayedDestruction {
 public:
  explicit ChunkedHandler(const HandlerParams& params, folly::EventBase* evb)
      : BaseSampleHandler(params), evb_(evb) {
  }

  ChunkedHandler() = delete;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    VLOG(10) << "ChunkedHandler::onHeadersComplete";
    proxygen::HTTPMessage resp;
    resp.setStatusCode(200);
    resp.setStatusMessage("Ok");
    resp.setIsChunked(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    firstFrameSize_ =
        std::min(getQueryParamAsNumber(msg, "keyFrame", 5000), kMaxFrameSize);
    otherFrameSize_ =
        std::min(getQueryParamAsNumber(msg, "frame", 500), kMaxFrameSize);
    auto segment = std::min(getQueryParamAsNumber(msg, "segment", 2000),
                            kMaxSegmentLength);
    totalChunkCount_ = segment / frameDelay_.count();
  }
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
  }

  void onEOM() noexcept override {
    VLOG(10) << "ChunkedHandler::onEOM";
    sleepFutureCallback();
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    txn_->sendAbort();
    failed_ = true;
  }

 private:
  ~ChunkedHandler() override = default;

  std::unique_ptr<folly::IOBuf> genRandBytes(uint32_t len) {
    auto buffer = folly::IOBuf::create(len);
    buffer->append(len);
    std::generate(
        buffer->writableData(), buffer->writableData() + len, std::ref(rbe_));
    return buffer;
  }

  void sendChunkRandomData(uint32_t chunkSize) {
    auto data = genRandBytes(chunkSize);
    txn_->sendChunkHeader(chunkSize);
    if (!failed_) {
      txn_->sendBody(std::move(data));
    }
    if (!failed_) {
      txn_->sendChunkTerminator();
    }
  }

  void sleepFutureCallback() {
    DestructorGuard destructorGuard(this);
    uint32_t chunkSize = chunk_ == 0 ? firstFrameSize_ : otherFrameSize_;
    if (failed_) {
      return;
    }

    chunk_++;
    if (chunk_ > totalChunkCount_) {
      txn_->sendEOM();
      return;
    }
    sendChunkRandomData(chunkSize);
    sleepFuture_ =
        folly::futures::sleep(frameDelay_).via(evb_).then([this](auto&&) {
          sleepFutureCallback();
        });
  }

  const uint32_t kMaxFrameSize{1000000};
  const uint32_t kMaxSegmentLength{60000};
  uint32_t firstFrameSize_{5000};
  uint32_t otherFrameSize_{500};
  std::chrono::milliseconds frameDelay_{33};
  uint32_t totalChunkCount_{60};
  uint32_t chunk_{0};
  folly::EventBase* evb_;
  folly::SemiFuture<folly::Unit> sleepFuture_;
  bool failed_{false};
  random_bytes_engine rbe_;
};

class EchoHandler : public BaseSampleHandler {
 public:
  explicit EchoHandler(const HandlerParams& params)
      : BaseSampleHandler(params) {
  }

  EchoHandler() = delete;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    VLOG(10) << "EchoHandler::onHeadersComplete";
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    sendFooter_ =
        (msg->getHTTPVersion() == proxygen::HTTPMessage::kHTTPVersion09);
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(200);
    resp.setStatusMessage("Ok");
    msg->getHeaders().forEach(
        [&](const std::string& header, const std::string& val) {
          resp.getHeaders().add(folly::to<std::string>("x-echo-", header), val);
        });
    resp.setWantsKeepalive(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
  }

  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
    VLOG(10) << "EchoHandler::onBody";
    txn_->sendBody(std::move(chain));
  }

  void onEOM() noexcept override {
    VLOG(10) << "EchoHandler::onEOM";
    if (sendFooter_) {
      auto& footer = getH1QFooter();
      txn_->sendBody(folly::IOBuf::copyBuffer(footer.data(), footer.length()));
    }
    txn_->sendEOM();
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    txn_->sendAbort();
  }

 private:
  bool sendFooter_{false};
};

class TransportCallbackBase
    : public proxygen::HTTPTransactionTransportCallback {
  void firstHeaderByteFlushed() noexcept override {
  }

  void firstByteFlushed() noexcept override {
  }

  void lastByteFlushed() noexcept override {
  }

  void trackedByteFlushed() noexcept override {
  }

  void lastByteAcked(
      std::chrono::milliseconds /* latency */) noexcept override {
  }

  void headerBytesGenerated(
      proxygen::HTTPHeaderSize& /* size */) noexcept override {
  }

  void headerBytesReceived(
      const proxygen::HTTPHeaderSize& /* size */) noexcept override {
  }

  void bodyBytesGenerated(size_t /* nbytes */) noexcept override {
  }

  void bodyBytesReceived(size_t /* size */) noexcept override {
  }
};

class ContinueHandler : public EchoHandler {
 public:
  explicit ContinueHandler(const HandlerParams& params) : EchoHandler(params) {
  }

  ContinueHandler() = delete;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    VLOG(10) << "ContinueHandler::onHeadersComplete";
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    if (msg->getHeaders().getSingleOrEmpty(proxygen::HTTP_HEADER_EXPECT) ==
        "100-continue") {
      resp.setStatusCode(100);
      resp.setStatusMessage("Continue");
      maybeAddAltSvcHeader(resp);
      txn_->sendHeaders(resp);
    }
    EchoHandler::onHeadersComplete(std::move(msg));
  }
};

class RandBytesGenHandler : public BaseSampleHandler {
 public:
  explicit RandBytesGenHandler(const HandlerParams& params)
      : BaseSampleHandler(params) {
  }

  RandBytesGenHandler() = delete;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    auto path = msg->getPathAsStringPiece();
    VLOG(10) << "RandBytesGenHandler::onHeadersComplete";
    VLOG(1) << "Request path: " << path;
    CHECK_GE(path.size(), 1);
    try {
      respBodyLen_ = folly::to<uint64_t>(path.subpiece(1));
    } catch (const folly::ConversionError&) {
      auto errorMsg = folly::to<std::string>(
          "Invalid URL: cannot extract requested response-length from url "
          "path: ",
          path);
      LOG(ERROR) << errorMsg;
      sendError(errorMsg);
      return;
    }
    if (respBodyLen_ > kMaxAllowedLength) {
      sendError(kErrorMsg);
      return;
    }

    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(200);
    resp.setStatusMessage("Ok");
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    if (msg->getMethod() == proxygen::HTTPMethod::GET) {
      sendBodyInChunks();
    }
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
    VLOG(10) << "RandBytesGenHandler::onBody";
    sendBodyInChunks();
  }

  void onEOM() noexcept override {
    VLOG(10) << "RandBytesGenHandler::onEOM";
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    VLOG(10) << "RandBytesGenHandler::onERROR";
    txn_->sendAbort();
  }

  void onEgressPaused() noexcept override {
    paused_ = true;
  }

  void onEgressResumed() noexcept override {
    paused_ = false;
    sendBodyInChunks();
  }

 private:
  void sendBodyInChunks() {
    if (error_) {
      LOG(ERROR) << "sendBodyInChunks no-op, error_=true";
      txn_->sendAbort();
      return;
    }
    uint64_t iter = respBodyLen_ / kMaxChunkSize;
    if (respBodyLen_ % kMaxChunkSize != 0) {
      ++iter;
    }
    VLOG(10) << "Sending response in " << iter << " chunks";
    for (uint64_t i = 0; i < iter && !paused_; i++) {
      uint64_t chunkSize = std::fmin(kMaxChunkSize, respBodyLen_);
      VLOG(10) << "Sending " << chunkSize << " bytes of data";
      txn_->sendBody(genRandBytes(chunkSize));
      respBodyLen_ -= chunkSize;
    }
    if (!paused_ && !eomSent_ && respBodyLen_ == 0) {
      VLOG(10) << "Sending response EOM";
      txn_->sendEOM();
      eomSent_ = true;
    }
  }

  std::unique_ptr<folly::IOBuf> randBytes(int len) {
    static folly::ThreadLocal<std::vector<uint8_t>> data;
    random_bytes_engine rbe;
    auto previousSize = data->size();
    if (previousSize < size_t(len)) {
      data->resize(len);
      std::generate(begin(*data) + previousSize, end(*data), std::ref(rbe));
    }
    return folly::IOBuf::wrapBuffer(folly::ByteRange(data->data(), len));
  }

  std::unique_ptr<folly::IOBuf> genRandBytes(int len) {
    int contentLength = (len / 2) + 1;
    auto randData = randBytes(contentLength);
    auto hex = folly::hexlify(randData->coalesce());
    hex.resize(len);
    return folly::IOBuf::copyBuffer(hex);
  }

  void sendError(const std::string& errorMsg) {
    proxygen::HTTPMessage resp;
    resp.setStatusCode(400);
    resp.setStatusMessage("Bad Request");
    resp.setWantsKeepalive(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    txn_->sendBody(folly::IOBuf::copyBuffer(errorMsg));
    txn_->sendEOM();
    error_ = true;
  }

  const uint64_t kMaxAllowedLength{1ULL * 1024 * 1024 * 1024}; // 1 GB
  const uint64_t kMaxChunkSize{100ULL * 1024};                 // 100 KB
  const std::string kErrorMsg = folly::to<std::string>(
      "More than 1GB of data requested. ", "Please request for smaller size.");
  uint64_t respBodyLen_;
  bool paused_{false};
  bool eomSent_{false};
  bool error_{false};
};

class DummyHandler : public BaseSampleHandler {
 public:
  explicit DummyHandler(const HandlerParams& params)
      : BaseSampleHandler(params) {
  }

  DummyHandler() = delete;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    VLOG(10) << "DummyHandler::onHeadersComplete";
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(200);
    resp.setStatusMessage("Ok");
    resp.setWantsKeepalive(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    if (msg->getMethod() == proxygen::HTTPMethod::GET) {
      txn_->sendBody(folly::IOBuf::copyBuffer(kDummyMessage));
    }
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
    VLOG(10) << "DummyHandler::onBody";
    txn_->sendBody(folly::IOBuf::copyBuffer(kDummyMessage));
  }

  void onEOM() noexcept override {
    VLOG(10) << "DummyHandler::onEOM";
    txn_->sendEOM();
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    txn_->sendAbort();
  }

 private:
  const std::string kDummyMessage =
      folly::to<std::string>("you reached mvfst.net, ",
                             "reach the /echo endpoint for an echo response ",
                             "query /<number> endpoints for a variable size "
                             "response with random bytes");
};

class DelayHandler
    : public BaseSampleHandler
    , private folly::AsyncTimeout {
 public:
  explicit DelayHandler(const HandlerParams& params, folly::EventBase* evb)
      : BaseSampleHandler(params), AsyncTimeout(evb) {
  }

  DelayHandler() = delete;

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    VLOG(10) << "DelayHandler::onHeadersComplete";
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(200);
    resp.setStatusMessage("Ok");
    resp.setWantsKeepalive(true);
    maybeAddAltSvcHeader(resp);
    VLOG(10) << "DelayHandler::onHeadersComplete calling sendHeaders";
    txn_->sendHeaders(resp);

    auto duration = getQueryParamAsNumber(msg, "duration", 0);
    responseBody_ = fmt::format(
        "Response Body for: {} {}", msg->getMethodString(), msg->getURL());
    scheduleTimeout(std::chrono::milliseconds(duration));
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
    VLOG(10) << "DelayHandler::onBody";
  }

  void onEOM() noexcept override {
    VLOG(10) << "DelayHandler::onEOM";
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    cancelTimeout();
  }

 private:
  void timeoutExpired() noexcept override {
    txn_->sendBody(folly::IOBuf::copyBuffer(responseBody_));
    txn_->sendEOM();
  }

  std::string responseBody_;
};

class HealthCheckHandler : public BaseSampleHandler {
 public:
  HealthCheckHandler(bool healthy, const HandlerParams& params)
      : BaseSampleHandler(params), healthy_(healthy) {
  }

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    VLOG(10) << "HealthCheckHandler::onHeadersComplete";
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    if (msg->getMethod() == proxygen::HTTPMethod::GET) {
      resp.setStatusCode(healthy_ ? 200 : 400);
      resp.setStatusMessage(healthy_ ? "Ok" : "Not Found");
    } else {
      resp.setStatusCode(405);
      resp.setStatusMessage("Method not allowed");
    }
    resp.setWantsKeepalive(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);

    txn_->sendBody(
        folly::IOBuf::copyBuffer(healthy_ ? "1-AM-ALIVE" : "1-AM-NOT-WELL"));
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
    VLOG(10) << "HealthCheckHandler::onBody";
    assert(false);
  }

  void onEOM() noexcept override {
    VLOG(10) << "HealthCheckHandler::onEOM";
    txn_->sendEOM();
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    txn_->sendAbort();
  }

 private:
  bool healthy_;
};

class WaitReleaseHandler : public BaseSampleHandler {
 public:
  WaitReleaseHandler(folly::EventBase* evb, const HandlerParams& params)
      : BaseSampleHandler(params), evb_(evb) {
  }

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;

  void sendErrorResponse(const std::string& body) {
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(400);
    resp.setStatusMessage("ERROR");
    resp.setWantsKeepalive(false);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    txn_->sendBody(folly::IOBuf::copyBuffer(body));
    txn_->sendEOM();
  }

  void sendOkResponse(const std::string& body, bool eom) {
    proxygen::HTTPMessage resp;
    VLOG(10) << "Setting http-version to " << getHttpVersion();
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(200);
    resp.setStatusMessage("OK");
    resp.setWantsKeepalive(true);
    resp.setIsChunked(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    txn_->sendBody(folly::IOBuf::copyBuffer(body));
    if (eom) {
      txn_->sendEOM();
    }
  }

  void release() {
    evb_->runImmediatelyOrRunInEventBaseThreadAndWait([this] {
      txn_->sendBody(folly::IOBuf::copyBuffer("released\n"));
      txn_->sendEOM();
    });
  }

  void maybeCleanup();

  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
    VLOG(10) << "WaitReleaseHandler::onBody - ignoring";
  }

  void onEOM() noexcept override {
    VLOG(10) << "WaitReleaseHandler::onEOM";
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    maybeCleanup();
    txn_->sendAbort();
  }

 private:
  static std::unordered_map<uint, WaitReleaseHandler*>& getWaitingHandlers();

  static std::mutex& getMutex();

  std::string path_;
  uint32_t id_{0};
  folly::EventBase* evb_;
};

namespace {
constexpr auto kPushFileName = "pusheen.txt";
};

class ServerPushHandler : public BaseSampleHandler {
  class ServerPushTxnHandler : public proxygen::HTTPPushTransactionHandler {
    void setTransaction(
        proxygen::HTTPTransaction* /* txn */) noexcept override {
    }

    void detachTransaction() noexcept override {
    }

    void onError(const proxygen::HTTPException& /* err */) noexcept override {
    }

    void onEgressPaused() noexcept override {
    }

    void onEgressResumed() noexcept override {
    }
  };

 public:
  explicit ServerPushHandler(const HandlerParams& params)
      : BaseSampleHandler(params) {
  }

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> /* msg */) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> /* chain */) noexcept override;

  void onEOM() noexcept override;

  void onError(const proxygen::HTTPException& /*error*/) noexcept override;

  void detachTransaction() noexcept override {
  }

 private:
  void sendPushPromise(proxygen::HTTPTransaction* /* pushTxn */,
                       const std::string& /* path */);

  void sendErrorResponse(const std::string& /* body */);

  void sendPushResponse(proxygen::HTTPTransaction* /* pushTxn */,
                        const std::string& /* url */,
                        const std::string& /* body */,
                        bool /* eom */);

  void sendOkResponse(const std::string& /* body */, bool /* eom */);

  std::string path_;
  ServerPushTxnHandler pushTxnHandler_;
};

class DeviousBatonHandler : public BaseSampleHandler {
 public:
  explicit DeviousBatonHandler(const HandlerParams& params,
                               folly::EventBase* evb)
      : BaseSampleHandler(params), evb_(evb) {
  }

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> /* msg */) noexcept override;

  void onWebTransportBidiStream(
      proxygen::HTTPCodec::StreamID id,
      proxygen::WebTransport::BidiStreamHandle stream) noexcept override;
  void onWebTransportUniStream(
      proxygen::HTTPCodec::StreamID id,
      proxygen::WebTransport::StreamReadHandle* readHandle) noexcept override;

  void onWebTransportSessionClose(
      folly::Optional<uint32_t> error) noexcept override;

  void onDatagram(std::unique_ptr<folly::IOBuf> datagram) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> /* chain */) noexcept override;

  void onEOM() noexcept override;

  void onError(const proxygen::HTTPException& /*error*/) noexcept override;

  void detachTransaction() noexcept override {
  }

  folly::Optional<devious::DeviousBaton> devious_;
  void readHandler(proxygen::WebTransport::StreamReadHandle* readHandle,
                   folly::Try<proxygen::WebTransport::StreamData> streamData);
  folly::EventBase* evb_{nullptr};
  std::map<uint64_t, devious::DeviousBaton::BatonMessageState> streams_;
};

class StaticFileHandler : public BaseSampleHandler {
 public:
  StaticFileHandler(const HandlerParams& params, std::string staticRoot)
      : BaseSampleHandler(params), staticRoot_(std::move(staticRoot)) {
  }

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    auto path = msg->getPathAsStringPiece();
    VLOG(10) << "StaticFileHandler::onHeadersComplete";
    VLOG(4) << "Request path: " << path;
    if (path.contains("..")) {
      sendError("Path cannot contain ..");
      return;
    }

    auto filepath = folly::to<std::string>(staticRoot_, "/", path);
    try {
      auto safepath = proxygen::SafePath::getPath(filepath, staticRoot_, true);
      file_ = std::make_unique<folly::File>(safepath);
    } catch (...) {
      auto errorMsg = folly::to<std::string>(
          "Invalid URL: cannot open requested file. "
          "path: '",
          path,
          "'");
      LOG(ERROR) << errorMsg << " file: '" << filepath << "'";
      sendError(errorMsg);
      return;
    }
    proxygen::HTTPMessage resp = createHttpResponse(200, "Ok");
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    // use a CPU executor since read(2) of a file can block
    folly::getUnsafeMutableGlobalCPUExecutor()->add(
        std::bind(&StaticFileHandler::readFile,
                  this,
                  folly::EventBaseManager::get()->getEventBase()));
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
  }

  void onEOM() noexcept override {
  }

  void onError(const proxygen::HTTPException& /*error*/) noexcept override {
    VLOG(10) << "StaticFileHandler::onError";
    txn_->sendAbort();
  }

  void onEgressPaused() noexcept override {
    VLOG(10) << "StaticFileHandler::onEgressPaused";
    paused_ = true;
  }

  void onEgressResumed() noexcept override {
    VLOG(10) << "StaticFileHandler::onEgressResumed";
    paused_ = false;
    folly::getUnsafeMutableGlobalCPUExecutor()->add(
        std::bind(&StaticFileHandler::readFile,
                  this,
                  folly::EventBaseManager::get()->getEventBase()));
  }

 private:
  void readFile(folly::EventBase* evb) {
    folly::IOBufQueue buf;
    while (file_ && !paused_) {
      // read 4k-ish chunks and foward each one to the client
      auto data = buf.preallocate(4096, 4096);
      auto rc = folly::readNoInt(file_->fd(), data.first, data.second);
      if (rc < 0) {
        // error
        VLOG(4) << "Read error=" << rc;
        file_.reset();
        evb->runInEventBaseThread([this] {
          LOG(ERROR) << "Error reading file";
          txn_->sendAbort();
        });
        break;
      } else if (rc == 0) {
        // done
        file_.reset();
        VLOG(4) << "Read EOF";
        evb->runInEventBaseThread([this] { txn_->sendEOM(); });
        break;
      } else {
        buf.postallocate(rc);
        evb->runInEventBaseThread([this, body = buf.move()]() mutable {
          txn_->sendBody(std::move(body));
        });
      }
    }
  }

  void sendError(const std::string& errorMsg) {
    proxygen::HTTPMessage resp = createHttpResponse(400, "Bad Request");
    resp.setWantsKeepalive(true);
    maybeAddAltSvcHeader(resp);
    txn_->sendHeaders(resp);
    txn_->sendBody(folly::IOBuf::copyBuffer(errorMsg));
    txn_->sendEOM();
  }

  std::unique_ptr<folly::File> file_;
  std::atomic<bool> paused_{false};
  std::string staticRoot_;
};

} // namespace quic::samples
