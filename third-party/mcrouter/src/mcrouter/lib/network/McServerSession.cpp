/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McServerSession.h"

#include <memory>

#include <folly/Executor.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/VirtualEventBase.h>
#include <folly/small_vector.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>

#include "mcrouter/lib/debug/FifoManager.h"
#include "mcrouter/lib/network/McFizzServer.h"
#include "mcrouter/lib/network/McSSLUtil.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/MultiOpParent.h"
#include "mcrouter/lib/network/WriteBuffer.h"

namespace facebook {
namespace memcache {

namespace {

ConnectionFifo getDebugFifo(
    const std::string& path,
    const folly::AsyncTransportWrapper* transport,
    const std::string& requestHandlerName) {
  if (!path.empty()) {
    if (auto fifoManager = FifoManager::getInstance()) {
      if (auto fifo = fifoManager->fetchThreadLocal(path)) {
        return ConnectionFifo(std::move(fifo), transport, requestHandlerName);
      }
    }
  }
  return ConnectionFifo();
}

folly::Optional<std::string> getCN(
    const folly::AsyncTransportCertificate* cert) {
  auto x509 = folly::OpenSSLTransportCertificate::tryExtractX509(cert);
  if (!x509) {
    return folly::none;
  }

  auto sub = X509_get_subject_name(x509.get());
  if (!sub) {
    return folly::none;
  }

  std::array<char, ub_common_name + 1> cn{};
  const auto res =
      X509_NAME_get_text_by_NID(sub, NID_commonName, cn.data(), ub_common_name);
  if (res <= 0) {
    return folly::none;
  }

  return std::string(cn.data(), res);
}
} // anonymous namespace

constexpr size_t kIovecVectorSize = 64;

McServerSession& McServerSession::create(
    folly::AsyncTransportWrapper::UniquePtr transport,
    std::shared_ptr<McServerOnRequest> cb,
    StateCallback& stateCb,
    const AsyncMcServerWorkerOptions& options,
    void* userCtxt,
    McServerSession::Queue* queue,
    const CompressionCodecMap* codecMap,
    KeepAlive keepAlive) {
  auto ptr = new McServerSession(
      std::move(transport),
      std::move(cb),
      stateCb,
      options,
      userCtxt,
      codecMap,
      keepAlive);

  assert(ptr->state_ == STREAMING);
  DestructorGuard dg(ptr);
  ptr->transport_->setReadCB(ptr);
  if (ptr->state_ != STREAMING) {
    throw std::runtime_error(
        "Failed to create McServerSession: setReadCB failed");
  }

  if (queue) {
    queue->push_front(*ptr);
  }

  // For secure connections, we need to delay calling the onAccepted client
  // callback until the handshake is complete.
  // we assume any secure transport will return non empty secure protocols
  if (ptr->transport_->getSecurityProtocol().empty()) {
    ptr->onAccepted();
  }

  return *ptr;
}

void McServerSession::applySocketOptions(
    folly::AsyncSocket& socket,
    const AsyncMcServerWorkerOptions& opts) {
  socket.setMaxReadsPerEvent(opts.maxReadsPerEvent);
  socket.setNoDelay(true);
  if (opts.tcpZeroCopyThresholdBytes > 0) {
    socket.setZeroCopy(true);
  }
  socket.setSendTimeout(opts.sendTimeout.count());
  if (opts.trafficClass > 0) {
    if (socket.setSockOpt(IPPROTO_IPV6, IPV6_TCLASS, &opts.trafficClass) != 0) {
      LOG_EVERY_N(ERROR, 1000) << "Failed to set TCLASS = " << opts.trafficClass
                               << " on socket. errno: " << errno;
    }
  }
}

McServerSession::McServerSession(
    folly::AsyncTransportWrapper::UniquePtr transport,
    std::shared_ptr<McServerOnRequest> cb,
    StateCallback& stateCb,
    const AsyncMcServerWorkerOptions& options,
    void* userCtxt,
    const CompressionCodecMap* codecMap,
    KeepAlive keepAlive)
    : options_(options),
      transport_(std::move(transport)),
      eventBase_(*transport_->getEventBase()),
      keepAlive_(std::move(keepAlive)),
      onRequest_(std::move(cb)),
      stateCb_(stateCb),
      sendWritesCallback_(*this),
      compressionCodecMap_(codecMap),
      parser_(*this, options_.minBufferSize, options_.maxBufferSize),
      userCtxt_(userCtxt),
      zeroCopySessionCB_(*this) {
  try {
    transport_->getPeerAddress(&socketAddress_);
  } catch (const std::exception& e) {
    // std::system_error or other exception, leave IP address empty
    LOG(WARNING) << "Failed to get socket address: " << e.what();
  }

  if (auto socket = transport_->getUnderlyingTransport<McFizzServer>()) {
    socket->accept(this);
  }
}

SecurityMech McServerSession::securityMech() const noexcept {
  return negotiatedMech_;
}

const apache::thrift::Cpp2RequestContext*
McServerSession::getConnectionLevelThriftRequestContext() const noexcept {
  return thriftRequestContext_->getRequestContext();
}

void McServerSession::pause(PauseReason reason) {
  pauseState_ |= static_cast<uint64_t>(reason);

  transport_->setReadCB(nullptr);
}

void McServerSession::resume(PauseReason reason) {
  pauseState_ &= ~static_cast<uint64_t>(reason);

  /* Client can half close the socket and in those cases there is
     no point in enabling reads */
  if (!pauseState_ && state_ == STREAMING && transport_->good()) {
    transport_->setReadCB(this);
  }
}

void McServerSession::onTransactionStarted(bool isSubRequest) {
  ++inFlight_;
  if (options_.maxInFlight > 0 && !isSubRequest) {
    if (++realRequestsInFlight_ >= options_.maxInFlight) {
      DestructorGuard dg(this);
      pause(PAUSE_THROTTLED);
    }
  }
}

void McServerSession::checkClosed() {
  if (!inFlight_ &&
      (!isZeroCopyEnabled() || (writeBufs_.zeroCopyQueueSize() == 0))) {
    assert(pendingWrites_.empty());

    if (state_ == CLOSING) {
      // It's possible to call close() more than once from the same stack.
      // Prevent second close() from doing anything
      state_ = CLOSED;
      // Call onCloseFinish() before transport reset to keep transport in tact
      onCloseFinish();
      if (transport_) {
        // prevent readEOF() from being called
        transport_->setReadCB(nullptr);
        transport_.reset();
      }
      destroy();
    }
  }
}

void McServerSession::onTransactionCompleted(bool isSubRequest) {
  DestructorGuard dg(this);

  assert(inFlight_ > 0);
  --inFlight_;
  if (options_.maxInFlight > 0 && !isSubRequest) {
    assert(realRequestsInFlight_ > 0);
    if (--realRequestsInFlight_ < options_.maxInFlight) {
      resume(PAUSE_THROTTLED);
    }
  }

  checkClosed();
}

void McServerSession::reply(std::unique_ptr<WriteBuffer> wb, uint64_t reqid) {
  DestructorGuard dg(this);

  if (parser_.outOfOrder()) {
    queueWrite(std::move(wb));
  } else {
    if (reqid == headReqid_) {
      /* head of line reply, write it and all contiguous blocked replies */
      queueWrite(std::move(wb));
      auto it = blockedReplies_.find(++headReqid_);
      while (it != blockedReplies_.end()) {
        queueWrite(std::move(it->second));
        blockedReplies_.erase(it);
        it = blockedReplies_.find(++headReqid_);
      }
    } else {
      /* can't write this reply now, save for later */
      blockedReplies_.emplace(reqid, std::move(wb));
    }
  }
}

void McServerSession::processMultiOpEnd() {
  currentMultiop_->recordEnd(tailReqid_++);
  currentMultiop_.reset();
}

void McServerSession::beginClose(folly::StringPiece reason) {
  if (options_.goAwayTimeout.count() == 0 ||
      parser_.protocol() != mc_caret_protocol) {
    close();
  } else {
    McServerRequestContext ctx(*this, kCaretConnectionControlReqId);
    GoAwayRequest goAway;
    goAway.reason_ref() = reason.str();
    McServerRequestContext::reply(std::move(ctx), std::move(goAway));
    goAwayTimeout_ = folly::AsyncTimeout::schedule(
        options_.goAwayTimeout, eventBase_, [this]() noexcept { close(); });
  }
}

void McServerSession::close() {
  DestructorGuard dg(this);

  // Reset timeout if set, since we're shutting down anyway.
  goAwayTimeout_ = nullptr;

  // Regardless of the reason we're closing, we should immediately stop reading
  // from the socket or we may get into invalid state.
  if (transport_) {
    transport_->setReadCB(nullptr);
  }

  if (currentMultiop_) {
    /* If we got closed in the middle of a multiop request,
       process it as if we saw the multi-op end sentinel */
    processMultiOpEnd();
  }

  if (state_ == STREAMING) {
    state_ = CLOSING;
    onCloseStart();
  }

  checkClosed();
}

void McServerSession::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  curBuffer_ = parser_.getReadBuffer();
  *bufReturn = curBuffer_.first;
  *lenReturn = curBuffer_.second;
}

void McServerSession::readDataAvailable(size_t len) noexcept {
  DestructorGuard dg(this);
  if (!parser_.readDataAvailable(len)) {
    close();
  }
}

void McServerSession::readEOF() noexcept {
  close();
}

void McServerSession::readErr(const folly::AsyncSocketException&) noexcept {
  close();
}

void McServerSession::multiOpEnd() {
  DestructorGuard dg(this);

  if (state_ != STREAMING) {
    return;
  }

  processMultiOpEnd();
}

void McServerSession::onRequest(
    McVersionRequest&& req,
    bool /* noreply = false */) {
  uint64_t reqid = 0;
  if (!parser_.outOfOrder()) {
    /* If we're already done streaming */
    if (state_ != STREAMING) {
      return;
    }
    reqid = tailReqid_++;
  }

  McServerRequestContext ctx(*this, reqid);

  if (options_.defaultVersionHandler) {
    McVersionReply reply(carbon::Result::OK);
    reply.value_ref() =
        folly::IOBuf(folly::IOBuf::COPY_BUFFER, options_.versionString);
    McServerRequestContext::reply(std::move(ctx), std::move(reply));
    return;
  }

  onRequest_->requestReady(std::move(ctx), std::move(req));
}

void McServerSession::onRequest(McShutdownRequest&&, bool) {
  uint64_t reqid = 0;
  if (!parser_.outOfOrder()) {
    reqid = tailReqid_++;
  }
  McServerRequestContext ctx(*this, reqid, true /* noReply */);
  McServerRequestContext::reply(
      std::move(ctx), McShutdownReply(carbon::Result::OK));
  stateCb_.onShutdown();
}

void McServerSession::onRequest(McQuitRequest&&, bool) {
  uint64_t reqid = 0;
  if (!parser_.outOfOrder()) {
    reqid = tailReqid_++;
  }
  McServerRequestContext ctx(*this, reqid, true /* noReply */);
  McServerRequestContext::reply(
      std::move(ctx), McQuitReply(carbon::Result::OK));
  close();
}

void McServerSession::caretRequestReady(
    const CaretMessageInfo& headerInfo,
    const folly::IOBuf& reqBody) {
  DestructorGuard dg(this);

  assert(parser_.protocol() == mc_caret_protocol);
  assert(parser_.outOfOrder());

  if (state_ != STREAMING) {
    return;
  }

  updateCompressionCodecIdRange(headerInfo);

  if (headerInfo.reqId == kCaretConnectionControlReqId) {
    processConnectionControlMessage(headerInfo);
    return;
  }

  McServerRequestContext ctx(*this, headerInfo.reqId);

  if (McVersionRequest::typeId == headerInfo.typeId &&
      options_.defaultVersionHandler) {
    McVersionReply versionReply(carbon::Result::OK);
    versionReply.value_ref() =
        folly::IOBuf(folly::IOBuf::COPY_BUFFER, options_.versionString);
    McServerRequestContext::reply(std::move(ctx), std::move(versionReply));
  } else {
    try {
      onRequest_->caretRequestReady(headerInfo, reqBody, std::move(ctx));
    } catch (const std::exception&) {
      // Ideally, ctx would be created after successful parsing of Caret data.
      // For now, if ctx hasn't been moved out of, mark as replied.
      ctx.replied_ = true;
      throw;
    }
  }
}

void McServerSession::processConnectionControlMessage(
    const CaretMessageInfo& headerInfo) {
  DestructorGuard dg(this);
  switch (headerInfo.typeId) {
    case GoAwayAcknowledgement::typeId: {
      // Client acknowledged GoAway, no new requests should be received, start
      // closing the connection.
      close();
      break;
    }
    default:
      // Unknown connection controll message, ignore it.
      break;
  }
}

void McServerSession::updateCompressionCodecIdRange(
    const CaretMessageInfo& headerInfo) noexcept {
  if (headerInfo.supportedCodecsSize == 0 || !compressionCodecMap_) {
    codecIdRange_ = CodecIdRange::Empty;
  } else {
    codecIdRange_ = {
        headerInfo.supportedCodecsFirstId, headerInfo.supportedCodecsSize};
  }
}

void McServerSession::parseError(
    carbon::Result result,
    folly::StringPiece reason) {
  DestructorGuard dg(this);

  if (state_ != STREAMING) {
    return;
  }

  McVersionReply errorReply(result);
  errorReply.message_ref() = reason.str();
  errorReply.value_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, reason.str());
  McServerRequestContext::reply(
      McServerRequestContext(*this, tailReqid_++), std::move(errorReply));
  close();
}

