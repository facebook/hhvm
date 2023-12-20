/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace server {

template <typename SM>
AsyncFizzServerT<SM>::AsyncFizzServerT(
    folly::AsyncTransportWrapper::UniquePtr socket,
    const std::shared_ptr<const FizzServerContext>& fizzContext,
    const std::shared_ptr<ServerExtensions>& extensions,
    AsyncFizzBase::TransportOptions transportOptions)
    : AsyncFizzBase(std::move(socket), std::move(transportOptions)),
      fizzContext_(fizzContext),
      extensions_(extensions),
      visitor_(*this),
      fizzServer_(state_, transportReadBuf_, readAeadOptions_, visitor_, this) {
}

template <typename SM>
void AsyncFizzServerT<SM>::accept(HandshakeCallback* callback) {
  handshakeCallback_ = callback;

  fizzServer_.accept(transport_->getEventBase(), fizzContext_, extensions_);
  startTransportReads();
}

template <typename SM>
bool AsyncFizzServerT<SM>::good() const {
  return !error() && !fizzServer_.inTerminalState() && transport_->good();
}

template <typename SM>
bool AsyncFizzServerT<SM>::readable() const {
  return transport_->readable();
}

template <typename SM>
bool AsyncFizzServerT<SM>::connecting() const {
  return handshakeCallback_ || transport_->connecting();
}

template <typename SM>
bool AsyncFizzServerT<SM>::error() const {
  return transport_->error() || fizzServer_.inErrorState();
}

template <typename SM>
bool AsyncFizzServerT<SM>::isDetachable() const {
  return !fizzServer_.actionProcessing() && AsyncFizzBase::isDetachable();
}

template <typename SM>
void AsyncFizzServerT<SM>::attachEventBase(folly::EventBase* evb) {
  state_.executor() = evb;
  AsyncFizzBase::attachEventBase(evb);
}

template <typename SM>
const Cert* AsyncFizzServerT<SM>::getPeerCertificate() const {
  return getState().clientCert().get();
}

template <typename SM>
const Cert* AsyncFizzServerT<SM>::getSelfCertificate() const {
  return getState().serverCert().get();
}

template <typename SM>
bool AsyncFizzServerT<SM>::isReplaySafe() const {
  // Server always provides replay protection.
  return true;
}

template <typename SM>
void AsyncFizzServerT<SM>::setReplaySafetyCallback(
    folly::AsyncTransport::ReplaySafetyCallback*) {
  LOG(FATAL) << "setReplaySafetyCallback() called on replay safe transport";
}

template <typename SM>
std::string AsyncFizzServerT<SM>::getApplicationProtocol() const noexcept {
  if (getState().alpn()) {
    return *getState().alpn();
  } else {
    return "";
  }
}

template <typename SM>
void AsyncFizzServerT<SM>::close() {
  if (transport_->good()) {
    fizzServer_.appCloseImmediate();
  } else {
    DelayedDestruction::DestructorGuard dg(this);
    folly::AsyncSocketException ase(
        folly::AsyncSocketException::END_OF_FILE, "socket closed locally");
    deliverAllErrors(ase, false);
    transport_->close();
  }
}

template <typename SM>
void AsyncFizzServerT<SM>::tlsShutdown() {
  if (transport_->good()) {
    // do not immediately close, wait to receive a close notify from the other
    // end
    fizzServer_.appClose();
  }
}

template <typename SM>
void AsyncFizzServerT<SM>::shutdownWrite() {
  DelayedDestruction::DestructorGuard dg(this);
  // Attempt to send a close_notify, then perform a half-shutdown of the write
  // side of the underlying socket. If previously submitted writes, and the
  // close_notify itself, are queued up in the Fizz layer, they won't make it
  // out before the socket shuts down.
  tlsShutdown();
  transport_->shutdownWrite();
}

template <typename SM>
void AsyncFizzServerT<SM>::shutdownWriteNow() {
  DelayedDestruction::DestructorGuard dg(this);
  // Similar to shutdownWrite(), attempt to write out the close_notify, but it
  // might not make it out to the transport layer before the socket is closed.
  tlsShutdown();
  transport_->shutdownWriteNow();
}

template <typename SM>
void AsyncFizzServerT<SM>::closeWithReset() {
  DelayedDestruction::DestructorGuard dg(this);
  if (transport_->good()) {
    fizzServer_.appCloseImmediate();
  }
  folly::AsyncSocketException ase(
      folly::AsyncSocketException::END_OF_FILE, "socket closed locally");
  deliverAllErrors(ase, false);
  transport_->closeWithReset();
}

template <typename SM>
void AsyncFizzServerT<SM>::closeNow() {
  DelayedDestruction::DestructorGuard dg(this);
  if (transport_->good()) {
    fizzServer_.appCloseImmediate();
  }
  folly::AsyncSocketException ase(
      folly::AsyncSocketException::END_OF_FILE, "socket closed locally");
  deliverAllErrors(ase, false);
  transport_->closeNow();
}

template <typename SM>
void AsyncFizzServerT<SM>::sendTicketWithAppToken(Buf appToken) {
  WriteNewSessionTicket nst;
  nst.appToken = std::move(appToken);
  fizzServer_.writeNewSessionTicket(std::move(nst));
}

template <typename SM>
folly::Optional<CipherSuite> AsyncFizzServerT<SM>::getCipher() const {
  return getState().cipher();
}

template <typename SM>
folly::Optional<NamedGroup> AsyncFizzServerT<SM>::getGroup() const {
  return getState().group();
}

template <typename SM>
std::vector<SignatureScheme> AsyncFizzServerT<SM>::getSupportedSigSchemes()
    const {
  return getState().context()->getSupportedSigSchemes();
}

template <typename SM>
Buf AsyncFizzServerT<SM>::getExportedKeyingMaterial(
    folly::StringPiece label,
    Buf context,
    uint16_t length) const {
  return fizzServer_.getExportedKeyingMaterial(
      *fizzContext_->getFactory(), label, std::move(context), length);
}

template <typename SM>
Buf AsyncFizzServerT<SM>::getEarlyEkm(
    folly::StringPiece label,
    const Buf& context,
    uint16_t length) const {
  return fizzServer_.getEarlyEkm(
      *fizzContext_->getFactory(), label, context, length);
}

template <typename SM>
void AsyncFizzServerT<SM>::writeAppData(
    folly::AsyncTransportWrapper::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf>&& buf,
    folly::WriteFlags flags) {
  DelayedDestruction::DestructorGuard dg(this);
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

  AppWrite write;
  write.callback = callback;
  write.data = std::move(buf);
  write.flags = flags;
  write.aeadOptions = writeAeadOptions_;
  auto size = write.data->computeChainDataLength();
  fizzServer_.appWrite(std::move(write));
  wroteApplicationBytes(size);
}

template <typename SM>
void AsyncFizzServerT<SM>::transportError(
    const folly::AsyncSocketException& ex) {
  DelayedDestruction::DestructorGuard dg(this);
  deliverAllErrors(ex);
}

template <typename SM>
void AsyncFizzServerT<SM>::transportDataAvailable() {
  fizzServer_.newTransportData();
}

template <typename SM>
void AsyncFizzServerT<SM>::pauseEvents() {
  fizzServer_.pause();
}

template <typename SM>
void AsyncFizzServerT<SM>::resumeEvents() {
  fizzServer_.resume();
}

template <typename SM>
void AsyncFizzServerT<SM>::deliverAllErrors(
    const folly::AsyncSocketException& ex,
    bool closeTransport) {
  deliverHandshakeError(ex);
  fizzServer_.moveToErrorState(ex);
  deliverError(ex, closeTransport);
}

template <typename SM>
void AsyncFizzServerT<SM>::deliverHandshakeError(folly::exception_wrapper ex) {
  if (handshakeCallback_) {
    auto callback = handshakeCallback_;
    handshakeCallback_ = nullptr;
    callback->fizzHandshakeError(this, std::move(ex));
  }
}

template <typename SM>
folly::Optional<Random> AsyncFizzServerT<SM>::getClientRandom() const {
  return getState().clientRandom();
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(DeliverAppData& data) {
  server_.deliverAppData(std::move(data.data));
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(WriteToSocket& data) {
  DCHECK(!data.contents.empty());
  Buf allData = std::move(data.contents.front().data);
  for (size_t i = 1; i < data.contents.size(); ++i) {
    allData->prependChain(std::move(data.contents[i].data));
  }
  server_.transport_->writeChain(data.callback, std::move(allData), data.flags);
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(
    ReportEarlyHandshakeSuccess&) {
  // Since the server can handle async events, it is possible for the
  // transport to become not good once we return from processing async events.
  // We want to error out the connection in this case.
  if (!server_.good()) {
    folly::AsyncSocketException ase(
        folly::AsyncSocketException::NOT_OPEN, "Transport is not good");
    server_.transportError(ase);
    return;
  }
  if (server_.handshakeCallback_) {
    auto callback = server_.handshakeCallback_;
    server_.handshakeCallback_ = nullptr;
    callback->fizzHandshakeSuccess(&server_);
  }
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(
    ReportHandshakeSuccess&) {
  // Since the server can handle async events, it is possible for the
  // transport to become not good once we return from processing async events.
  // We want to error out the connection in this case.
  if (!server_.good()) {
    folly::AsyncSocketException ase(
        folly::AsyncSocketException::NOT_OPEN, "Transport is not good");
    server_.transportError(ase);
    return;
  }

  // Disable record aligned reads. At this point, we are aligned on a record
  // boundary (if handshakeRecordAlignedReads = true).
  server_.updateReadHint(0);

  if (server_.handshakeCallback_) {
    auto callback = server_.handshakeCallback_;
    server_.handshakeCallback_ = nullptr;
    callback->fizzHandshakeSuccess(&server_);
  }
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(ReportError& error) {
  folly::AsyncSocketException ase(
      folly::AsyncSocketException::SSL_ERROR, error.error.what().toStdString());
  server_.deliverHandshakeError(std::move(error.error));
  server_.deliverAllErrors(ase);
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(WaitForData& wfd) {
  server_.fizzServer_.waitForData();
  server_.updateReadHint(wfd.recordSizeHint);

  if (server_.handshakeCallback_) {
    // Make sure that the read callback is installed.
    server_.startTransportReads();
  }
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(MutateState& mutator) {
  mutator(server_.state_);
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(
    AttemptVersionFallback& fallback) {
  if (!server_.handshakeCallback_) {
    VLOG(2) << "fizz fallback without callback";
    return;
  }
  auto callback = server_.handshakeCallback_;
  server_.handshakeCallback_ = nullptr;
  if (!server_.transportReadBuf_.empty()) {
    fallback.clientHello->prependChain(server_.transportReadBuf_.move());
  }
  callback->fizzHandshakeAttemptFallback(AttemptVersionFallback{
      std::move(fallback.clientHello), std::move(fallback.sni)});
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(
    SecretAvailable& secret) {
  fizz_probe_secret_available(
      secret.secret.secret.size(),
      secret.secret.secret.data(),
      KeyLogWriter::secretToNSSLabel(secret.secret.type)
          .value_or(std::numeric_limits<KeyLogWriter::Label>::max()),
      server_.getClientRandom()->data());

  server_.secretAvailable(secret.secret);
}

template <typename SM>
void AsyncFizzServerT<SM>::ActionMoveVisitor::operator()(EndOfData& eod) {
  server_.endOfTLS(std::move(eod.postTlsData));
}

template <typename SM>
void AsyncFizzServerT<SM>::initiateKeyUpdate(
    KeyUpdateRequest keyUpdateRequest) {
  KeyUpdateInitiation kui;
  kui.request_update = keyUpdateRequest;
  fizzServer_.initiateKeyUpdate(std::move(kui));
}
} // namespace server
} // namespace fizz
