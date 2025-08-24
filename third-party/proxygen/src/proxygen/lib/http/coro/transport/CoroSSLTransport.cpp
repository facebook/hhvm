/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/CoroSSLTransport.h"
#include <folly/logging/xlog.h>

#include <folly/io/Cursor.h>
#include <folly/io/async/ssl/BasicTransportCertificate.h>
#include <folly/io/async/ssl/SSLErrors.h>

namespace {

using folly::ssl::OpenSSLUtils;
using proxygen::coro::CoroSSLTransport;
using proxygen::coro::TimedBaton;

constexpr size_t kMinSSLWriteSize = 1500;
static const uint32_t kMinReadSize = 1460;
static const uint32_t kMaxReadSize = 4000;
static const uint32_t kMaxBufferSize = 128 * 1024;
constexpr int CORO_SSL_TRANSPORT_RETRY = -1;

CoroSSLTransport* transportFromBio(BIO* bio) {
  auto appData = OpenSSLUtils::getBioAppData(bio);
  XCHECK(appData);
  CoroSSLTransport* transport = reinterpret_cast<CoroSSLTransport*>(appData);
  XCHECK(transport);
  return transport;
}

int coroSSLTransportBioWrite(BIO* bio, const char* buf, int sz) {
  BIO_clear_retry_flags(bio);
  return transportFromBio(bio)->bioWrite(buf, sz);
}
int coroSSLTransportBioWriteEx(BIO* bio,
                               const char* buf,
                               size_t sz,
                               size_t* nw) {
  BIO_clear_retry_flags(bio);
  auto rc = coroSSLTransportBioWrite(bio, buf, sz);
  if (rc >= 0) {
    *nw = rc;
    return 1;
  } else if (rc == CORO_SSL_TRANSPORT_RETRY) {
    BIO_set_retry_write(bio);
  }
  return 0;
}
int coroSSLTransportBioRead(BIO* bio, char* buf, int sz) {
  BIO_clear_retry_flags(bio);
  auto rc = transportFromBio(bio)->bioRead(buf, sz);
  if (rc == CORO_SSL_TRANSPORT_RETRY) {
    BIO_set_retry_read(bio);
  }
  return rc;
}
int coroSSLTransportBioReadEx(BIO* bio, char* buf, size_t sz, size_t* nr) {
  auto rc = coroSSLTransportBioRead(bio, buf, sz);
  if (rc >= 0) {
    *nr = rc;
    return 1;
  }
  return 0;
}
long coroSSLTransportBioCtrl(BIO* bio, int i, long l, void* p) {
  return transportFromBio(bio)->bioCtrl(i, l, p);
}
int coroSSLTransportBioCreate(BIO* bio) {
  BIO_set_init(bio, 1);
  return 1;
}
int coroSSLTransportBioDestroy(BIO* /*bio*/) {
  return 1;
}
long coroSSLTransportBioCallbackCtrl(BIO* bio, int idx, BIO_info_cb* cb) {
  return transportFromBio(bio)->bioCallbackCtrl(idx, cb);
}

folly::ssl::BioMethodUniquePtr initBioMethod() {
  BIO_METHOD* newmeth = nullptr;
  newmeth = BIO_meth_new(BIO_get_new_index(), "coro_ssl_transport_bio_method");
  if (!newmeth) {
    return nullptr;
  }
  BIO_meth_set_create(newmeth, coroSSLTransportBioCreate);
  BIO_meth_set_destroy(newmeth, coroSSLTransportBioDestroy);
  BIO_meth_set_ctrl(newmeth, coroSSLTransportBioCtrl);
  BIO_meth_set_callback_ctrl(newmeth, coroSSLTransportBioCallbackCtrl);
  BIO_meth_set_read(newmeth, coroSSLTransportBioRead);
  BIO_meth_set_read_ex(newmeth, coroSSLTransportBioReadEx);
  BIO_meth_set_write(newmeth, coroSSLTransportBioWrite);
  BIO_meth_set_write_ex(newmeth, coroSSLTransportBioWriteEx);

  return folly::ssl::BioMethodUniquePtr(newmeth);
}
// Note: This is a Leaky Meyer's Singleton. The reason we can't use a non-leaky
// thing is because we will be setting this BIO_METHOD* inside BIOs owned by
// various SSL objects which may get callbacks even during teardown. We may
// eventually try to fix this
BIO_METHOD* getCoroSSLBioMethod() {
  static auto const instance = initBioMethod().release();
  return instance;
}

std::optional<std::chrono::steady_clock::time_point> deadlineFromTimeout(
    std::chrono::milliseconds timeout) {
  if (timeout.count() > 0) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    return deadline;
  }
  return std::nullopt;
}

