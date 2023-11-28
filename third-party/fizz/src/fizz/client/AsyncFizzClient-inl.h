/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace client {

template <typename SM>
AsyncFizzClientT<SM>::AsyncFizzClientT(
    folly::AsyncTransportWrapper::UniquePtr socket,
    std::shared_ptr<const FizzClientContext> fizzContext,
    const std::shared_ptr<ClientExtensions>& extensions,
    AsyncFizzBase::TransportOptions transportOptions)
    : AsyncFizzBase(std::move(socket), std::move(transportOptions)),
      fizzContext_(std::move(fizzContext)),
      extensions_(extensions),
      visitor_(*this),
      fizzClient_(state_, transportReadBuf_, readAeadOptions_, visitor_, this) {
}

template <typename SM>
AsyncFizzClientT<SM>::AsyncFizzClientT(
    folly::EventBase* eventBase,
    std::shared_ptr<const FizzClientContext> fizzContext,
    const std::shared_ptr<ClientExtensions>& extensions,
    AsyncFizzBase::TransportOptions transportOptions)
    : AsyncFizzBase(
          folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(eventBase)),
          std::move(transportOptions)),
      fizzContext_(std::move(fizzContext)),
      extensions_(extensions),
      visitor_(*this),
      fizzClient_(state_, transportReadBuf_, readAeadOptions_, visitor_, this) {
}

template <typename SM>
void AsyncFizzClientT<SM>::connect(
    HandshakeCallback* callback,
    folly::Optional<std::string> hostname,
    folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
    std::chrono::milliseconds timeout) {
  auto pskIdentity = hostname;
  connect(
      callback,
      std::make_shared<DefaultCertificateVerifier>(VerificationContext::Client),
      std::move(hostname),
      std::move(pskIdentity),
      std::move(echConfigs),
      std::move(timeout));
}

template <typename SM>
void AsyncFizzClientT<SM>::connect(
    HandshakeCallback* callback,
    std::shared_ptr<const CertificateVerifier> verifier,
    folly::Optional<std::string> sni,
    folly::Optional<std::string> pskIdentity,
    folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
    std::chrono::milliseconds timeout) {
  DelayedDestruction::DestructorGuard dg(this);

  // shouldn't attempt to connect a second time
  CHECK(!callback_.hasValue()) << "connect already called";
  callback_.emplace(callback);

  if (!transport_->good()) {
    folly::AsyncSocketException ase(
        folly::AsyncSocketException::NOT_OPEN,
        "handshake connect called but socket isn't open");
    deliverAllErrors(ase, false);
    return;
  }

  sni_ = sni;
  pskIdentity_ = pskIdentity;

  if (timeout != std::chrono::milliseconds::zero()) {
    startHandshakeTimeout(timeout);
  }

  startTransportReads();

  folly::Optional<CachedPsk> cachedPsk = folly::none;
  if (pskIdentity) {
    cachedPsk = fizzContext_->getPsk(*pskIdentity);
  }

  auto echPolicy = fizzContext_->getECHPolicy();
  if (!echConfigs && echPolicy && sni.hasValue()) {
    echConfigs = echPolicy->getConfig(sni.value());
  }

  fizzClient_.connect(
      fizzContext_,
      std::move(verifier),
      std::move(sni),
      std::move(cachedPsk),
      std::move(echConfigs),
      extensions_);
}

