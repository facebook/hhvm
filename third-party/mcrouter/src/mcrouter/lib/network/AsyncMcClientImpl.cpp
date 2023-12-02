/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsyncMcClientImpl.h"

#include <netinet/tcp.h>

#include <memory>

#include <folly/SingletonThreadLocal.h>
#include <folly/container/EvictingCacheMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/debug/FifoManager.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/network/McFizzClient.h"
#include "mcrouter/lib/network/McSSLUtil.h"
#include "mcrouter/lib/network/SocketConnector.h"
#include "mcrouter/lib/network/SocketUtil.h"
#include "mcrouter/lib/network/ThreadLocalSSLContextProvider.h"

namespace facebook {
namespace memcache {

constexpr size_t kReadBufferSizeMin = 256;
constexpr size_t kReadBufferSizeMax = 4096;
constexpr size_t kStackIovecs = 128;
constexpr size_t kMaxBatchSize = 24576 /* 24KB */;

namespace {
class OnEventBaseDestructionCallback final
    : public folly::EventBase::OnDestructionCallback {
 public:
  explicit OnEventBaseDestructionCallback(AsyncMcClientImpl& client)
      : client_(client) {}
  ~OnEventBaseDestructionCallback() final = default;
  void onEventBaseDestruction() noexcept final {
    client_.closeNow();
  }

 private:
  AsyncMcClientImpl& client_;
};

struct GoAwayContext : public folly::AsyncTransportWrapper::WriteCallback {
  GoAwayAcknowledgement message;
  McSerializedRequest data;
  std::unique_ptr<GoAwayContext> selfPtr;

  explicit GoAwayContext(const CodecIdRange& supportedCodecs)
      : data(message, 0, mc_caret_protocol, supportedCodecs) {}