void McServerSession::sendZeroCopyIOBuf(
    WriteBuffer& wbuf,
    const struct iovec* iovs,
    size_t iovsCount) {
  DestructorGuard dg(this);
  using BufferContext = std::tuple<
      std::reference_wrapper<WriteBuffer>,
      std::reference_wrapper<WriteBufferQueue>,
      bool /* batch */,
      DestructorGuard>;

  // IOBuf FreeFn is a function pointer, so cannot use lambdas. Pass in
  // destructor guard to ensure that McServerSession is not destructed before
  // this IOBuf is destroyed.
  auto wbufInfo = new BufferContext(
      std::ref(wbuf), std::ref(writeBufs_), !options_.singleWrite, dg);
  assert(wbufInfo != nullptr);
  wbuf.setZeroCopyPendingNotifications(iovsCount);
  std::unique_ptr<folly::IOBuf> chainTail;
  for (size_t i = 0; i < iovsCount; i++) {
    size_t len = iovs[i].iov_len;
    if (len > 0) {
      std::unique_ptr<folly::IOBuf> iobuf = folly::IOBuf::takeOwnership(
          iovs[i].iov_base,
          len,
          [](void* /* unused */, void* userData) {
            assert(userData != nullptr);
            BufferContext* bufferContext =
                reinterpret_cast<BufferContext*>(userData);
            auto& wb = std::get<0>(*bufferContext).get();
            if (wb.decZeroCopyPendingNotifications() == 0) {
              auto& q = std::get<1>(*bufferContext).get();
              auto batch = std::get<2>(*bufferContext);
              q.releaseZeroCopyChain(wb, batch);
              delete (bufferContext);
            }
          },
          wbufInfo,
          true /* freeOnError */);
      if (!chainTail) {
        chainTail = std::move(iobuf);
      } else {
        chainTail->prependChain(std::move(iobuf));
      }
    } else {
      wbuf.decZeroCopyPendingNotifications();
    }
  }

  zeroCopySessionCB_.incCallbackPending();
  transport_->writeChain(
      &zeroCopySessionCB_ /* write cb */,
      std::move(chainTail),
      folly::WriteFlags::WRITE_MSG_ZEROCOPY);
}