std::chrono::milliseconds timeoutFromDeadline(
    std::optional<std::chrono::steady_clock::time_point> deadline) {
  std::chrono::milliseconds timeout{0};
  if (deadline) {
    auto now = std::chrono::steady_clock::now();
    if (now > *deadline) {
      throw folly::AsyncSocketException(folly::AsyncSocketException::TIMED_OUT,
                                        "SSL opereation timed out");
    }
    timeout =
        std::chrono::duration_cast<std::chrono::milliseconds>(*deadline - now);
  }
  return timeout;
}

// This converts "illegal" shutdowns into ZERO_RETURN
inline bool zero_return(int error, int rc, int errno_copy) {
  if (error == SSL_ERROR_ZERO_RETURN) {
    // Peer has closed the connection for writing by sending the
    // close_notify alert. The underlying transport might not be closed, but
    // assume it is and return EOF.
    return true;
  }
#ifdef _WIN32
  // on windows underlying TCP socket may error with this code
  // if the sending/receiving client crashes or is killed
  if (error == SSL_ERROR_SYSCALL && errno_copy == WSAECONNRESET) {
    return true;
  }
#endif
  // NOTE: OpenSSL has a bug where SSL_ERROR_SYSCALL and errno 0 indicates
  // an unexpected EOF from the peer. This will be changed in OpenSSL 3.0
  // and reported as SSL_ERROR_SSL with reason
  // SSL_R_UNEXPECTED_EOF_WHILE_READING. We should then explicitly check for
  // that. See https://www.openssl.org/docs/man1.1.1/man3/SSL_get_error.html
  if (rc == SSL_ERROR_SYSCALL && errno_copy == 0) {
    // ignore anything else in the error queue
    ERR_clear_error();
    return true;
  }
  return false;
}

int getCoroSSLTransportExDataIndex() {
  static auto index = SSL_get_ex_new_index(
      0, (void*)"CoroSSLTransport data index", nullptr, nullptr, nullptr);
  return index;
}

CoroSSLTransport* getCoroSSLTransportFromSSL(const SSL* ssl) {
  return static_cast<CoroSSLTransport*>(
      SSL_get_ex_data(ssl, getCoroSSLTransportExDataIndex()));
}

folly::coro::Task<CoroSSLTransport::IOResult> waitForIO(
    TimedBaton& baton,
    std::optional<std::chrono::steady_clock::time_point>& currentDeadline,
    std::optional<std::chrono::steady_clock::time_point> deadline) {
  if (!currentDeadline || *currentDeadline > deadline) {
    // Only set the timeout if the deadline is sooner than the current deadline
    currentDeadline = deadline;
    baton.setTimeout(timeoutFromDeadline(deadline));
  }
  auto res = co_await baton.wait();
  currentDeadline.reset();
  if (res == TimedBaton::Status::cancelled) {
    // This can happen with outstanding writes in closeWithReset
    XLOG(DBG6) << "IO wait cancelled";
    co_yield folly::coro::co_error(folly::AsyncSocketException(
        folly::AsyncSocketException::CANCELED, "IO wait cancelled"));
  } else if (res == TimedBaton::Status::timedout) {
    co_yield folly::coro::co_error(folly::AsyncSocketException(
        folly::AsyncSocketException::TIMED_OUT, "SSL opereation timed out"));
  } else {
    co_return CoroSSLTransport::IOResult::Success;
  }
}

} // namespace

