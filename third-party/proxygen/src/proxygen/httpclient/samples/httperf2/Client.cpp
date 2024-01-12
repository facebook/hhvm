/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpclient/samples/httperf2/Client.h>

#include <boost/cast.hpp>
#include <fizz/protocol/CertUtils.h>
#include <folly/FileUtil.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/portability/OpenSSL.h>
#include <proxygen/httpserver/samples/hq/HQLoggerHelper.h>
#include <proxygen/httpserver/samples/hq/InsecureVerifierDangerousDoNotUseInProduction.h>
#include <quic/QuicConstants.h>

using namespace folly;
using namespace proxygen;
using namespace std;

DEFINE_int32(max_outstanding_transactions,
             6,
             "Maximum allowed in-flight transactions per HTTP session");
DEFINE_int32(req_per_loop,
             100,
             "Maximum number of requests to send per event loop");
DEFINE_int32(connect_timeout, 120, "Connect timeout in ms");
DEFINE_int32(conn_flow_control,
             proxygen::http2::kInitialWindow,
             "Connection flow control");
DEFINE_int32(stream_flow_control,
             proxygen::http2::kInitialWindow,
             "Stream flow control");
DEFINE_string(congestion,
              "cubic",
              "Congestion control algorithm, cubic/newreno/bbr/none");
DEFINE_int32(max_receive_packet_size,
             quic::kDefaultUDPReadBufferSize,
             "Max UDP packet size Quic can receive");
DEFINE_bool(
    can_ignore_pmtu,
    false,
    "We can ignore PTMU for the transport and use advertised recv buffer");
DEFINE_bool(udp_connect, true, "Whether or not to connect the udp socket");
DEFINE_bool(pacing, false, "Whether to enable pacing in Quic");
DEFINE_uint32(quic_batching_mode,
              static_cast<uint32_t>(quic::QuicBatchingMode::BATCHING_MODE_GSO),
              "QUIC batching mode");
DEFINE_uint32(quic_batch_size,
              quic::kDefaultQuicMaxBatchSize,
              "Maximum number of packets that can be batchedi in Quic");
DEFINE_bool(early_data, false, "Enable Fizz early data");
DEFINE_uint32(quic_recv_batch_size,
              16,
              "Number of packets to receiver per loop.");

static std::atomic_bool exitAllClientsSoon{false};

void Client::exitAllSoon() {
  exitAllClientsSoon = true;
}

Client::Client(EventBase* eventBase,
               const WheelTimerInstance& transactionTimeouts,
               HTTPerfStats& stats,
               Optional<SocketAddress> bindAddr,
               const SocketAddress& address,
               HTTPMessage& request,
               const std::string& requestData,
               uint32_t n_requests,
               FinishedCallback* callback,
               const string& plaintextProto,
               const string& serverName)
    : eventBase_(eventBase),
      stats_(stats),
      requestsSent_(0),
      bindAddr_(std::move(bindAddr)),
      address_(address),
      sslSession_(nullptr),
      request_(request),
      requestData_(requestData),
      requests_(n_requests),
      callback_(callback),
      connector_(this, transactionTimeouts),
      serverName_(serverName),
      plaintextProto_(plaintextProto) {
  CHECK_GT(requests_, 0);
  connector_.setPlaintextProtocol(plaintextProto);
}

Client::~Client() {
  if (isLoopCallbackScheduled()) {
    cancelLoopCallback();
  }
  inDestructor_ = true;
  collector_.stopCallbacks();
  if (session_) {
    // drain() might cause synchronous destruction, protect ourselves
    // from it.
    DelayedDestructionBase::DestructorGuard guard(session_);
    VLOG(4) << "shutting down session";
    session_->dropConnection("shutting down");
    // The above will destroy session_ and kill all the transactions
    // But we don't need any callbacks because we're toast
    session_->setInfoCallback(nullptr);
    session_ = nullptr;
  }
}

void Client::start() {
  connect();
}

void Client::exit() {
  if (callback_ != nullptr) {
    FinishedCallback* callback = callback_;
    callback_ = nullptr;
    callback->clientFinished(this);
  }
}

void Client::setSSLParameters(const folly::SSLContextPtr& sslContext,
                              std::shared_ptr<folly::ssl::SSLSession> session) {
  sslContext_ = sslContext;
  sslSession_ = session;
  if (sslSession_ != nullptr) {
    shouldReuseSession_ = true;
  }
}