void McServerSession::queueWrite(std::unique_ptr<WriteBuffer> wb) {
  if (wb == nullptr) {
    return;
  }
  if (options_.singleWrite) {
    if (FOLLY_UNLIKELY(debugFifo_.isConnected())) {
      writeToDebugFifo(wb.get());
    }
    const struct iovec* iovs = wb->getIovsBegin();
    size_t iovCount = wb->getIovsCount();
    if (isZeroCopyEnabled() && wb->shouldApplyZeroCopy()) {
      auto& wbuf = writeBufs_.insertZeroCopy(std::move(wb));
      // Creates a chain of IOBufs and uses TCP copy avoidance
      sendZeroCopyIOBuf(wbuf, iovs, iovCount);
      if (zeroCopySessionCB_.getCallbackPending() > 0) {
        pause(PAUSE_WRITE);
      }
    } else {
      writeBufs_.push(std::move(wb));
      transport_->writev(this, iovs, iovCount);
      if (!writeBufs_.empty()) {
        /* We only need to pause if the sendmsg() call didn't write everything
           in one go */
        pause(PAUSE_WRITE);
      }
    }
  } else {
    if (!writeScheduled_) {
      eventBase_.runInLoop(&sendWritesCallback_, /* thisIteration= */ true);
      writeScheduled_ = true;
    }
    if (isZeroCopyEnabled() && wb->shouldApplyZeroCopy()) {
      isNextWriteBatchZeroCopy_ = true;
    }
    pendingWrites_.pushBack(std::move(wb));
  }
}