template <typename SM>
void AsyncFizzClientT<SM>::connect(
    const folly::SocketAddress& connectAddr,
    folly::AsyncSocket::ConnectCallback* callback,
    std::shared_ptr<const CertificateVerifier> verifier,
    folly::Optional<std::string> sni,
    folly::Optional<std::string> pskIdentity,
    std::chrono::milliseconds totalTimeout,
    std::chrono::milliseconds socketTimeout,
    const folly::SocketOptionMap& options,
    const folly::SocketAddress& bindAddr) {
  DelayedDestruction::DestructorGuard dg(this);

  // shouldn't attempt to connect a second time
  CHECK(!callback_.hasValue()) << "connect already called";
  callback_.emplace(callback);

  verifier_ = std::move(verifier);
  sni_ = sni;
  pskIdentity_ = pskIdentity;

  if (totalTimeout != std::chrono::milliseconds::zero()) {
    startHandshakeTimeout(std::move(totalTimeout));
  }

  auto underlyingSocket =
      transport_->getUnderlyingTransport<folly::AsyncSocket>();
  if (underlyingSocket) {
    underlyingSocket->disableTransparentTls();
    underlyingSocket->connect(
        this,
        connectAddr,
        static_cast<int>(socketTimeout.count()),
        options,
        bindAddr);
  } else {
    folly::AsyncSocketException ase(
        folly::AsyncSocketException::BAD_ARGS,
        "could not find underlying socket");
    deliverAllErrors(ase, false);
  }
}

template <typename SM>
bool AsyncFizzClientT<SM>::good() const {
  return !error() && !fizzClient_.inTerminalState() && transport_->good();
}

template <typename SM>
bool AsyncFizzClientT<SM>::readable() const {
  return transport_->readable();
}

template <typename SM>
bool AsyncFizzClientT<SM>::connecting() const {
  return callback_ || transport_->connecting();
}

template <typename SM>
bool AsyncFizzClientT<SM>::error() const {
  return transport_->error() || fizzClient_.inErrorState();
}

template <typename SM>
const Cert* AsyncFizzClientT<SM>::getPeerCertificate() const {
  return earlyDataState_ ? getState().earlyDataParams()->serverCert.get()
                         : getState().serverCert().get();
}

template <typename SM>
const Cert* AsyncFizzClientT<SM>::getSelfCertificate() const {
  return earlyDataState_ ? getState().earlyDataParams()->clientCert.get()
                         : getState().clientCert().get();
}

template <typename SM>
bool AsyncFizzClientT<SM>::isReplaySafe() const {
  return !earlyDataState_.hasValue();
}

template <typename SM>
void AsyncFizzClientT<SM>::setReplaySafetyCallback(
    folly::AsyncTransport::ReplaySafetyCallback* callback) {
  DCHECK(!callback || !isReplaySafe());
  replaySafetyCallback_ = callback;
}