void Client::setupFizzContext(std::shared_ptr<fizz::client::PskCache> pskCache,
                              bool pskKe,
                              const std::string& cert,
                              const std::string& key) {
  fizzContext_ = std::make_shared<fizz::client::FizzClientContext>();
  fizzContext_->setSupportedAlpns({plaintextProto_});
  if (FLAGS_early_data) {
    fizzContext_->setSendEarlyData(true);
  }

  if (pskKe) {
    fizzContext_->setSupportedPskModes({fizz::PskKeyExchangeMode::psk_ke});
  }

  fizzContext_->setPskCache(std::move(pskCache));
  if (!cert.empty()) {
    std::string certData;
    std::string keyData;
    folly::readFile(cert.c_str(), certData);
    if (!key.empty()) {
      folly::readFile(key.c_str(), keyData);
    }
    auto selfCert = fizz::CertUtils::makeSelfCert(certData, keyData);
    fizzContext_->setClientCertificate(std::move(selfCert));
  }
}

void Client::setQuicPskCache(std::shared_ptr<quic::QuicPskCache> quicPskCache) {
  quicPskCache_ = std::move(quicPskCache);
}

void Client::setUseQuic(bool useQuic) {
  useQuic_ = useQuic;
}

bool Client::supportsTickets() const {
  return (sslContext_ &&
          !(SSL_CTX_get_options(sslContext_->getSSLCtx()) & SSL_OP_NO_TICKET));
}

std::shared_ptr<folly::ssl::SSLSession> Client::extractSSLSession() {
  auto session = std::move(sslSession_);
  sslSession_ = nullptr;
  return session;
}

static quic::TransportSettings createTransportSettings() {
  quic::TransportSettings transportSettings;
  transportSettings.attemptEarlyData = FLAGS_early_data;
  transportSettings.pacingEnabled = FLAGS_pacing;
  if (folly::to<uint16_t>(FLAGS_max_receive_packet_size) <
      quic::kDefaultUDPSendPacketLen) {
    throw std::invalid_argument(
        folly::to<std::string>("max_receive_packet_size needs to be at least ",
                               quic::kDefaultUDPSendPacketLen));
  }
  transportSettings.maxRecvPacketSize = FLAGS_max_receive_packet_size;
  transportSettings.canIgnorePathMTU = FLAGS_can_ignore_pmtu;
  if (FLAGS_conn_flow_control != 0) {
    transportSettings.advertisedInitialConnectionFlowControlWindow =
        FLAGS_conn_flow_control;
  }
  // TODO FLAGS_stream_*
  if (FLAGS_stream_flow_control != 0) {
    transportSettings.advertisedInitialBidiLocalStreamFlowControlWindow =
        FLAGS_stream_flow_control;
    transportSettings.advertisedInitialBidiRemoteStreamFlowControlWindow =
        FLAGS_stream_flow_control;
    transportSettings.advertisedInitialUniStreamFlowControlWindow =
        FLAGS_stream_flow_control;
  }

  auto ccType = quic::congestionControlStrToType(FLAGS_congestion);
  if (!ccType) {
    throw std::invalid_argument(folly::to<std::string>(
        "invalid congestion control ", FLAGS_congestion));
  }
  transportSettings.defaultCongestionController = *ccType;
  transportSettings.batchingMode =
      quic::getQuicBatchingMode(FLAGS_quic_batching_mode);
  transportSettings.maxBatchSize = FLAGS_quic_batch_size;
  transportSettings.shouldRecvBatch = true;
  transportSettings.maxRecvBatchSize = FLAGS_quic_recv_batch_size;
  transportSettings.connectUDP = FLAGS_udp_connect;
  transportSettings.shouldDrain = false;
  return transportSettings;
}

///////// Connection Setup