void McServerSession::sendWrites() {
  DestructorGuard dg(this);

  if (pendingWrites_.empty()) {
    return;
  }

  bool doZeroCopy = isNextWriteBatchZeroCopy_;
  writeScheduled_ = false;
  isNextWriteBatchZeroCopy_ = false;

  folly::small_vector<struct iovec, kIovecVectorSize> iovs;
  WriteBuffer* firstBuf = nullptr;
  while (!pendingWrites_.empty()) {
    auto wb = pendingWrites_.popFront();
    if (!wb->noReply()) {
      if (FOLLY_UNLIKELY(debugFifo_.isConnected())) {
        writeToDebugFifo(wb.get());
      }
      iovs.insert(
          iovs.end(),
          wb->getIovsBegin(),
          wb->getIovsBegin() + wb->getIovsCount());
    }
    if (pendingWrites_.empty()) {
      wb->markEndOfBatch();
    }
    if (doZeroCopy) {
      if (!firstBuf) {
        firstBuf = &writeBufs_.insertZeroCopy(std::move(wb));
      } else {
        writeBufs_.insertZeroCopy(std::move(wb));
      }
    } else {
      writeBufs_.push(std::move(wb));
    }
  }

  if (doZeroCopy) {
    assert(firstBuf != nullptr);
    sendZeroCopyIOBuf(*firstBuf, iovs.data(), iovs.size());
  } else {
    transport_->writev(this, iovs.data(), iovs.size());
  }
}