using namespace folly::ssl;
using folly::AsyncSocketException;

namespace proxygen::coro {
bool CoroSSLTransport::setupSSLBio() {
  auto sslBio = BIO_new(getCoroSSLBioMethod());

  if (!sslBio) {
    return false;
  }

  OpenSSLUtils::setBioAppData(sslBio, this);
  SSL_set_bio(ssl_.get(), sslBio, sslBio);
  return true;
}

CoroSSLTransport::CoroSSLTransport(
    std::unique_ptr<folly::coro::TransportIf> transport,
    std::shared_ptr<const folly::SSLContext> sslContext,
    // const std::shared_ptr<ClientExtensions>& extensions = nullptr,
    TransportOptions transportOptions)
    : transport_(std::move(transport)),
      localAddr_(transport_->getLocalAddress()),
      peerAddr_(transport_->getPeerAddress()),
      transportOptions_(std::move(transportOptions)),
      ctx_(std::move(sslContext)),
      readsBlocked_(transport_->getEventBase(), std::chrono::milliseconds(0)),
      writesBlocked_(transport_->getEventBase(), std::chrono::milliseconds(0)) {
  readsBlocked_.signal();
  writesBlocked_.signal();
}

CoroSSLTransport::~CoroSSLTransport() {
  closeNow();
  *deleted_ = true;
}

folly::coro::Task<void> CoroSSLTransport::connect(
    // std::shared_ptr<const CertificateVerifier> verifier,
    folly::Optional<std::string> sni,
    // Optional<std::string> pskIdentity,
    // Optional<std::vector<ech::ECHConfig>> echConfigs,
    std::chrono::milliseconds timeout) {
  getEventBase()->dcheckIsInEventBaseThread();

  try {
    ssl_.reset(ctx_->createSSL());
  } catch (std::exception&) {
    static const folly::Indestructible<AsyncSocketException> ex(
        AsyncSocketException::INTERNAL_ERROR,
        "error calling SSLContext::createSSL()");
    XLOG(ERR) << "CoroSSLTransport::connect(this=" << this
              << "): " << ex->what();
    throw(*ex);
  }

  if (!setupSSLBio()) {
    static const folly::Indestructible<AsyncSocketException> ex(
        AsyncSocketException::INTERNAL_ERROR, "error creating SSL bio");
    XLOG(ERR) << "CoroSSLTransport::connect(this=" << this
              << "): " << ex->what();
    co_yield folly::coro::co_error(*ex);
  }

  if (!applyVerificationOptions(ssl_)) {
    static const folly::Indestructible<AsyncSocketException> ex(
        AsyncSocketException::INTERNAL_ERROR,
        "error applying the SSL verification options");
    XLOG(ERR) << "CoroSSLTransport::connect(this=" << this
              << "): " << ex->what();
    co_yield folly::coro::co_error(*ex);
  }

  SSLSessionUniquePtr sessionPtr = sslSessionManager_.getRawSession();
  if (sessionPtr) {
    SSL_set_session(ssl_.get(), sessionPtr.get());
  }
  if (sni && !sni->empty()) {
    sni_ = *sni;
    SSL_set_tlsext_host_name(ssl_.get(), sni_.c_str());
  }

  SSL_set_ex_data(ssl_.get(), getCoroSSLTransportExDataIndex(), this);
  sslSessionManager_.attachToSSL(ssl_.get());

  co_return co_await doConnect(timeout);
}

folly::coro::Task<void> CoroSSLTransport::doConnect(
    std::chrono::milliseconds timeout) {
  co_await folly::coro::co_safe_point;
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancellationCallback{
      cancelToken, [&] { cancellationSource_.requestCancellation(); }};
  auto handshakeDeadline = deadlineFromTimeout(timeout);
  while (true) {
    int ret;
    {
      // If openssl is not built with TSAN then we can get a TSAN false positive
      // when calling SSL_connect from multiple threads.
      folly::annotate_ignore_thread_sanitizer_guard g(__FILE__, __LINE__);
      ret = SSL_connect(ssl_.get());
    }
    XLOG(DBG6) << "SSL_connect returned=" << ret;
    if (ret <= 0) {
      auto res = co_await folly::coro::co_withCancellation(
          cancellationSource_.getToken(),
          handleReturnMaybeIO(ret, handshakeDeadline));
      if (res == IOResult::EndOfFile) {
        co_yield folly::coro::co_error(folly::AsyncSocketException(
            AsyncSocketException::END_OF_FILE, "EOF during handshake"));
      }
    } else {
      XLOG(DBG3) << "CoroSSLTransport " << this << ": successfully connected";
      co_return;
    }
  }
}

bool CoroSSLTransport::applyVerificationOptions(
    const folly::ssl::SSLUniquePtr& ssl) {
  // apply the settings specified in verifyPeer_
  if (verifyPeer_ == folly::SSLContext::SSLVerifyPeerEnum::USE_CTX) {
    XLOG_IF(WARNING, transportOptions_.verifier) << "Verifier set but ignored";
    if (ctx_->needsPeerVerification()) {
      if (ctx_->checkPeerName()) {
        std::string peerNameToVerify =
            !ctx_->peerFixedName().empty() ? ctx_->peerFixedName() : sni_;

        X509_VERIFY_PARAM* param = SSL_get0_param(ssl.get());
        if (!X509_VERIFY_PARAM_set1_host(
                param, peerNameToVerify.c_str(), peerNameToVerify.length())) {
          return false;
        }
      }

      SSL_set_verify(ssl.get(),
                     ctx_->getVerificationMode(),
                     CoroSSLTransport::sslVerifyCallback);
    }
  } else {
    if (verifyPeer_ == folly::SSLContext::SSLVerifyPeerEnum::VERIFY ||
        verifyPeer_ ==
            folly::SSLContext::SSLVerifyPeerEnum::VERIFY_REQ_CLIENT_CERT) {
      SSL_set_verify(ssl.get(),
                     folly::SSLContext::getVerificationMode(verifyPeer_),
                     CoroSSLTransport::sslVerifyCallback);
    }
  }

  return true;
}

bool CoroSSLTransport::getSSLSessionReused() const {
  if (ssl_ != nullptr) {
    return SSL_session_reused(ssl_.get());
  }
  return false;
}

std::string CoroSSLTransport::getApplicationProtocol() const {
  const unsigned char* protoName = nullptr;
  unsigned protoLength = 0;
  SSL_get0_alpn_selected(ssl_.get(), &protoName, &protoLength);
  return std::string(reinterpret_cast<const char*>(protoName), protoLength);
}

const char* CoroSSLTransport::getNegotiatedCipherName() const {
  return (ssl_ != nullptr) ? SSL_get_cipher_name(ssl_.get()) : nullptr;
}

int CoroSSLTransport::getSSLVersion() const {
  return (ssl_ != nullptr) ? SSL_version(ssl_.get()) : 0;
}

int CoroSSLTransport::sslVerifyCallback(int preverifyOk,
                                        X509_STORE_CTX* x509Ctx) {
  SSL* ssl = (SSL*)X509_STORE_CTX_get_ex_data(
      x509Ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
  auto* self = getCoroSSLTransportFromSSL(ssl);

  XLOG(DBG3) << "CoroSSLTransport::sslVerifyCallback() this=" << self << ", "
             << ", preverifyOk=" << preverifyOk;

  if (!preverifyOk) {
    // OpenSSL verification failure, no need to call CertificateIdentityVerifier
    return 0;
  }

  // only invoke the CertificateIdentityVerifier for the leaf certificate and
  // only if OpenSSL's preverify succeeded

  int currentDepth = X509_STORE_CTX_get_error_depth(x509Ctx);
  if (currentDepth != 0 || !self->transportOptions_.verifier) {
    return 1;
  }

  X509* peerX509 = X509_STORE_CTX_get_current_cert(x509Ctx);
  X509_up_ref(peerX509);
  folly::ssl::X509UniquePtr peer{peerX509};
  auto cn = OpenSSLUtils::getCommonName(peerX509);
  auto cert = std::make_unique<BasicTransportCertificate>(std::move(cn),
                                                          std::move(peer));

  try {
    auto verifiedCert =
        self->transportOptions_.verifier->verifyLeaf(*cert.get());
    self->peerCertData_ = std::move(verifiedCert);
  } catch (folly::CertificateIdentityVerifierException& e) {

    XLOG(ERR) << "CoroSSLTransport::sslVerifyCallback(this=" << self
              << ") Failed to verify leaf certificate identity(ies): " << e;
    return 0;
  }

  return 1;
}

folly::coro::Task<size_t> CoroSSLTransport::read(
    folly::MutableByteRange buf, std::chrono::milliseconds timeout) {
  SCOPE_EXIT {
    XLOG(DBG6) << "read complete";
  };
  if (buf.size() == 0) {
    co_return 0;
  }
  co_await folly::coro::co_safe_point;
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancellationCallback{
      cancelToken, [&] { cancellationSource_.requestCancellation(); }};
  auto readDeadline = deadlineFromTimeout(timeout);
  while (true) {
    auto ret = SSL_read(ssl_.get(), buf.data(), buf.size());
    XLOG(DBG6) << "SSL_read returned=" << ret;
    if (ret <= 0) {
      auto res = co_await folly::coro::co_withCancellation(
          cancellationSource_.getToken(),
          handleReturnMaybeIO(ret, readDeadline));
      if (res == IOResult::EndOfFile) {
        co_return 0;
      }
    } else {
      co_return ret;
    }
  }
}

folly::coro::Task<size_t> CoroSSLTransport::read(
    folly::IOBufQueue& buf,
    size_t minReadSize,
    size_t newAllocationSize,
    std::chrono::milliseconds timeout) {
  // this flavor or read should have a max read size!
  auto rbuf = buf.preallocate(minReadSize, newAllocationSize);
  auto rc =
      co_await folly::coro::TransportIf::read(rbuf.first, rbuf.second, timeout);
  buf.postallocate(rc);
  co_return rc;
}

folly::coro::Task<CoroSSLTransport::IOResult>
CoroSSLTransport::handleReturnMaybeIO(
    int ret, std::optional<std::chrono::steady_clock::time_point> deadline) {
  XLOG(DBG6) << "handleReturnMaybeIO";
  int sslError;
  unsigned long errError;
  int errnoCopy = errno;
  if (willBlock(ret, &sslError, &errError)) {
    if (sslError == SSL_ERROR_WANT_READ) {
      return transportRead(deadline);
    } else {
      return waitForIO(writesBlocked_, writeDeadline_, deadline);
    }
  } else {
    if (zero_return(sslError, ret, errnoCopy)) {
      return []() -> folly::coro::Task<IOResult> {
        co_return IOResult::EndOfFile;
      }();
    }
    ERR_clear_error();
    throw folly::SSLException(sslError, errError, ret, errnoCopy);
  }
}

bool CoroSSLTransport::willBlock(int ret,
                                 int* sslErrorOut,
                                 unsigned long* errErrorOut) noexcept {
  XLOG(DBG6) << "willBlock";
  *errErrorOut = 0;
  int error = *sslErrorOut = SSL_get_error(ssl_.get(), ret);
  if (error == SSL_ERROR_WANT_READ) {
    XLOG(DBG6) << "CoroSSLTransport(" << this << "): SSL_ERROR_WANT_READ";
    return true;
  }
  if (error == SSL_ERROR_WANT_WRITE) {
    XLOG(DBG6) << "CoroSSLTransport(" << this << "): SSL_ERROR_WANT_WRITE";
    return true;
  }

  // No support yet for SSL_ERROR_WANT_ASYNC
  unsigned long lastError = *errErrorOut = ERR_get_error();
  XLOG(DBG6) << "CoroSSLTransport(" << this << "): SSL error: " << error << ", "
             << "errno: " << errno << ", " << "ret: " << ret << ", "
             << "read: " << BIO_number_read(SSL_get_rbio(ssl_.get())) << ", "
             << "written: " << BIO_number_written(SSL_get_wbio(ssl_.get()))
             << ", " << "func: " << ERR_func_error_string(lastError) << ", "
             << "reason: " << ERR_reason_error_string(lastError);
  return false;
}

folly::coro::Task<CoroSSLTransport::IOResult> CoroSSLTransport::transportRead(
    std::optional<std::chrono::steady_clock::time_point> deadline) {
  // Only one transport_->read call at a time
  if (readsBlocked_.getStatus() == TimedBaton::Status::notReady) {
    co_return co_await waitForIO(readsBlocked_, readDeadline_, deadline);
  } else {
    // this flavor or read should have a max read size - it's implicitly
    // AsyncSocket maxReadsPerEvent_ * kMaxReadSize
    readsBlocked_.reset();
    auto rc = co_await transport_->read(transportReadBuf_,
                                        kMinReadSize,
                                        kMaxReadSize,
                                        timeoutFromDeadline(deadline));
    readsBlocked_.signal();
    co_return rc == 0 ? IOResult::EndOfFile : IOResult::Success;
  }
}

int CoroSSLTransport::bioRead(char* buf, size_t sz) {
  if (transportReadBuf_.empty()) {
    return CORO_SSL_TRANSPORT_RETRY;
  }
  folly::io::Cursor cursor(transportReadBuf_.front());
  auto nRead = cursor.pullAtMost(buf, sz);
  XLOG(DBG6) << "transportReadBuf_ size=" << transportReadBuf_.chainLength()
             << " returning nRead=" << nRead;
  transportReadBuf_.trimStart(nRead);
  return nRead;
}

folly::coro::Task<folly::Unit> CoroSSLTransport::write(
    folly::ByteRange buf,
    std::chrono::milliseconds timeout,
    folly::WriteFlags writeFlags,
    WriteInfo* writeInfo) {
  if (pendingShutdown_) {
    throw folly::AsyncSocketException(AsyncSocketException::END_OF_FILE,
                                      "write after shutdownWrite");
  }
  return writeImpl(buf, timeout, writeFlags, writeInfo, /*writev=*/false);
}

folly::coro::Task<folly::Unit> CoroSSLTransport::writeImpl(
    folly::ByteRange buf,
    std::chrono::milliseconds timeout,
    folly::WriteFlags /*writeFlags*/,
    WriteInfo* writeInfo,
    bool writev) {
  if (!writev) {
    XCHECK_EQ(writers_, 0UL) << "One write at a time please";
  }
  co_await folly::coro::co_safe_point;
  auto deadline = deadlineFromTimeout(timeout);
  {
    writers_++;
    SCOPE_EXIT {
      writers_--;
    };
    while (true) {
      // bioWrite gets the timeout from here
      writesBlocked_.setTimeout(timeoutFromDeadline(deadline));
      auto rc = SSL_write(ssl_.get(), buf.data(), buf.size());
      XLOG(DBG6) << "SSL_write returned=" << rc;
      if (rc <= 0) {
        auto res = co_await handleReturnMaybeIO(rc, deadline);
        if (res == IOResult::EndOfFile) {
          co_yield folly::coro::co_error(folly::AsyncSocketException(
              AsyncSocketException::END_OF_FILE, "EOF during write"));
        }
      } else {
        XCHECK_EQ(static_cast<size_t>(rc), buf.size());
        break;
      }
    }
  }
  if (pendingShutdown_) {
    shutdownWrite();
  }
  if (writeInfo) {
    writeInfo->bytesWritten = buf.size();
  }
  co_return folly::Unit();
}

folly::coro::Task<folly::Unit> CoroSSLTransport::write(
    folly::IOBufQueue& data,
    std::chrono::milliseconds timeout,
    folly::WriteFlags writeFlags,
    WriteInfo* writeInfo) {
  if (pendingShutdown_) {
    co_yield folly::coro::co_error(folly::AsyncSocketException(
        AsyncSocketException::END_OF_FILE, "write after shutdownWrite"));
  }
  XCHECK_EQ(writers_, 0UL) << "One write at a time please";
  auto deadline = deadlineFromTimeout(timeout);
  do {
    // TODO: If data is smaller than kMinSSLWriteSize, we could skip the extra
    // coro overhead
    data.gather(std::min(kMinSSLWriteSize, data.chainLength()));
    auto pendingWrite = data.pop_front();
    WriteInfo singleWriteInfo;
    {
      writers_++;
      SCOPE_EXIT {
        writers_--;
      };
      co_await writeImpl(
          folly::ByteRange(pendingWrite->data(), pendingWrite->length()),
          timeoutFromDeadline(deadline),
          writeFlags,
          &singleWriteInfo,
          /*writev=*/true);
    }
    if (pendingShutdown_) {
      shutdownWrite();
    }
    if (writeInfo) {
      writeInfo->bytesWritten += singleWriteInfo.bytesWritten;
    }
  } while (!data.empty());

  co_return folly::Unit();
}

int CoroSSLTransport::bioWrite(const char* buf, size_t sz) {
  // We have to startInlineUnsafe because transport_->write may not execute
  // before deletion, and it will use-after-free.
  // We could cancel it, but in particular SSL_shutdown generates a bioWrite
  // (close_notify) and the dtor calls close and cancels the coros.
  // Maybe change Transport::write to do the setup inline
  int ret = sz;
  try {
    XLOG(DBG6) << "transport_->write sz=" << sz;
    if (transportBytesOutstanding_ > kMaxBufferSize) {
      if (writesBlocked_.getStatus() != TimedBaton::Status::notReady) {
        XLOG(DBG6) << "Blocking writes, transportBytesOutstanding_="
                   << transportBytesOutstanding_;
        writesBlocked_.reset();
      }
      return CORO_SSL_TRANSPORT_RETRY;
    }
    transportBytesOutstanding_ += sz;
    // TODO: writeFlags
    co_withExecutor(
        getEventBase(),
        transport_->write(
            folly::ByteRange{reinterpret_cast<const uint8_t*>(buf), sz},
            writesBlocked_.getTimeout()))
        .startInlineUnsafe()
        .via(getEventBase())
        .thenTry([this, sz, deleted = deleted_](
                     folly::Try<folly::Unit> result) {
          XLOG(DBG6) << "Write completed sz=" << sz;
          if (!*deleted) {
            XCHECK_GE(transportBytesOutstanding_, sz);
            transportBytesOutstanding_ -= sz;
            XLOG(DBG6) << "transportBytesOutstanding_="
                       << transportBytesOutstanding_;
            if (result.hasException()) {
              XLOG(ERR) << "Write error ex=" << result.exception().what();
              transport_->closeWithReset();
              return;
            }
            if (transportBytesOutstanding_ <= kMaxBufferSize &&
                writesBlocked_.getStatus() == TimedBaton::Status::notReady) {
              XLOG(DBG6) << "Resuming writes";
              writesBlocked_.signal();
            }
            if (pendingShutdown_) {
              shutdownWrite();
            }
          }
        });
  } catch (const std::exception& ex) {
    // This is a catch-all so we don't jump past OpenSSL code
    XLOG(ERR) << ex.what();
    return -1;
  }
  return ret;
}

// Close both directions of the transport, delayed shutdown OK - allow writes
// to drain
void CoroSSLTransport::close() {
  shutdownWrite();
  if (!shutdownRead()) {
    if (pendingShutdown_) {
      // will call transport_->close() from deferred shutdownWrite if needed
      pendingClose_ = true;
    } else {
      // close the transport now
      transport_->close();
    }
  }
  // otherwise shutdownWrite already called close
}

// Close immediately.  Fail any unfinished writes, may not write close notify.
void CoroSSLTransport::closeNow() {
  failWrites();
  shutdownWrite();
  if (!shutdownRead() || pendingShutdown_) {
    // We're not done shutting down, just close the transport
    transport_->close();
  }
  // otherwise shutdownWrite already called close
}

void CoroSSLTransport::failWrites() {
  // interrupt any write coros
  writesBlocked_.signal(TimedBaton::Status::cancelled);
}

void CoroSSLTransport::shutdownWrite() {
  if (ssl_ && (SSL_get_shutdown(ssl_.get()) & SSL_SENT_SHUTDOWN) == 0) {
    if (writers_ > 0 || transportBytesOutstanding_ > 0) {
      // Can't call SSL_shutdown until SSL_write calls finish and flush
      if (!pendingShutdown_) {
        XLOG(DBG6) << "Delayed shutdown with pending writes";
        pendingShutdown_ = true;
      }
      return;
    }
    pendingShutdown_ = false;
    XLOG(DBG6) << "SSL_shutdown";
    int rc = SSL_shutdown(ssl_.get());
    if (rc == 0) {
      // peer's notify has not yet been received
      XLOG(DBG4) << "SSL writes shutdown";
      if (pendingClose_) {
        transport_->close();
      } else {
        transport_->shutdownWrite();
      }
    } else if (rc == 1) {
      // fully shutdown.  There shouldn't be a read in progress (would have
      // returned 0 already), but call shutdownRead in case.
      XLOG(DBG4) << "SSL completely shutdown";
      shutdownRead();
      transport_->close();
    } else {
      int sslError;
      unsigned long errError;
      // Logs the error
      (void)willBlock(rc, &sslError, &errError);
      ERR_clear_error();
      transport_->closeWithReset();
    }
  }
}

void CoroSSLTransport::closeWithReset() {
  // Don't call shutdownWrite, which begins graceful SSL close
  failWrites();
  shutdownRead();
  transport_->closeWithReset();
}

bool CoroSSLTransport::shutdownRead() {
  // interrupt any read coros
  cancellationSource_.requestCancellation();
  if (ssl_ && (SSL_get_shutdown(ssl_.get()) & SSL_RECEIVED_SHUTDOWN) == 0) {
    XLOG(DBG6) << "Shutting down reads but CLOSE_NOTIFY not received";
    return false;
  }
  return true;
}

const folly::AsyncTransportCertificate* CoroSSLTransport::getPeerCertificate()
    const {
  if (peerCertData_) {
    return peerCertData_.get();
  }
  if (ssl_ != nullptr) {
    auto peerX509 = SSL_get_peer_certificate(ssl_.get());
    if (peerX509) {
      // already up ref'd
      folly::ssl::X509UniquePtr peer(peerX509);
      auto cn = OpenSSLUtils::getCommonName(peerX509);
      peerCertData_ = std::make_unique<BasicTransportCertificate>(
          std::move(cn), std::move(peer));
    }
  }
  return peerCertData_.get();
}

/* BIO callbacks */
long CoroSSLTransport::bioCtrl(int /*i*/, long /*l*/, void* /*p*/) {
  return 1; //?
}
int CoroSSLTransport::bioCallbackCtrl(int /*idx*/, BIO_info_cb* /*cb*/) {
  return 1; //?
}

} // namespace proxygen::coro