  void writeSuccess() noexcept final {
    auto self = std::move(selfPtr);
    self.reset();
  }
  void writeErr(size_t, const folly::AsyncSocketException&) noexcept final {
    auto self = std::move(selfPtr);
    self.reset();
  }
};

inline size_t calculateIovecsTotalSize(const struct iovec* iovecs, size_t num) {
  size_t size = 0;
  while (num) {
    size += iovecs->iov_len;
    ++iovecs;
    --num;
  }
  return size;
}

} // namespace

void AsyncMcClientImpl::WriterLoop::runLoopCallback() noexcept {
  // Delay this write until the end of current loop (e.g. after
  // runActiveFibers() callback). That way we achieve better batching without
  // affecting latency.
  if (!client_.flushList_ && !rescheduled_) {
    rescheduled_ = true;
    client_.eventBase_.runInLoop(this, /* thisIteration */ true);
    return;
  }
  rescheduled_ = false;
  client_.pushMessages();
}

AsyncMcClientImpl::AsyncMcClientImpl(
    folly::VirtualEventBase& eventBase,
    ConnectionOptions options)
    : eventBase_(eventBase.getEventBase()),
      numConnectTimeoutRetriesLeft_(options.numConnectTimeoutRetries),
      queue_(options.accessPoint->getProtocol() != mc_ascii_protocol),
      outOfOrder_(options.accessPoint->getProtocol() != mc_ascii_protocol),
      writer_(*this),
      connectionOptions_(std::move(options)),
      eventBaseDestructionCallback_(
          std::make_unique<OnEventBaseDestructionCallback>(*this)) {
  eventBase.runOnDestruction(*eventBaseDestructionCallback_);
  if (connectionOptions_.compressionCodecMap) {
    supportedCompressionCodecs_ =
        connectionOptions_.compressionCodecMap->getIdRange();
  }
}

std::shared_ptr<AsyncMcClientImpl> AsyncMcClientImpl::create(
    folly::VirtualEventBase& eventBase,
    ConnectionOptions options) {
  auto client = std::shared_ptr<AsyncMcClientImpl>(
      new AsyncMcClientImpl(eventBase, std::move(options)), Destructor());
  client->selfPtr_ = client;
  return client;
}

void AsyncMcClientImpl::closeNow() {
  DestructorGuard dg(this);
  if (socket_) {
    isAborting_ = true;
    // We need to destroy it immediately.
    socket_->closeNow();
    socket_.reset();
    isAborting_ = false;
  } else if (connectionState_ == ConnectionState::Connecting) {
    isAborting_ = true;
  }
}

void AsyncMcClientImpl::setConnectionStatusCallbacks(
    ConnectionStatusCallbacks callbacks) {
  DestructorGuard dg(this);

  connectionCallbacks_ = std::move(callbacks);

  if (connectionState_ == ConnectionState::Up && connectionCallbacks_.onUp) {
    connectionCallbacks_.onUp(*socket_, getNumConnectRetries());
  }
}

void AsyncMcClientImpl::setRequestStatusCallbacks(
    RequestStatusCallbacks callbacks) {
  DestructorGuard dg(this);

  requestStatusCallbacks_ = std::move(callbacks);
}

void AsyncMcClientImpl::setAuthorizationCallbacks(
    AuthorizationCallbacks callbacks) {
  authorizationCallbacks_ = std::move(callbacks);
}

AsyncMcClientImpl::~AsyncMcClientImpl() {
  assert(queue_.getPendingRequestCount() == 0);
  assert(queue_.getInflightRequestCount() == 0);
  if (socket_) {
    // Close the socket immediately. We need to process all callbacks, such as
    // readEOF and connectError, before we exit destructor.
    socket_->closeNow();
  }
  eventBaseDestructionCallback_->cancel();
}

typename Transport::RequestQueueStats AsyncMcClientImpl::getRequestQueueStats()
    const {
  return RequestQueueStats{
      queue_.getPendingRequestCount(), queue_.getInflightRequestCount()};
}

void AsyncMcClientImpl::setThrottle(size_t maxInflight, size_t maxPending) {
  maxInflight_ = maxInflight;
  maxPending_ = maxPending;
}

void AsyncMcClientImpl::sendCommon(McClientRequestContextBase& req) {
  switch (req.reqContext.serializationResult()) {
    case McSerializedRequest::Result::OK:
      incMsgId(nextMsgId_);

      queue_.markAsPending(req);
      scheduleNextWriterLoop();
      if (connectionState_ == ConnectionState::Down) {
        attemptConnection();
      }
      return;
    case McSerializedRequest::Result::BAD_KEY:
      req.replyError(carbon::Result::BAD_KEY, "The key provided is invalid");
      return;
    case McSerializedRequest::Result::ERROR:
      req.replyError(
          carbon::Result::LOCAL_ERROR, "Error when serializing the request.");
      return;
  }
}

size_t AsyncMcClientImpl::getNumToSend() const {
  size_t numToSend = queue_.getPendingRequestCount();
  if (maxInflight_ != 0) {
    if (maxInflight_ <= queue_.getInflightRequestCount()) {
      numToSend = 0;
    } else {
      numToSend =
          std::min(numToSend, maxInflight_ - queue_.getInflightRequestCount());
    }
  }
  return numToSend;
}

void AsyncMcClientImpl::scheduleNextWriterLoop() {
  if (connectionState_ == ConnectionState::Up &&
      !writer_.isLoopCallbackScheduled() &&
      (getNumToSend() > 0 || pendingGoAwayReply_)) {
    if (flushList_) {
      flushList_->push_back(writer_);
    } else {
      eventBase_.runInLoop(&writer_);
    }
  }
}

void AsyncMcClientImpl::cancelWriterCallback() {
  writer_.cancelLoopCallback();
}

void AsyncMcClientImpl::pushMessages() {
  DestructorGuard dg(this);

  assert(connectionState_ == ConnectionState::Up);
  auto numToSend = getNumToSend();
  // Call batch status callback
  if (requestStatusCallbacks_.onWrite && numToSend > 0) {
    requestStatusCallbacks_.onWrite(numToSend);
  }

  std::array<struct iovec, kStackIovecs> iovecs;
  size_t iovsUsed = 0;
  size_t batchSize = 0;
  McClientRequestContextBase* tail = nullptr;

  auto sendBatchFun = [this](
                          McClientRequestContextBase* tailReq,
                          const struct iovec* iov,
                          size_t iovCnt,
                          bool last) {
    tailReq->isBatchTail = true;
    socket_->writev(
        this,
        iov,
        iovCnt,
        last ? folly::WriteFlags::NONE : folly::WriteFlags::CORK);
    return connectionState_ == ConnectionState::Up;
  };

  while (queue_.getPendingRequestCount() != 0 && numToSend > 0 &&
         /* we might be already not UP, because of failed writev */
         connectionState_ == ConnectionState::Up) {
    auto& req = queue_.peekNextPending();

    auto iov = req.reqContext.getIovs();
    auto iovcnt = req.reqContext.getIovsCount();
    if (debugFifo_.isConnected()) {
      debugFifo_.startMessage(MessageDirection::Sent, req.reqContext.typeId());
      debugFifo_.writeData(iov, iovcnt);
    }

    if (iovsUsed + iovcnt > kStackIovecs && iovsUsed) {
      // We're out of inline iovecs, flush what we batched.
      if (!sendBatchFun(tail, iovecs.data(), iovsUsed, false)) {
        break;
      }
      iovsUsed = 0;
      batchSize = 0;
    }

    if (iovcnt >= kStackIovecs || (iovsUsed == 0 && numToSend == 1)) {
      // Req is either too big to batch or it's the last one, so just send it
      // alone.
      queue_.markNextAsSending();
      sendBatchFun(&req, iov, iovcnt, numToSend == 1);
    } else {
      auto size = calculateIovecsTotalSize(iov, iovcnt);

      if (size + batchSize > kMaxBatchSize && iovsUsed) {
        // We already accumulated too much data, flush what we have.
        if (!sendBatchFun(tail, iovecs.data(), iovsUsed, false)) {
          break;
        }
        iovsUsed = 0;
        batchSize = 0;
      }

      queue_.markNextAsSending();
      if (size >= kMaxBatchSize || (iovsUsed == 0 && numToSend == 1)) {
        // Req is either too big to batch or it's the last one, so just send it
        // alone.
        sendBatchFun(&req, iov, iovcnt, numToSend == 1);
      } else {
        memcpy(iovecs.data() + iovsUsed, iov, sizeof(struct iovec) * iovcnt);
        iovsUsed += iovcnt;
        batchSize += size;
        tail = &req;

        if (numToSend == 1) {
          // This was the last request flush everything.
          sendBatchFun(tail, iovecs.data(), iovsUsed, true);
        }
      }
    }

    --numToSend;
  }
  if (connectionState_ == ConnectionState::Up && pendingGoAwayReply_) {
    // Note: we're not waiting for all requests to be sent, since that may take
    // a while and if we didn't succeed in one loop, this means that we're
    // already backlogged.
    sendGoAwayReply();
  }
  pendingGoAwayReply_ = false;
  scheduleNextWriterLoop();
}

void AsyncMcClientImpl::sendGoAwayReply() {
  auto ctxPtr = std::make_unique<GoAwayContext>(supportedCompressionCodecs_);
  auto& ctx = *ctxPtr;
  switch (ctx.data.serializationResult()) {
    case McSerializedRequest::Result::OK: {
      auto iov = ctx.data.getIovs();
      auto iovcnt = ctx.data.getIovsCount();
      // Pass context ownership of itself, writev will call a callback that
      // will destroy the context.
      ctx.selfPtr = std::move(ctxPtr);
      socket_->writev(&ctx, iov, iovcnt);
      break;
    }
    default:
      // Ignore errors on GoAway.
      break;
  }
}

void AsyncMcClientImpl::attemptConnection() {
  // We may use a lot of stack memory (e.g. hostname resolution) or some
  // expensive SSL code. This should be always executed on main context.
  folly::fibers::runInMainContext([this] {
    assert(connectionState_ == ConnectionState::Down);
    connectionState_ = ConnectionState::Connecting;
    pendingGoAwayReply_ = false;

    auto expectedSocket = createSocket(eventBase_, connectionOptions_);
    if (expectedSocket.hasError()) {
      connectErr(expectedSocket.error());
      return;
    }
    socket_ = std::move(expectedSocket).value();

    auto expectedSocketAddress = getSocketAddress(connectionOptions_);
    if (expectedSocketAddress.hasError()) {
      const auto& ex = expectedSocketAddress.error();
      LOG_FAILURE(
          "AsyncMcClient", failure::Category::kBadEnvironment, "{}", ex.what());
      connectErr(ex);
      return;
    }
    folly::SocketAddress address = std::move(expectedSocketAddress).value();

    socket_->setSendTimeout(connectionOptions_.writeTimeout.count());

    const auto mech = connectionOptions_.accessPoint->getSecurityMech();
    auto socketOptions = createSocketOptions(address, connectionOptions_);
    if ((isAsyncSSLSocketMech(mech)) &&
        connectionOptions_.securityOpts.sslHandshakeOffload) {
      // we keep ourself alive during connection.
      auto self = selfPtr_.lock();
      auto sslSocket = socket_->getUnderlyingTransport<folly::AsyncSSLSocket>();
      socket_.release();
      folly::AsyncSSLSocket::UniquePtr sslSockPtr(sslSocket);
      // offload the handshake
      connectSSLSocketWithAuxIO(
          std::move(sslSockPtr),
          std::move(address),
          connectionOptions_.connectTimeout.count(),
          std::move(socketOptions))
          .thenValue([self](folly::AsyncSocket::UniquePtr socket) {
            CHECK(self->eventBase_.isInEventBaseThread());
            if (self->isAborting_) {
              // closeNow was called before we connected, so we need to fail
              folly::AsyncSocketException ex(
                  folly::AsyncSocketException::INVALID_STATE,
                  "Client closed before connect completed");
              self->connectErr(ex);
              self->isAborting_ = false;
            } else {
              self->socket_ = std::move(socket);
              self->connectSuccess();
            }
          })
          .thenError(
              folly::tag_t<folly::AsyncSocketException>{},
              [self](const folly::AsyncSocketException& ex) {
                CHECK(self->eventBase_.isInEventBaseThread());
                self->connectErr(ex);
                // handle the case where the client was aborting mid connect
                if (self->isAborting_) {
                  self->isAborting_ = false;
                }
              });
    } else {
      // connect inline on the current evb
      if (mech == SecurityMech::TLS13_FIZZ) {
        auto fizzClient = socket_->getUnderlyingTransport<McFizzClient>();
        fizzClient->connect(
            this,
            address,
            connectionOptions_.connectTimeout.count(),
            socketOptions);
      } else {
        auto asyncSock = socket_->getUnderlyingTransport<folly::AsyncSocket>();
        asyncSock->connect(
            this,
            address,
            connectionOptions_.connectTimeout.count(),
            socketOptions);
      }
    }
  });
}

void AsyncMcClientImpl::connectSuccess() noexcept {
  assert(connectionState_ == ConnectionState::Connecting);
  DestructorGuard dg(this);
  connectionState_ = ConnectionState::Up;

  const auto mech = connectionOptions_.accessPoint->getSecurityMech();
  if (isAsyncSSLSocketMech(mech)) {
    auto* sslSocket = socket_->getUnderlyingTransport<folly::AsyncSSLSocket>();
    assert(sslSocket != nullptr);
    if (mech == SecurityMech::TLS_TO_PLAINTEXT) {
      // if we negotiated the right alpn, then server also supports this and we
      // can fall back to plaintext.
      auto fallback = McSSLUtil::moveToPlaintext(*sslSocket);
      if (!fallback) {
        // configured to use this but failed to negotiate.  will end up doing
        // ssl anyway
        auto errorMessage = folly::to<std::string>(
            "Failed to negotiate TLS to plaintext.  SSL will be used for ",
            connectionOptions_.accessPoint->toHostPortString());
        LOG_FAILURE(
            "AsyncMcClient", failure::Category::kBadEnvironment, errorMessage);
      } else {
        // replace socket with this
        socket_.reset(fallback.release());
        // we need to set timeout options here.  things like tcp opts and
        // qos are sock opts and don't change on the FD.
        socket_->setSendTimeout(connectionOptions_.writeTimeout.count());
      }
    } else if (mech == SecurityMech::KTLS12) {
      auto ktlsSock = McSSLUtil::moveToKtls(*sslSocket);
      // it's not an error if this fails - it could be a kernel that doesn't
      // support this or something else - on error we'll just continue using
      // user space ssl
      if (ktlsSock) {
        socket_.reset(ktlsSock.release());
        // we need to set timeout options here.  things like tcp opts and
        // qos are sock opts and don't change on the FD.
        socket_->setSendTimeout(connectionOptions_.writeTimeout.count());
      }
    }
  }

  // Now authorize the connection
  if (isAsyncSSLSocketMech(mech) && authorizationCallbacks_.onAuthorize &&
      !authorizationCallbacks_.onAuthorize(*socket_, connectionOptions_)) {
    if (connectionOptions_.securityOpts.sslAuthorizationEnforce) {
      // Enforcement is enabled, fail all requests and close connection.
      isAborting_ = true;
      processShutdown("Authorization failure");
      isAborting_ = false;
      return;
    }
  }

  assert(queue_.getInflightRequestCount() == 0);
  assert(queue_.getParserInitializer() == nullptr);

  if (connectionOptions_.enableQoS) {
    // guard this since getPeerAddress could throw
    try {
      folly::SocketAddress address;
      socket_->getPeerAddress(&address);
      auto asyncSock = socket_->getUnderlyingTransport<folly::AsyncSocket>();
      if (asyncSock) {
        checkWhetherQoSIsApplied(
            asyncSock->getNetworkSocket().toFd(),
            address,
            connectionOptions_,
            "AsyncMcClient");
      }
    } catch (const std::exception& ex) {
      LOG_FAILURE(
          "AsyncMcClient",
          failure::Category::kSystemError,
          "Failed to enableQOS: {}",
          folly::exceptionStr(ex));
    }
  }

  if (!connectionOptions_.debugFifoPath.empty()) {
    if (auto fifoManager = FifoManager::getInstance()) {
      if (auto fifo =
              fifoManager->fetchThreadLocal(connectionOptions_.debugFifoPath)) {
        debugFifo_ = ConnectionFifo(
            std::move(fifo), socket_.get(), connectionOptions_.routerInfoName);
      }
    }
  }

  if (connectionCallbacks_.onUp) {
    connectionCallbacks_.onUp(*socket_, getNumConnectRetries());
  }

  numConnectTimeoutRetriesLeft_ = connectionOptions_.numConnectTimeoutRetries;

  scheduleNextWriterLoop();
  parser_ = std::make_unique<ParserT>(
      *this,
      kReadBufferSizeMin,
      kReadBufferSizeMax,
      connectionOptions_.useJemallocNodumpAllocator,
      connectionOptions_.compressionCodecMap,
      &debugFifo_);
  socket_->setReadCB(this);
}

void AsyncMcClientImpl::connectErr(
    const folly::AsyncSocketException& ex) noexcept {
  assert(connectionState_ == ConnectionState::Connecting);
  DestructorGuard dg(this);

  std::string errorMessage;
  if (ex.getType() == folly::AsyncSocketException::SSL_ERROR) {
    errorMessage = folly::sformat(
        "SSLError: {}. Connect to {} failed.",
        ex.what(),
        connectionOptions_.accessPoint->toHostPortString());
    LOG_FAILURE(
        "AsyncMcClient", failure::Category::kBadEnvironment, errorMessage);
  }

  carbon::Result error = carbon::Result::CONNECT_ERROR;
  ConnectionDownReason reason = ConnectionDownReason::CONNECT_ERROR;
  if (ex.getType() == folly::AsyncSocketException::TIMED_OUT) {
    error = carbon::Result::CONNECT_TIMEOUT;
    reason = ConnectionDownReason::CONNECT_TIMEOUT;
    errorMessage = folly::to<std::string>(
        "Timed out when trying to connect to server. Ex: ", ex.what());
  } else if (isAborting_) {
    error = carbon::Result::ABORTED;
    reason = ConnectionDownReason::ABORTED;
    errorMessage =
        folly::to<std::string>("Connection aborted. Ex: ", ex.what());
  } else {
    errorMessage = ex.what();
  }

  assert(queue_.getInflightRequestCount() == 0);
  connectionState_ = ConnectionState::Down;
  // We don't need it anymore, so let it perform complete cleanup.
  socket_.reset();

  if (ex.getType() == folly::AsyncSocketException::TIMED_OUT &&
      numConnectTimeoutRetriesLeft_ > 0) {
    --numConnectTimeoutRetriesLeft_;
    attemptConnection();
  } else {
    queue_.failAllPending(error, errorMessage);
    if (connectionCallbacks_.onDown) {
      connectionCallbacks_.onDown(reason, getNumConnectRetries());
    }
    numConnectTimeoutRetriesLeft_ = connectionOptions_.numConnectTimeoutRetries;
  }
}

void AsyncMcClientImpl::processShutdown(folly::StringPiece errorMessage) {
  DestructorGuard dg(this);
  switch (connectionState_) {
    case ConnectionState::Up: // on error, UP always transitions to ERROR state
      // Cancel loop callback, or otherwise we might attempt to write
      // something while processing error state.
      cancelWriterCallback();
      connectionState_ = ConnectionState::Error;
      // We're already in ERROR state, no need to listen for reads.
      socket_->setReadCB(nullptr);
      // We can safely close connection, it will stop all writes.
      socket_->close();

      [[fallthrough]];

    case ConnectionState::Error:
      queue_.failAllSent(
          isAborting_ ? carbon::Result::ABORTED : carbon::Result::REMOTE_ERROR,
          errorMessage);
      if (queue_.getInflightRequestCount() == 0) {
        // No need to send any of remaining requests if we're aborting.
        if (isAborting_) {
          queue_.failAllPending(carbon::Result::ABORTED, errorMessage);
        }

        // This is a last processShutdown() for this error and it is safe
        // to go DOWN.
        if (connectionCallbacks_.onDown) {
          connectionCallbacks_.onDown(
              isAborting_ ? ConnectionDownReason::ABORTED
                          : ConnectionDownReason::ERROR,
              getNumConnectRetries());
        }

        connectionState_ = ConnectionState::Down;
        // We don't need it anymore, so let it perform complete cleanup.
        socket_.reset();

        // In case we still have some pending requests, then try reconnecting
        // immediately.
        if (queue_.getPendingRequestCount() != 0) {
          attemptConnection();
        }
      }
      return;
    case ConnectionState::Connecting:
    // connectError is not a remote error, it's processed in connectError.
    case ConnectionState::Down:
      // We shouldn't have any errors while not connected.
      CHECK(false);
  }
}

void AsyncMcClientImpl::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  curBuffer_ = parser_->getReadBuffer();
  *bufReturn = curBuffer_.first;
  *lenReturn = curBuffer_.second;
}

void AsyncMcClientImpl::readDataAvailable(size_t len) noexcept {
  assert(curBuffer_.first != nullptr && curBuffer_.second >= len);
  DestructorGuard dg(this);
  parser_->readDataAvailable(len);
}

void AsyncMcClientImpl::readEOF() noexcept {
  assert(connectionState_ == ConnectionState::Up);
  processShutdown("Connection closed by the server.");
}

void AsyncMcClientImpl::readErr(
    const folly::AsyncSocketException& ex) noexcept {
  assert(connectionState_ == ConnectionState::Up);
  std::string errorMessage = folly::sformat(
      "Failed to read from socket with remote endpoint \"{}\". Exception: {}",
      connectionOptions_.accessPoint->toString(),
      ex.what());
  VLOG(1) << errorMessage;
  processShutdown(errorMessage);
}

void AsyncMcClientImpl::writeSuccess() noexcept {
  assert(
      connectionState_ == ConnectionState::Up ||
      connectionState_ == ConnectionState::Error);
  DestructorGuard dg(this);

  bool last;
  do {
    auto& req = queue_.markNextAsSent();
    last = req.isBatchTail;
    req.scheduleTimeout();
  } while (!last);

  // It is possible that we're already processing error, but still have a
  // successfull write.
  if (connectionState_ == ConnectionState::Error) {
    processShutdown("Connection was in ERROR state.");
  }
}

void AsyncMcClientImpl::writeErr(
    size_t bytesWritten,
    const folly::AsyncSocketException& ex) noexcept {
  assert(
      connectionState_ == ConnectionState::Up ||
      connectionState_ == ConnectionState::Error);

  std::string errorMessage = folly::sformat(
      "Failed to write into socket with remote endpoint \"{}\", "
      "wrote {} bytes. Exception: {}",
      connectionOptions_.accessPoint->toString(),
      bytesWritten,
      ex.what());
  VLOG(1) << errorMessage;

  // We're already in an error state, so all requests in pendingReplyQueue_ will
  // be replied with an error.

  bool last;
  do {
    auto& req = queue_.markNextAsSent();
    last = req.isBatchTail;
  } while (!last);

  processShutdown(errorMessage);
}

void AsyncMcClientImpl::onEgressBuffered() {
  if (requestStatusCallbacks_.onPartialWrite) {
    requestStatusCallbacks_.onPartialWrite();
  }
}

void AsyncMcClientImpl::onEgressBufferCleared() {}

folly::StringPiece AsyncMcClientImpl::clientStateToStr() const {
  switch (connectionState_) {
    case ConnectionState::Up:
      return "UP";
    case ConnectionState::Down:
      return "DOWN";
    case ConnectionState::Connecting:
      return "CONNECTING";
    case ConnectionState::Error:
      return "ERROR";
  }
  return "state is incorrect";
}

void AsyncMcClientImpl::logErrorWithContext(folly::StringPiece reason) {
  LOG_FAILURE(
      "AsyncMcClient",
      failure::Category::kOther,
      "Error: \"{}\", client state: {}, remote endpoint: {}, "
      "number of requests sent through this client: {}, "
      "McClientRequestContextQueue info: {}",
      reason,
      clientStateToStr(),
      connectionOptions_.accessPoint->toString(),
      nextMsgId_,
      queue_.debugInfo());
}

void AsyncMcClientImpl::handleConnectionControlMessage(
    const CaretMessageInfo& headerInfo) {
  DestructorGuard dg(this);
  // Handle go away request.
  switch (headerInfo.typeId) {
    case GoAwayRequest::typeId: {
      // No need to process GoAway if the connection is already closing.
      if (connectionState_ != ConnectionState::Up) {
        break;
      }
      if (connectionCallbacks_.onDown) {
        connectionCallbacks_.onDown(
            ConnectionDownReason::SERVER_GONE_AWAY, getNumConnectRetries());
      }
      pendingGoAwayReply_ = true;
      scheduleNextWriterLoop();
      break;
    }
    default:
      // Ignore unknown control messages.
      break;
  }
}

void AsyncMcClientImpl::parseError(
    carbon::Result /* result */,
    folly::StringPiece reason) {
  logErrorWithContext(reason);
  // mc_parser can call the parseError multiple times, process only first.
  if (connectionState_ != ConnectionState::Up) {
    return;
  }
  DestructorGuard dg(this);
  processShutdown(reason);
}

bool AsyncMcClientImpl::nextReplyAvailable(uint64_t reqId) {
  assert(connectionState_ == ConnectionState::Up);

  auto initializer = queue_.getParserInitializer(reqId);

  if (initializer) {
    (*initializer)(*parser_);
    return true;
  }

  return false;
}

void AsyncMcClientImpl::incMsgId(uint32_t& msgId) {
  msgId += 2;
}

void AsyncMcClientImpl::updateTimeoutsIfShorter(
    std::chrono::milliseconds connectTimeout,
    std::chrono::milliseconds writeTimeout) {
  if (!connectTimeout.count() && !writeTimeout.count()) {
    return;
  }
  auto selfWeak = selfPtr_;
  eventBase_.runInEventBaseThread([selfWeak, connectTimeout, writeTimeout]() {
    if (auto self = selfWeak.lock()) {
      if (!self->connectionOptions_.connectTimeout.count() ||
          self->connectionOptions_.connectTimeout > connectTimeout) {
        self->connectionOptions_.connectTimeout = connectTimeout;
      }

      if (!self->connectionOptions_.writeTimeout.count() ||
          self->connectionOptions_.writeTimeout > writeTimeout) {
        self->connectionOptions_.writeTimeout = writeTimeout;
      }

      if (self->socket_) {
        self->socket_->setSendTimeout(
            self->connectionOptions_.writeTimeout.count());
      }
    }
  });
}

double AsyncMcClientImpl::getRetransmitsPerKb() {
  if (socket_ != nullptr) {
    struct tcp_info tcpinfo;
    socklen_t len = sizeof(struct tcp_info);

    if (socket_->getUnderlyingTransport<folly::AsyncSocket>()->getSockOpt(
            IPPROTO_TCP, TCP_INFO, &tcpinfo, &len) == 0) {
      const uint64_t totalKBytes = socket_->getRawBytesWritten() / 1024;
      if (totalKBytes == lastKBytes_) {
        return 0.0;
      }
      const auto retransPerKByte = (tcpinfo.tcpi_total_retrans - lastRetrans_) /
          (double)(totalKBytes - lastKBytes_);
      lastKBytes_ = totalKBytes;
      lastRetrans_ = tcpinfo.tcpi_total_retrans;
      return retransPerKByte;
    }
  }
  return -1.0;
}

int64_t AsyncMcClientImpl::getNumConnectRetries() noexcept {
  return connectionOptions_.numConnectTimeoutRetries -
      numConnectTimeoutRetriesLeft_;
}

} // namespace memcache
} // namespace facebook