void McServerSession::writeToDebugFifo(const WriteBuffer* wb) noexcept {
  if (!wb->isSubRequest()) {
    debugFifo_.startMessage(MessageDirection::Sent, wb->typeId());
    hasPendingMultiOp_ = false;
  } else {
    // Handle multi-op
    if (!hasPendingMultiOp_) {
      debugFifo_.startMessage(MessageDirection::Sent, wb->typeId());
      hasPendingMultiOp_ = true;
    }
    if (wb->isEndContext()) {
      // Multi-op replies always finish with an end context
      hasPendingMultiOp_ = false;
    }
  }
  debugFifo_.writeData(wb->getIovsBegin(), wb->getIovsCount());
}

void McServerSession::completeWrite() {
  writeBufs_.pop(!options_.singleWrite /* popBatch */);
}

void McServerSession::writeSuccess() noexcept {
  DestructorGuard dg(this);
  completeWrite();

  if (writeBufs_.empty() && state_ == STREAMING) {
    onWriteQuiescence();
    /* No-op if not paused */
    resume(PAUSE_WRITE);
  }
}

void McServerSession::writeErr(
    size_t /* bytesWritten */,
    const folly::AsyncSocketException&) noexcept {
  DestructorGuard dg(this);
  completeWrite();
  close();
}

bool McServerSession::handshakeVer(
    folly::AsyncSSLSocket* sock,
    bool preverifyOk,
    X509_STORE_CTX* ctx) noexcept {
  return McSSLUtil::verifySSL(sock, preverifyOk, ctx);
}

void McServerSession::handshakeSuc(folly::AsyncSSLSocket* sock) noexcept {
  DestructorGuard dg(this);

  negotiatedMech_ = SecurityMech::TLS;
  auto maybeCN = getCN(sock->getPeerCertificate());
  if (maybeCN.hasValue()) {
    clientCommonName_.assign(*maybeCN);
  }

  if (McSSLUtil::negotiatedPlaintextFallback(*sock)) {
    auto fallback = McSSLUtil::moveToPlaintext(*sock);
    CHECK(fallback);
    auto asyncSock = fallback->getUnderlyingTransport<folly::AsyncSocket>();
    CHECK(asyncSock);
    applySocketOptions(*asyncSock, options_);
    transport_.reset(fallback.release());
    negotiatedMech_ = SecurityMech::TLS_TO_PLAINTEXT;
  } else if (options_.useKtls12) {
    // try to flip to using ktls
    if (auto ktlsTransport = McSSLUtil::moveToKtls(*sock)) {
      auto asyncSock =
          ktlsTransport->getUnderlyingTransport<folly::AsyncSocket>();
      CHECK(asyncSock);
      applySocketOptions(*asyncSock, options_);
      transport_.reset(ktlsTransport.release());
      negotiatedMech_ = SecurityMech::KTLS12;
    }
  }

  // sock is currently wrapped by transport_, but underlying socket may
  // change by end of this function due to negotiatedPlaintextFallback or ktls.
  transport_->setReadCB(this);

  onAccepted();
}