template <typename SM>
std::string AsyncFizzClientT<SM>::getApplicationProtocol() const noexcept {
  if (earlyDataState_) {
    if (getState().earlyDataParams()->alpn) {
      return *getState().earlyDataParams()->alpn;
    } else {
      return "";
    }
  } else {
    if (getState().alpn()) {
      return *getState().alpn();
    } else {
      return "";
    }
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::close() {
  if (transport_->good()) {
    fizzClient_.appCloseImmediate();
  } else {
    DelayedDestruction::DestructorGuard dg(this);
    folly::AsyncSocketException ase(
        folly::AsyncSocketException::END_OF_FILE, "socket closed locally");
    deliverAllErrors(ase, false);
    transport_->close();
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::tlsShutdown() {
  if (transport_->good()) {
    // do not immediately close, wait to receive a close notify from the other
    // end
    fizzClient_.appClose();
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::shutdownWrite() {
  DelayedDestruction::DestructorGuard dg(this);
  // allow any previous writes as well as the close_notify to be written out
  // before closing the socket
  tlsShutdown();
  transport_->shutdownWrite();
}

template <typename SM>
void AsyncFizzClientT<SM>::shutdownWriteNow() {
  DelayedDestruction::DestructorGuard dg(this);
  // try to write out a close_notify although the ensuing shutdownWriteNow call
  // on the underlying socket might prevent these bytes (and any other previous
  // writes) from making it through
  tlsShutdown();
  transport_->shutdownWriteNow();
}

template <typename SM>
void AsyncFizzClientT<SM>::closeWithReset() {
  DelayedDestruction::DestructorGuard dg(this);
  if (transport_->good()) {
    fizzClient_.appCloseImmediate();
  }
  folly::AsyncSocketException ase(
      folly::AsyncSocketException::END_OF_FILE, "socket closed locally");
  deliverAllErrors(ase, false);
  transport_->closeWithReset();
}

template <typename SM>
void AsyncFizzClientT<SM>::closeNow() {
  DelayedDestruction::DestructorGuard dg(this);
  if (transport_->good()) {
    fizzClient_.appCloseImmediate();
  }
  folly::AsyncSocketException ase(
      folly::AsyncSocketException::END_OF_FILE, "socket closed locally");
  deliverAllErrors(ase, false);
  transport_->closeNow();
}

template <typename SM>
void AsyncFizzClientT<SM>::connectSuccess() noexcept {
  startTransportReads();

  folly::Optional<CachedPsk> cachedPsk = folly::none;
  if (pskIdentity_) {
    cachedPsk = fizzContext_->getPsk(*pskIdentity_);
  }
  fizzClient_.connect(
      fizzContext_,
      std::move(verifier_),
      sni_,
      std::move(cachedPsk),
      folly::Optional<std::vector<ech::ECHConfig>>(folly::none),
      extensions_);
}

template <typename SM>
void AsyncFizzClientT<SM>::connectErr(
    const folly::AsyncSocketException& ex) noexcept {
  deliverAllErrors(ex, false);
}

template <typename SM>
void AsyncFizzClientT<SM>::writeAppData(
    folly::AsyncTransportWrapper::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf>&& buf,
    folly::WriteFlags flags) {
  if (!good()) {
    if (callback) {
      callback->writeErr(
          0,
          folly::AsyncSocketException(
              folly::AsyncSocketException::INVALID_STATE,
              "fizz app write in error state"));
    }
    return;
  }

  if (earlyDataState_) {
    auto size = buf->computeChainDataLength();
    if (!earlyDataState_->pendingAppWrites.empty() ||
        size > earlyDataState_->remainingEarlyData) {
      AppWrite w;
      w.callback = callback;
      w.data = std::move(buf);
      w.flags = flags;
      w.aeadOptions = writeAeadOptions_;

      earlyDataState_->remainingEarlyData = 0;
      earlyDataState_->pendingAppWrites.push_back(std::move(w));
    } else {
      EarlyAppWrite w;
      w.callback = callback;
      w.data = std::move(buf);
      w.flags = flags;
      w.aeadOptions = writeAeadOptions_;

      if (earlyDataRejectionPolicy_ ==
          EarlyDataRejectionPolicy::AutomaticResend) {
        // We need to call unshare() to make a copy of the data here since we
        // may need to resend it after we've already called writeSuccess().
        // Particularly when using the write and writev interfaces, the
        // application is allowed to delete the underlying buffer after getting
        // the write callback.
        auto writeCopy = w.data->clone();
        writeCopy->unshare();
        earlyDataState_->resendBuffer.append(std::move(writeCopy));
      }

      earlyDataState_->remainingEarlyData -= size;
      fizzClient_.earlyAppWrite(std::move(w));
    }
  } else {
    AppWrite w;
    w.callback = callback;
    w.data = std::move(buf);
    w.flags = flags;
    w.aeadOptions = writeAeadOptions_;

    // Instead of dealing with the ordering of all 3 potential queues (early
    // data resend buffer, early data pending writes, and pending handshake
    // writes), we only queue up pending handshake writes if early data is
    // disabled through the user submitted FizzClientContext configuration.
    // Otherwise, we revert to the previous behavior where the write would fail
    // at the transport level.
    if (connecting() && !fizzContext_->getSendEarlyData()) {
      // underlying socket hasn't been connected, collect app data writes until
      // it is, then flush them
      pendingHandshakeAppWrites_.push_back(std::move(w));
    } else {
      fizzClient_.appWrite(std::move(w));
    }
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::transportError(
    const folly::AsyncSocketException& ex) {
  DelayedDestruction::DestructorGuard dg(this);
  deliverAllErrors(ex);
}

template <typename SM>
void AsyncFizzClientT<SM>::transportDataAvailable() {
  fizzClient_.newTransportData();
}

template <typename SM>
void AsyncFizzClientT<SM>::pauseEvents() {
  fizzClient_.pause();
}

template <typename SM>
void AsyncFizzClientT<SM>::resumeEvents() {
  fizzClient_.resume();
}

template <typename SM>
void AsyncFizzClientT<SM>::deliverAllErrors(
    const folly::AsyncSocketException& ex,
    bool closeTransport) {
  DelayedDestruction::DestructorGuard dg(this);
  deliverHandshakeError(ex);

  if (replaySafetyCallback_) {
    replaySafetyCallback_ = nullptr;
  }

  // if there are writes pending, call their error callback and clear the queue
  while (!pendingHandshakeAppWrites_.empty()) {
    auto w = std::move(pendingHandshakeAppWrites_.front());
    pendingHandshakeAppWrites_.pop_front();
    if (w.callback) {
      w.callback->writeErr(0, ex);
    }
  }

  while (earlyDataState_ && !earlyDataState_->pendingAppWrites.empty()) {
    auto w = std::move(earlyDataState_->pendingAppWrites.front());
    earlyDataState_->pendingAppWrites.pop_front();
    if (w.callback) {
      w.callback->writeErr(0, ex);
    }
  }
  fizzClient_.moveToErrorState(ex);
  deliverError(ex, closeTransport);
}

template <typename SM>
void AsyncFizzClientT<SM>::deliverHandshakeError(folly::exception_wrapper ex) {
  if (callback_) {
    cancelHandshakeTimeout();
    auto cb = callback_;
    callback_ = folly::none;
    switch (cb->type()) {
      case AsyncClientCallbackPtr::Type::HandshakeCallback:
        if (cb->asHandshakeCallbackPtr()) {
          cb->asHandshakeCallbackPtr()->fizzHandshakeError(this, std::move(ex));
        }
        break;
      case AsyncClientCallbackPtr::Type::AsyncSocketConnCallback:
        if (cb->asAsyncSocketConnCallbackPtr()) {
          auto* callback = cb->asAsyncSocketConnCallbackPtr();
          ex.handle(
              [callback](const folly::AsyncSocketException& ase) {
                callback->connectErr(ase);
              },
              [callback](const std::exception& stdEx) {
                folly::AsyncSocketException ase(
                    folly::AsyncSocketException::SSL_ERROR, stdEx.what());
                callback->connectErr(ase);
              },
              [callback](...) {
                folly::AsyncSocketException ase(
                    folly::AsyncSocketException::SSL_ERROR, "unknown error");
                callback->connectErr(ase);
              });
          break;
        }
    }
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(DeliverAppData& data) {
  client_.deliverAppData(std::move(data.data));
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(WriteToSocket& data) {
  DCHECK(!data.contents.empty());
  Buf allData = std::move(data.contents.front().data);
  for (size_t i = 1; i < data.contents.size(); ++i) {
    allData->prependChain(std::move(data.contents[i].data));
  }
  client_.transport_->writeChain(data.callback, std::move(allData), data.flags);
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(
    ReportEarlyHandshakeSuccess& earlySuccess) {
  client_.earlyDataState_ = EarlyDataState();
  client_.earlyDataState_->remainingEarlyData = earlySuccess.maxEarlyDataSize;
  if (client_.callback_) {
    auto cb = client_.callback_;
    client_.callback_ = folly::none;
    switch (cb->type()) {
      case AsyncClientCallbackPtr::Type::HandshakeCallback:
        if (cb->asHandshakeCallbackPtr()) {
          cb->asHandshakeCallbackPtr()->fizzHandshakeSuccess(&client_);
        }
        break;
      case AsyncClientCallbackPtr::Type::AsyncSocketConnCallback:
        if (cb->asAsyncSocketConnCallbackPtr()) {
          cb->asAsyncSocketConnCallbackPtr()->connectSuccess();
        }
        break;
    }
  }
}

template <typename SM>
folly::Optional<folly::AsyncSocketException>
AsyncFizzClientT<SM>::handleEarlyReject() {
  switch (earlyDataRejectionPolicy_) {
    case EarlyDataRejectionPolicy::FatalConnectionError: {
      return folly::AsyncSocketException(
          folly::AsyncSocketException::EARLY_DATA_REJECTED,
          "fizz early data rejected");
    }
    case EarlyDataRejectionPolicy::AutomaticResend: {
      if (earlyParametersMatch(getState())) {
        if (!earlyDataState_->resendBuffer.empty()) {
          AppWrite resend;
          resend.data = earlyDataState_->resendBuffer.move();
          fizzClient_.appWrite(std::move(resend));
        }
      } else {
        return folly::AsyncSocketException(
            folly::AsyncSocketException::EARLY_DATA_REJECTED,
            "fizz early data rejected, could not be resent");
      }
      break;
    }
  }
  return folly::none;
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(
    ReportHandshakeSuccess& success) {
  client_.cancelHandshakeTimeout();

  // Disable record aligned reads. At this point, we are aligned on a record
  // boundary (if handshakeRecordAlignedReads = true).
  client_.updateReadHint(0);

  // if there are app writes pending this handshake success, flush them first,
  // then flush the early data buffers
  auto& pendingHandshakeAppWrites = client_.pendingHandshakeAppWrites_;
  while (!pendingHandshakeAppWrites.empty()) {
    auto w = std::move(pendingHandshakeAppWrites.front());
    pendingHandshakeAppWrites.pop_front();
    client_.fizzClient_.appWrite(std::move(w));
  }

  if (client_.earlyDataState_) {
    if (!success.earlyDataAccepted) {
      auto ex = client_.handleEarlyReject();
      if (ex) {
        if (client_.pskIdentity_) {
          client_.fizzContext_->removePsk(*client_.pskIdentity_);
        }
        client_.deliverAllErrors(*ex, false);
        client_.transport_->closeNow();
        return;
      }
    }

    while (!client_.earlyDataState_->pendingAppWrites.empty()) {
      auto w = std::move(client_.earlyDataState_->pendingAppWrites.front());
      client_.earlyDataState_->pendingAppWrites.pop_front();
      client_.fizzClient_.appWrite(std::move(w));
    }
    client_.earlyDataState_.clear();
  }
  if (client_.callback_) {
    auto cb = client_.callback_;
    client_.callback_ = folly::none;
    switch (cb->type()) {
      case AsyncClientCallbackPtr::Type::HandshakeCallback:
        if (cb->asHandshakeCallbackPtr()) {
          cb->asHandshakeCallbackPtr()->fizzHandshakeSuccess(&client_);
        }
        break;
      case AsyncClientCallbackPtr::Type::AsyncSocketConnCallback:
        if (cb->asAsyncSocketConnCallbackPtr()) {
          cb->asAsyncSocketConnCallbackPtr()->connectSuccess();
        }
        break;
    }
  }
  if (client_.replaySafetyCallback_) {
    auto callback = client_.replaySafetyCallback_;
    client_.replaySafetyCallback_ = nullptr;
    callback->onReplaySafe();
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(
    ReportEarlyWriteFailed& write) {
  // If the state machine reports that an early write happened after early data
  // was already rejected, we need to invoke some write callback so that the
  // write isn't leaked. For now we just call writeSuccess and let the actual
  // rejection or early data get sorted out after full handshake success.
  //
  // TODO: buffer these callbacks until full handshake success, and call
  //       writeSuccess/writeErr depending on whether we are treating rejection
  //       as a fatal error.
  if (write.write.callback) {
    write.write.callback->writeSuccess();
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(ReportError& error) {
  folly::AsyncSocketException ase(
      folly::AsyncSocketException::SSL_ERROR, error.error.what().toStdString());
  client_.deliverHandshakeError(std::move(error.error));
  client_.deliverAllErrors(ase);
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(WaitForData& wfd) {
  client_.fizzClient_.waitForData();
  client_.updateReadHint(wfd.recordSizeHint);
  if (client_.callback_) {
    // Make sure that the read callback is installed.
    client_.startTransportReads();
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(MutateState& mutator) {
  mutator(client_.state_);
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(
    NewCachedPsk& newCachedPsk) {
  if (client_.pskIdentity_) {
    client_.fizzContext_->putPsk(
        *client_.pskIdentity_, std::move(newCachedPsk.psk));
  }
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(
    SecretAvailable& secret) {
  fizz_probe_secret_available(
      secret.secret.secret.size(),
      secret.secret.secret.data(),
      KeyLogWriter::secretToNSSLabel(secret.secret.type)
          .value_or(std::numeric_limits<KeyLogWriter::Label>::max()),
      client_.getClientRandom()->data());

  client_.secretAvailable(secret.secret);
}

template <typename SM>
void AsyncFizzClientT<SM>::ActionMoveVisitor::operator()(EndOfData& eod) {
  client_.endOfTLS(std::move(eod.postTlsData));
}

template <typename SM>
folly::Optional<CipherSuite> AsyncFizzClientT<SM>::getCipher() const {
  return getState().cipher();
}

template <typename SM>
folly::Optional<NamedGroup> AsyncFizzClientT<SM>::getGroup() const {
  return getState().group();
}

template <typename SM>
std::vector<SignatureScheme> AsyncFizzClientT<SM>::getSupportedSigSchemes()
    const {
  return getState().context()->getSupportedSigSchemes();
}

template <typename SM>
Buf AsyncFizzClientT<SM>::getExportedKeyingMaterial(
    folly::StringPiece label,
    Buf context,
    uint16_t length) const {
  return fizzClient_.getExportedKeyingMaterial(
      *fizzContext_->getFactory(), label, std::move(context), length);
}

template <typename SM>
Buf AsyncFizzClientT<SM>::getEarlyEkm(
    folly::StringPiece label,
    const Buf& context,
    uint16_t length) const {
  return fizzClient_.getEarlyEkm(
      *fizzContext_->getFactory(), label, context, length);
}

template <typename SM>
bool AsyncFizzClientT<SM>::pskResumed() const {
  return getState().pskMode().has_value();
}

template <typename SM>
folly::Optional<Random> AsyncFizzClientT<SM>::getClientRandom() const {
  return getState().clientRandom();
}

template <typename SM>
void AsyncFizzClientT<SM>::initiateKeyUpdate(
    KeyUpdateRequest keyUpdateRequest) {
  KeyUpdateInitiation kui;
  kui.request_update = keyUpdateRequest;
  fizzClient_.initiateKeyUpdate(std::move(kui));
}

template <typename SM>
bool AsyncFizzClientT<SM>::echRequested() const {
  return getState().echState().has_value();
}

template <typename SM>
bool AsyncFizzClientT<SM>::echAccepted() const {
  return echRequested() && getState().echState()->status == ECHStatus::Accepted;
}

template <typename SM>
folly::Optional<std::vector<ech::ECHConfig>>
AsyncFizzClientT<SM>::getEchRetryConfigs() const {
  if (!getState().echState().has_value() ||
      !getState().echState()->retryConfigs.has_value()) {
    return folly::none;
  }
  return getState().echState()->retryConfigs.value();
}
} // namespace client
} // namespace fizz