void Client::connect() {
  connector_.reset();
  start_ = std::chrono::steady_clock::now();

  static const SocketOptionMap opts{{{SOL_SOCKET, SO_REUSEADDR}, 1}};

  if (sslContext_) {
    connector_.connectSSL(eventBase_,
                          address_,
                          sslContext_,
                          shouldReuseSession_ ? sslSession_ : nullptr,
                          std::chrono::milliseconds(FLAGS_connect_timeout),
                          opts);
  } else if (fizzContext_ && !useQuic_) {
    connector_.connectFizz(
        eventBase_,
        address_,
        fizzContext_,
        std::make_shared<
            proxygen::InsecureVerifierDangerousDoNotUseInProduction>(),
        std::chrono::milliseconds(FLAGS_connect_timeout),
        std::chrono::milliseconds(0),
        opts,
        folly::AsyncSocket::anyAddress(),
        serverName_);
  } else if (fizzContext_ && useQuic_) {
    hqConnector_ = std::make_unique<HQConnector>(
        this, std::chrono::milliseconds(FLAGS_connect_timeout));
    hqConnector_->setTransportSettings(createTransportSettings());
    hqConnector_->setQuicPskCache(quicPskCache_);
    hqConnector_->connect(
        eventBase_,
        bindAddr_,
        address_,
        fizzContext_,
        std::make_shared<
            proxygen::InsecureVerifierDangerousDoNotUseInProduction>(),
        std::chrono::milliseconds(FLAGS_connect_timeout),
        folly::emptySocketOptionMap,
        serverName_,
        qlogger_);
  } else {
    connector_.connect(eventBase_,
                       address_,
                       std::chrono::milliseconds(FLAGS_connect_timeout),
                       opts);
  }
}

///////// Request logic

bool Client::shouldExit() const {
  return exitAllClientsSoon ||
         (requestsSent_ >= requests_ && outstandingTransactions_ == 0);
}

void Client::runLoopCallback() noexcept {
  if (session_) {
    sendRequest();
  } else if (shouldExit()) {
    exit();
  } else {
    connect();
  }
}

void Client::sendRequest() {
  if (shouldExit()) {
    // TODO: should track responses + errors before calling finished
    exit();
    return;
  }
  if (isConnecting()) {
    // We are in the middle of the connect process. The request will
    // happen once the connect succeeds.
    return;
  }
  CHECK(session_);
  uint32_t requestsThisLoop = 0;
  while ((outstandingTransactions_ <
          uint32_t(FLAGS_max_outstanding_transactions)) &&
         (requestsSent_ < requests_) &&
         (requestsThisLoop++ < uint32_t(FLAGS_req_per_loop))) {
    auto handler = new TransactionHandler(this);
    auto txn = session_->newTransaction(handler);
    if (!txn) {
      // The session doesn't support any more transactions
      delete handler;
      break;
    }
    outstandingTransactions_++;
    requestsSent_++;
    stats_.addRequest();
    if (requestsSent_ == requests_ && useQuic_) {
      // We need to drain before we send the last request
      // in quic due to the way h1q works.
      session_->drain();
    }
    txn->sendHeaders(request_);
    if (!requestData_.empty()) {
      auto data =
          folly::IOBuf::wrapBuffer(requestData_.data(), requestData_.length());
      txn->sendBody(std::move(data));
    }
    txn->sendEOM();
  }
  if ((outstandingTransactions_ <
       uint32_t(FLAGS_max_outstanding_transactions)) &&
      (requestsSent_ < requests_)) {
    eventBase_->runInLoop(this);
  }
}

void Client::connectSuccess(proxygen::HQUpstreamSession* session) {
  CHECK(!session_);
  const auto transport = session->getQuicSocket();
  auto client =
      CHECK_NOTNULL(dynamic_cast<const quic::QuicClientTransport*>(transport));
  if (client->isTLSResumed()) {
    stats_.addResume();
  } else {
    stats_.addHandshake();
  }
  connectSuccessCommon(session);
}

void Client::connectError(const quic::QuicErrorCode&) {
  connectErrorCommon();
}

void Client::connectSuccess(HTTPUpstreamSession* session) {
  CHECK(!session_);
  session->setByteEventTracker(nullptr);
  if (sslContext_) {
    auto sslSocket = dynamic_cast<AsyncSSLSocket*>(session->getTransport());
    const unsigned char* nextProto = nullptr;
    unsigned nextProtoLength = 0;
    sslSocket->getSelectedNextProtocol(&nextProto, &nextProtoLength);
    if (nextProto) {
      VLOG(4) << "Client selected next protocol "
              << string((const char*)nextProto, nextProtoLength);
    } else {
      VLOG(4) << "Client did not select a next protocol";
    }
    if (sslSocket->getSSLSessionReused()) {
      stats_.addResume();
    } else {
      sslSession_ = sslSocket->getSSLSession();
      stats_.addHandshake();
    }
  }
  auto asyncSock =
      session->getTransport()->getUnderlyingTransport<AsyncSocket>();
  if (asyncSock) {
    if (asyncSock->getNetworkSocket().toFd() >= 0) {
      // Set linger timeout to 0 to avoid sockets in time wait state.
      struct linger optLinger = {1, 0};
      asyncSock->setSockOpt(SOL_SOCKET, SO_LINGER, &optLinger);
    }
  }
  connectSuccessCommon(session);
}

