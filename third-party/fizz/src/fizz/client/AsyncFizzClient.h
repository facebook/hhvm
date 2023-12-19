/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/ClientExtensions.h>
#include <fizz/client/ClientProtocol.h>
#include <fizz/client/EarlyDataRejectionPolicy.h>
#include <fizz/client/FizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/protocol/AsyncFizzBase.h>
#include <fizz/protocol/Exporter.h>
#include <fizz/util/Tracing.h>
#include <folly/io/SocketOptionMap.h>

namespace fizz {
namespace client {

template <typename SM>
class AsyncFizzClientT : public AsyncFizzBase,
                         private folly::AsyncSocket::ConnectCallback {
 public:
  class HandshakeCallback {
   public:
    virtual ~HandshakeCallback() = default;

    virtual void fizzHandshakeSuccess(AsyncFizzClientT* transport) noexcept = 0;

    virtual void fizzHandshakeError(
        AsyncFizzClientT* transport,
        folly::exception_wrapper ex) noexcept = 0;
  };

  using UniquePtr =
      std::unique_ptr<AsyncFizzClientT, folly::DelayedDestruction::Destructor>;

  /**
   * Creates an AsyncFizzClient using an open socket. Connections are made using
   * connect() APIs taking a HandshakeCallback.
   **/
  AsyncFizzClientT(
      folly::AsyncTransportWrapper::UniquePtr socket,
      std::shared_ptr<const FizzClientContext> fizzContext,
      const std::shared_ptr<ClientExtensions>& extensions = nullptr,
      AsyncFizzBase::TransportOptions transportOptions =
          AsyncFizzBase::TransportOptions());

  /**
   * Creates an AsyncFizzClient using an event base. This will open the socket
   *for you when you call the connec() API taking a SocketAddress and
   *ConnectCallback.
   **/
  AsyncFizzClientT(
      folly::EventBase* eventBase,
      std::shared_ptr<const FizzClientContext> fizzContext,
      const std::shared_ptr<ClientExtensions>& extensions = nullptr,
      AsyncFizzBase::TransportOptions transportOptions =
          AsyncFizzBase::TransportOptions());

  /**
   * Performs a TLS handshake using the open socket passed into the constructor.
   **/
  virtual void connect(
      HandshakeCallback* callback,
      std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> sni,
      folly::Optional<std::string> pskIdentity,
      folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
      std::chrono::milliseconds = std::chrono::milliseconds(0));

  /**
   * Opens a socket to the given address and performs a TLS handshake.
   **/
  virtual void connect(
      const folly::SocketAddress& connectAddr,
      folly::AsyncSocket::ConnectCallback* callback,
      std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> sni,
      folly::Optional<std::string> pskIdentity,
      std::chrono::milliseconds totalTimeout = std::chrono::milliseconds(0),
      std::chrono::milliseconds socketTimeout = std::chrono::milliseconds(0),
      const folly::SocketOptionMap& options = folly::emptySocketOptionMap,
      const folly::SocketAddress& bindAddr = folly::AsyncSocket::anyAddress());

  /**
   * Variant of the TLS handshake connect() API above that uses the default
   *certificate verifier implementation.
   **/
  virtual void connect(
      HandshakeCallback* callback,
      folly::Optional<std::string> hostname,
      folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
      std::chrono::milliseconds = std::chrono::milliseconds(0));

  bool good() const override;
  bool readable() const override;
  bool connecting() const override;
  bool error() const override;

  const Cert* getPeerCertificate() const override;
  const Cert* getSelfCertificate() const override;

  bool isReplaySafe() const override;
  void setReplaySafetyCallback(
      folly::AsyncTransport::ReplaySafetyCallback* callback) override;
  std::string getApplicationProtocol() const noexcept override;

  void tlsShutdown() override;
  void shutdownWrite() override;
  void shutdownWriteNow() override;
  void close() override;
  void closeWithReset() override;
  void closeNow() override;

  /**
   * Set the policy for dealing with rejected early data.
   *
   * Note that early data must be also be enabled on the FizzClientContext for
   * early data to be used.
   */
  void setEarlyDataRejectionPolicy(EarlyDataRejectionPolicy policy) {
    CHECK(!earlyDataState_);
    earlyDataRejectionPolicy_ = policy;
  }

  /**
   * Internal state access for logging/testing.
   */
  const State& getState() const {
    return state_;
  }

  folly::Optional<CipherSuite> getCipher() const override;

  folly::Optional<NamedGroup> getGroup() const override;

  std::vector<SignatureScheme> getSupportedSigSchemes() const override;

  Buf getExportedKeyingMaterial(
      folly::StringPiece label,
      Buf context,
      uint16_t length) const override;

  Buf getEarlyEkm(folly::StringPiece label, const Buf& context, uint16_t length)
      const;

  bool pskResumed() const;

  folly::Optional<Random> getClientRandom() const override;

  folly::Optional<std::string> getPskIdentity() const {
    return pskIdentity_;
  }

  void initiateKeyUpdate(KeyUpdateRequest keyUpdateRequest) override;
  /**
   * Queries the state of ECH (whether it was requested and whether it was
   * accepted).
   */
  bool echRequested() const;

  bool echAccepted() const;

  /**
   * ECH configs supported by server. May be sent if ECH is requested,
   * useful in the rejection case.
   */
  folly::Optional<std::vector<ech::ECHConfig>> getEchRetryConfigs() const;

 protected:
  ~AsyncFizzClientT() override = default;
  void writeAppData(
      folly::AsyncTransportWrapper::WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override;

  void transportError(const folly::AsyncSocketException& ex) override;

  void transportDataAvailable() override;

  void pauseEvents() override;

  void resumeEvents() override;

 private:
  void deliverAllErrors(
      const folly::AsyncSocketException& ex,
      bool closeTransport = true);
  void deliverHandshakeError(folly::exception_wrapper ex);

  void connectErr(const folly::AsyncSocketException& ex) noexcept override;
  void connectSuccess() noexcept override;

  folly::Optional<folly::AsyncSocketException> handleEarlyReject();

  // Helper function to perform appWrite and check for key update.
  void performAppWrite(AppWrite w);

  class ActionMoveVisitor {
   public:
    explicit ActionMoveVisitor(AsyncFizzClientT<SM>& client)
        : client_(client) {}

    void operator()(DeliverAppData&);
    void operator()(WriteToSocket&);
    void operator()(ReportEarlyHandshakeSuccess&);
    void operator()(ReportHandshakeSuccess&);
    void operator()(ReportEarlyWriteFailed&);
    void operator()(ReportError&);
    void operator()(WaitForData&);
    void operator()(MutateState&);
    void operator()(NewCachedPsk&);
    void operator()(SecretAvailable&);
    void operator()(EndOfData&);

   private:
    AsyncFizzClientT<SM>& client_;
  };

  struct AsyncClientCallbackPtr {
    ~AsyncClientCallbackPtr() = default;

    explicit AsyncClientCallbackPtr(HandshakeCallback* in)
        : handshakePtr_(in), ptrType_(Type::HandshakeCallback) {}

    explicit AsyncClientCallbackPtr(folly::AsyncSocket::ConnectCallback* in)
        : asyncSocketConnCallbackPtr_(in),
          ptrType_(Type::AsyncSocketConnCallback) {}

    AsyncClientCallbackPtr(const AsyncClientCallbackPtr& other) {
      ptrType_ = other.ptrType_;
      if (other.ptrType_ == Type::AsyncSocketConnCallback) {
        asyncSocketConnCallbackPtr_ = other.asyncSocketConnCallbackPtr_;
      } else {
        handshakePtr_ = other.handshakePtr_;
      }
    }

    AsyncClientCallbackPtr& operator=(const AsyncClientCallbackPtr& other) {
      ptrType_ = other.ptrType_;
      if (other.ptrType_ == Type::AsyncSocketConnCallback) {
        asyncSocketConnCallbackPtr_ = other.asyncSocketConnCallbackPtr_;
      } else {
        handshakePtr_ = other.handshakePtr_;
      }
      return *this;
    }

    AsyncClientCallbackPtr& operator=(folly::AsyncSocket::ConnectCallback* in) {
      ptrType_ = Type::AsyncSocketConnCallback;
      handshakePtr_ = nullptr;
      asyncSocketConnCallbackPtr_ = in;
      return this;
    }

    AsyncClientCallbackPtr& operator=(HandshakeCallback* in) {
      ptrType_ = Type::HandshakeCallback;
      asyncSocketConnCallbackPtr_ = nullptr;
      handshakePtr_ = in;
      return this;
    }

    enum class Type : uint8_t {
      HandshakeCallback,
      AsyncSocketConnCallback,
    };

    HandshakeCallback* asHandshakeCallbackPtr() {
      if (ptrType_ == Type::HandshakeCallback) {
        return handshakePtr_;
      }
      return nullptr;
    }

    folly::AsyncSocket::ConnectCallback* asAsyncSocketConnCallbackPtr() {
      if (ptrType_ == Type::AsyncSocketConnCallback) {
        return asyncSocketConnCallbackPtr_;
      }
      return nullptr;
    }

    Type type() const {
      return ptrType_;
    }

   private:
    union {
      HandshakeCallback* handshakePtr_;
      folly::AsyncSocket::ConnectCallback* asyncSocketConnCallbackPtr_;
    };

    Type ptrType_;
  };

  folly::Optional<AsyncClientCallbackPtr> callback_;

  std::shared_ptr<const FizzClientContext> fizzContext_;

  std::shared_ptr<ClientExtensions> extensions_;

  folly::Optional<std::string> sni_;

  folly::Optional<std::string> pskIdentity_;

  State state_;

  ActionMoveVisitor visitor_;

  FizzClient<ActionMoveVisitor, SM> fizzClient_;

  struct EarlyDataState {
    // How much data is remaining in max early data size.
    uint32_t remainingEarlyData{0};

    // Early data that has been written so far. Only used with AutomaticResend
    // rejection policy.
    folly::IOBufQueue resendBuffer{folly::IOBufQueue::cacheChainLength()};

    // Writes that we haven't written yet due to exceeding the max early data
    // size.
    std::deque<AppWrite> pendingAppWrites;
  };

  // Only set if we are currently in early data state.
  folly::Optional<EarlyDataState> earlyDataState_;

  EarlyDataRejectionPolicy earlyDataRejectionPolicy_{
      EarlyDataRejectionPolicy::FatalConnectionError};

  folly::AsyncTransport::ReplaySafetyCallback* replaySafetyCallback_{nullptr};

  // Set when using socket connect() API to later pass into the state machine
  std::shared_ptr<const CertificateVerifier> verifier_;

  // Contains app writes pending a handshake success. Will be cleared once the
  // handshake succeeds.
  std::deque<AppWrite> pendingHandshakeAppWrites_;
};

using AsyncFizzClient = AsyncFizzClientT<ClientStateMachine>;
} // namespace client
} // namespace fizz

#include <fizz/client/AsyncFizzClient-inl.h>