void McServerSession::handshakeErr(
    folly::AsyncSSLSocket*,
    const folly::AsyncSocketException& e) noexcept {
  auto type = e.getType();
  if (type !=
      folly::AsyncSocketException::AsyncSocketExceptionType::SSL_ERROR) {
    LOG_EVERY_N(ERROR, 10000) << "SSL handshake failure: " << e.what();
  } else {
    LOG_EVERY_N(ERROR, 100) << "SSL Handshake failure: " << e.what();
  }
  close();
}

void McServerSession::fizzHandshakeSuccess(
    fizz::server::AsyncFizzServer* transport) noexcept {
  DestructorGuard dg(this);

  negotiatedMech_ = SecurityMech::TLS13_FIZZ;
  auto maybeCN = getCN(transport->getPeerCertificate());
  if (maybeCN.hasValue()) {
    clientCommonName_.assign(*maybeCN);
  }
  onAccepted();
}

void McServerSession::fizzHandshakeError(
    fizz::server::AsyncFizzServer*,
    folly::exception_wrapper e) noexcept {
  e.handle(
      [socketAddress = socketAddress_](const folly::AsyncSocketException& ex) {
        auto type = ex.getType();
        // we log non SSL errors less frequently as they are most likely network
        // related / not specific to SSL itself
        if (type !=
            folly::AsyncSocketException::AsyncSocketExceptionType::SSL_ERROR) {
          LOG_EVERY_N(ERROR, 10000) << "Fizz Handshake failure: " << ex.what()
                                    << ". Peer IP address: " << socketAddress;
        } else {
          LOG_EVERY_N(ERROR, 100) << "Fizz Handshake failure: " << ex.what()
                                  << ". Peer IP address: " << socketAddress;
        }
      },
      [socketAddress = socketAddress_](const std::exception& ex) {
        LOG_EVERY_N(ERROR, 100) << "Fizz Handshake failure: " << ex.what()
                                << ". Peer IP address: " << socketAddress;
      });
  close();
}

void McServerSession::fizzHandshakeAttemptFallback(
    fizz::server::AttemptVersionFallback fallback) {
  DestructorGuard dg(this);
  auto transport = transport_->getUnderlyingTransport<McFizzServer>();
  CHECK(transport) << " transport should not be nullptr";
  transport->setReadCB(nullptr);
  auto evb = transport->getEventBase();
  auto socket = transport->getUnderlyingTransport<folly::AsyncSocket>();
  CHECK(socket) << " socket should not be nullptr";
  auto fd = socket->detachNetworkSocket().toFd();
  const auto& ctx = transport->getFallbackContext();

  folly::AsyncSSLSocket::UniquePtr sslSocket(new folly::AsyncSSLSocket(
      ctx, evb, folly::NetworkSocket::fromFd(fd), true /* server */));
  sslSocket->setPreReceivedData(std::move(fallback.clientHello));
  sslSocket->enableClientHelloParsing();
  sslSocket->forceCacheAddrOnFailure(true);
  // need to re apply the socket options
  applySocketOptions(*sslSocket, options_);

  // We need to reset the transport before calling sslAccept(). The reason is
  // that sslAccept() may call some callbacks inline (e.g. handshakeSuc()) that
  // may need to see the actual transport_.
  transport_.reset(sslSocket.release());
  auto underlyingSslSocket =
      transport_->getUnderlyingTransport<folly::AsyncSSLSocket>();
  DCHECK(underlyingSslSocket) << "Underlying socket should be AsyncSSLSocket";
  underlyingSslSocket->sslAccept(this);
}

void McServerSession::onAccepted() {
  DCHECK(!onAcceptedCalled_);
  DCHECK(transport_);
  /* Initializes request context, used for identity extraction downstream */
  thriftRequestContext_ =
      std::make_unique<const McServerThriftRequestContext>(transport_.get());
  /* Trims the certificate memory */
  if (auto sock = transport_->getUnderlyingTransport<folly::AsyncSSLSocket>()) {
    McSSLUtil::dropCertificateX509Payload(*sock);
  }
  debugFifo_ = getDebugFifo(
      options_.debugFifoPath, transport_.get(), onRequest_->name());
  parser_.setDebugFifo(&debugFifo_);
  onAcceptedCalled_ = true;
  stateCb_.onAccepted(*this);
}

void McServerSession::onCloseStart() {
  stateCb_.onCloseStart(*this);
}

void McServerSession::onCloseFinish() {
  stateCb_.onCloseFinish(*this, onAcceptedCalled_);
}

void McServerSession::onWriteQuiescence() {
  stateCb_.onWriteQuiescence(*this);
}

} // namespace memcache
} // namespace facebook