void Client::connectError(const AsyncSocketException& /*ex*/) {
  connectErrorCommon();
}

void Client::connectSuccessCommon(HTTPSessionBase* session) {
  CHECK(!session_);
  session_ = session;
  session_->setInfoCallback(&collector_);
  session->setFlowControl(FLAGS_stream_flow_control,
                          FLAGS_stream_flow_control,
                          FLAGS_conn_flow_control);
  // Don't artificially limit streams
  session->setMaxConcurrentOutgoingStreams(
      std::numeric_limits<uint32_t>::max());
  end_ = std::chrono::steady_clock::now();
  stats_.addConnection(millisecondsBetween(end_, start_).count());
  sendRequest();
}

void Client::connectErrorCommon() {
  stats_.addConnectError();
  requestsSent_++;
  if (shouldExit()) {
    exit();
  } else {
    eventBase_->runInLoop(this);
  }
}

// HTTPTransaction::Handler callbacks

void Client::TransactionHandler::detachTransaction() noexcept {
  DCHECK(!waitingForResponse_);
  DCHECK(!inMessage_);
  if (!parent_->inDestructor_) {
    DCHECK_GT(parent_->outstandingTransactions_, 0);
    parent_->outstandingTransactions_--;
    VLOG(3) << __func__ << " requestsSent=" << parent_->requestsSent_
            << " requests=" << parent_->requests_
            << " outstanding=" << parent_->outstandingTransactions_;
    if (!parent_->isLoopCallbackScheduled()) {
      parent_->Client::eventBase_->runInLoop(parent_);
    }
  }
  delete this;
}

void Client::TransactionHandler::onHeadersComplete(
    unique_ptr<HTTPMessage> msg) noexcept {
  parent_->stats_.addResponseCode(msg->getStatusCode());
  // TODO: the below updates happen rather late. We would ideally set this
  // in a onMessageBegin() callback, but HTTPTransaction::Handler has no
  // such callback
  waitingForResponse_ = false;
  inMessage_ = true;
  // TODO: should count (approximate) header bytes received also
}

void Client::TransactionHandler::onBody(
    unique_ptr<folly::IOBuf> chain) noexcept {
  parent_->stats_.addBytesReceived(chain->computeChainDataLength());
}

void Client::TransactionHandler::onTrailers(
    unique_ptr<HTTPHeaders> /*trailers*/) noexcept {
}

void Client::TransactionHandler::onEOM() noexcept {
  inMessage_ = false;
  parent_->stats_.addResponse(millisecondsSince(requestStart_).count());

  // TODO: not always true. Could have sent partial headers and then
  // error'd, but since we don't have a onMessageBegin(), we have to guess
  if (waitingForResponse_) {
    parent_->stats_.addEOFResponse();
  } else if (inMessage_) {
    parent_->stats_.addEOFError();
  }
}

void Client::TransactionHandler::onUpgrade(
    ::proxygen::UpgradeProtocol /*protocol*/) noexcept {
}

void Client::TransactionHandler::onError(
    const ::proxygen::HTTPException& error) noexcept {
  parent_->stats_.addErrorLat(millisecondsSince(requestStart_).count());
  if (error.getProxygenError() == kErrorTimeout) {
    parent_->stats_.addTimeoutError();
  }
  if (error.getDirection() == HTTPException::Direction::INGRESS) {
    waitingForResponse_ = false;
    if (inMessage_) {
      parent_->stats_.addMessageError();
    } else {
      // TODO: not always correct. Could have sent partial headers and then
      // error'd, but since we don't have a onMessageBegin(), we have to
      // guess
      parent_->stats_.addEOFResponse();
    }
  }
  inMessage_ = false;
}

// HTTPSession::InfoCallback callbacks
void Client::InfoCollector::stopCallbacks() {
  parent_ = nullptr;
}

void Client::InfoCollector::onDestroy(const HTTPSessionBase& /*sess*/) {
  if (parent_) {
    // In parent's session object destructor. Since our parent client
    // still exists, try to send another request.
    parent_->session_ = nullptr;
    parent_->Client::eventBase_->runInLoop(parent_);
  }
}

void Client::setQLoggerPath(const std::string& path) {
  // TODO: Where do i started, there are at leat 10 reasons the following line
  // is fucked up:
  qlogger_ = std::make_shared<quic::samples::HQLoggerHelper>(
      path, true /* pretty */, quic::VantagePoint::Client);
}
