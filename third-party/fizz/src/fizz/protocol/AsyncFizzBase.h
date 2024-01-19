/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/KeyScheduler.h>
#include <fizz/record/Types.h>
#include <folly/io/IOBufIovecBuilder.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/WriteChainAsyncTransportWrapper.h>

namespace fizz {

using Cert = folly::AsyncTransportCertificate;

/**
 * This class is a wrapper around AsyncTransportWrapper to handle most app level
 * interactions. The derived client and server classes utilize the protected
 * methods.
 */
class AsyncFizzBase : public folly::WriteChainAsyncTransportWrapper<
                          folly::AsyncTransportWrapper>,
                      protected folly::AsyncTransportWrapper::WriteCallback,
                      protected folly::AsyncTransportWrapper::ReadCallback,
                      protected folly::EventRecvmsgCallback {
 public:
  using UniquePtr =
      std::unique_ptr<AsyncFizzBase, folly::DelayedDestruction::Destructor>;
  using ReadCallback = folly::AsyncTransportWrapper::ReadCallback;

  class HandshakeTimeout : public folly::AsyncTimeout {
   public:
    HandshakeTimeout(AsyncFizzBase& transport, folly::EventBase* eventBase)
        : folly::AsyncTimeout(eventBase), transport_(transport) {}

    ~HandshakeTimeout() override = default;

    void timeoutExpired() noexcept override {
      transport_.handshakeTimeoutExpired();
    }

   private:
    AsyncFizzBase& transport_;
  };

  class SecretCallback {
   public:
    virtual ~SecretCallback() = default;
    /**
     * Each of the below is called when the corresponding secret is received.
     */
    virtual void externalPskBinderAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void resumptionPskBinderAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void earlyExporterSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void clientEarlyTrafficSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void clientHandshakeTrafficSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void serverHandshakeTrafficSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void exporterMasterSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void resumptionMasterSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void clientAppTrafficSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
    virtual void serverAppTrafficSecretAvailable(
        const std::vector<uint8_t>&) noexcept {}
  };

  class EndOfTLSCallback {
   public:
    virtual ~EndOfTLSCallback() = default;
    virtual void endOfTLS(
        AsyncFizzBase* transport,
        std::unique_ptr<folly::IOBuf> endOfData) = 0;
  };

  /* Interface used to get a reference to an folly::IOBufIovecBuilder
   */
  struct IOVecQueueOps {
    virtual ~IOVecQueueOps() = default;
    virtual void allocateBuffers(folly::IOBufIovecBuilder::IoVecVec& iovs) = 0;
    virtual std::unique_ptr<folly::IOBuf> extractIOBufChain(size_t len) = 0;
  };

  struct TransportOptions {
    /**
     * Controls whether or not the async recv callback should be registered
     * (for io_uring)
     */
    bool registerEventCallback{false};

    /*
     * AsyncTransport read mode.
     *
     * This setting controls the strategy for reading data from the underlying
     * socket.
     *
     *   ReadMode::ReadBuffer (default)
     *      Under this mode, Fizz will allocate contiguous chunks of memory to
     *      read incoming encrypted records. This might lead to higher mem usage
     *      due to the way the memory is allocated from an IOBufQueue and also
     *      due to the inability to do in place decryption for shared buffers
     *
     *   ReadMode::ReadVec
     *      Under this mode, Fizz will use vectored IO (`readv`) to read
     *      incoming data. This can help avoid additional copies at the expense
     *      of allocating extra ref counting objects.
     *
     */
    folly::AsyncReader::ReadCallback::ReadMode readMode{ReadMode::ReadBuffer};

    /*
     * Under `ReadMode::ReadVec`, Fizz will allocate buffers for vectored I/O
     * through the `iovecQueue`.
     *
     * This field must be set if `readMode` is `ReadMode::ReadVec`.
     *
     * Multiple AsyncFizzBase instances may share the underlying
     * `IOVecQueueOps`, provided that the implementation allows it. This can be
     * useful in limiting the amount of memory allocated across a process with
     * many Fizz connections.
     */
    std::shared_ptr<IOVecQueueOps> ioVecQueue{};

    /**
     * Under ReadMode::ReadBuffer, whenever Fizz's available read buffer space
     * is under `readBufferMinReadSize`, `readBufferAllocationSize` controls the
     * size of the buffer that Fizz will allocate.
     *
     * Higher values will result in larger memory allocations which may
     * negatively affect memory usage on servers with many idle connections.
     * However, this may result in fewer syscalls and allocation calls being
     * made on the data path.
     */
    size_t readBufferAllocationSize{4000};

    /**
     * Under ReadMode::ReadBuffer, whenever the underlying transport becomes
     * available to read, Fizz will ensure that *at least*
     * `readBuferMinReadSize` worth of contiguous data is read per read call. If
     * the internal read buffer is smaller than `readBufferMinReadSize`, then
     * Fizz will allocate `readBufferAllocationSize` worth of new buffer space
     * to satisfy this read.
     */
    size_t readBufferMinReadSize{1460};
    /**
     * When set, `zeroCopyMemStore` points to an instance of a
     * `ZeroCopyMemStore` that outlives all `AsyncFizzBase` instances.
     *
     * When set, Fizz will attempt to perform TCP zero copy receives. This
     * requires appropriate kernel and hardware support. With appropriate
     * support, encrypted data received by the NIC will be handed directly to
     * Fizz for decryption (normally, Fizz will copy data from the kernel).
     */
    ZeroCopyMemStore* zeroCopyMemStore{nullptr};
  };

  explicit AsyncFizzBase(
      folly::AsyncTransportWrapper::UniquePtr transport,
      TransportOptions options);

  ~AsyncFizzBase() override;

  /**
   * App level information for reading/writing app data.
   */
  ReadCallback* getReadCallback() const override;
  void setReadCB(ReadCallback* callback) override;
  void writeChain(
      folly::AsyncTransportWrapper::WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override;

  /**
   * App data usage accounting.
   */
  size_t getAppBytesWritten() const override;
  size_t getAppBytesReceived() const override;
  size_t getAppBytesBuffered() const override;

  /**
   * Information about the current transport state.
   * To be implemented by derived classes.
   */
  bool good() const override = 0;
  bool readable() const override = 0;
  bool connecting() const override = 0;
  bool error() const override = 0;

  /**
   * Get the certificates in fizz::Cert form.
   */
  const Cert* getPeerCertificate() const override = 0;

  const Cert* getSelfCertificate() const override = 0;

  bool isReplaySafe() const override = 0;
  void setReplaySafetyCallback(
      folly::AsyncTransport::ReplaySafetyCallback* callback) override = 0;
  std::string getApplicationProtocol() const noexcept override = 0;

  /**
   * Get the CipherSuite negotiated in this transport.
   */
  virtual folly::Optional<CipherSuite> getCipher() const = 0;

  /**
   * Get the NamedGroup negotiated in this transport.
   */
  virtual folly::Optional<NamedGroup> getGroup() const = 0;

  /**
   * Get the supported signature schemes in this transport.
   */
  virtual std::vector<SignatureScheme> getSupportedSigSchemes() const = 0;

  /**
   * Get the exported material.
   */
  Buf getExportedKeyingMaterial(
      folly::StringPiece label,
      Buf context,
      uint16_t length) const override = 0;

  /**
   * Clean up transport on destruction
   */
  void destroy() override;

  /**
   * Identify the transport as Fizz.
   */
  std::string getSecurityProtocol() const override {
    return "Fizz";
  }

  /**
   * EventBase operations.
   */
  void attachTimeoutManager(folly::TimeoutManager* manager) {
    handshakeTimeout_.attachTimeoutManager(manager);
  }
  void detachTimeoutManager() {
    handshakeTimeout_.detachTimeoutManager();
  }
  void attachEventBase(folly::EventBase* eventBase) override {
    handshakeTimeout_.attachEventBase(eventBase);
    transport_->attachEventBase(eventBase);
    resumeEvents();

    // we want to avoid setting a read cb on a bad transport (i.e. closed or
    // disconnected) unless we have a read callback we can pass the errors to.
    if (transport_->good() || readCallback_) {
      startTransportReads();
    }
  }
  void detachEventBase() override {
    handshakeTimeout_.detachEventBase();
    transport_->setEventCallback(nullptr);
    transport_->setReadCB(nullptr);
    transport_->detachEventBase();
    pauseEvents();
  }
  bool isDetachable() const override {
    return !handshakeTimeout_.isScheduled() && transport_->isDetachable();
  }

  void setSecretCallback(SecretCallback* cb) {
    secretCallback_ = cb;
  }

  SecretCallback* getSecretCallback() {
    return secretCallback_;
  }

  // Note we clearly do not own the callback, and thus it is the caller's
  // responsibility to ensure the callback outlives the lifetime of
  // the fizz base instance. There are a couple key behavior differences if
  // this callback is set.
  // 1. We do not close the transport on receivng a close notify. It is your
  // responsibility to do whatever is appropriate.
  // 2. We do not call readEOF on any read callback set on the transport.
  // 3. Depending on when the tls connection is closed, there may be pending
  // data that exists past the close notify, this is passed along to the caller
  // in the endOfTLS method and the caller must decide what to do with the data
  virtual void setEndOfTLSCallback(EndOfTLSCallback* cb) {
    endOfTLSCallback_ = cb;
  }

  /**
   * setHandshakeRecordAlignedReads defines the behavior for reading data
   * from the backing transport during the handshake.
   *
   * This must be called prior to initiating the handshake.
   *
   * If true, this indicates that during the handshake, Fizz will read data
   * such that at the end of the handshake, the next byte in the underlying
   * transport's buffer (e.g. the kernel buffer) is guaranteed to be aligned
   * on a record boundary.
   *
   * In practice, this means that during the handshake, Fizz will read records
   * by (1) reading the record header and (2) reading just enough bytes to
   * complete the current record. This uses more system calls.
   *
   * If false, Fizz will read data from the underlying transport in chunks not
   * tied to any record boundary.
   */
  void setHandshakeRecordAlignedReads(bool flag) {
    constexpr size_t kRecordHeaderSize = 5;
    if (flag) {
      readSizeHint_ = kRecordHeaderSize;
    }
  }

  /**
   * If set, after AsyncFizzBase has been used to write `bytes` worth of data,
   * AsyncFizzBase will automatically initiate a key_update
   */
  void setRekeyAfterWriting(size_t bytes) {
    keyUpdateThreshold_ = bytes;
  }

  /*
   * Gets the threshold value set for automatic key update
   */
  size_t getRekeyAfterWriting() const {
    return keyUpdateThreshold_;
  }
  /*
   * Gets the client random associated with this connection. The CR can be
   * used as a transport agnostic identifier (for instance, for NSS keylogging)
   */
  virtual folly::Optional<Random> getClientRandom() const = 0;

  /*
   * Used to shut down the tls session, without shutting down the underlying
   * transport.
   */
  virtual void tlsShutdown() = 0;

  /*
   * Redeclared from AsyncTransport. Attempts to perform a full TLS shutdown,
   * then shuts down the underlying transport.
   */
  virtual void shutdownWrite() override = 0;

  /*
   * Redeclared from AsyncTransport. Attempts to perform a full TLS shutdown,
   * then shuts down the underlying transport.
   */
  virtual void shutdownWriteNow() override = 0;

  /*
   * Sets whether or not to force in-place decryption of records. This is
   * usually safe to do, as long as the application can handle chained IOBufs
   * (as opposed to a contiguous buffer in the non-in-place case).
   */
  void setDecryptInplace(bool inPlace) {
    readAeadOptions_.bufferOpt = inPlace
        ? Aead::BufferOption::AllowInPlace
        : Aead::BufferOption::RespectSharedPolicy;
  }

  /*
   * This sets whether or not to always perform encryption in-place, using the
   * same buffers passed in for writing to hold the encrypted data. The code
   * will do this opportunistically in certain cases (unique buffer passed in
   * and not split into records), but by setting this to true you can indicate
   * that the buffers passed in can always be used for in-place encryption
   * safely. This is not enabled by default (for safety).
   *
   * If you pass in unshared IOBufs for writing, you can set this to true.
   * Otherwise, if you have a shared buffer, its contents will be overwritten
   * (without throwing an error), affecting the other IOBufs sharing the
   * underlying buffer. Thus, in many cases it's not appropriate to set this to
   * true when passing in shared buffers, as the original plaintext in the
   * buffer will be lost.
   */
  void setEncryptInplace(bool inPlace) {
    writeAeadOptions_.bufferOpt = inPlace
        ? Aead::BufferOption::AllowInPlace
        : Aead::BufferOption::RespectSharedPolicy;
  }

  /* Initialize a key update. */
  virtual void initiateKeyUpdate(KeyUpdateRequest keyUpdateRequest) = 0;

 protected:
  /**
   * Start reading raw data from the transport.
   */
  virtual void startTransportReads();

  /**
   * Interface for the derived class to schedule a handshake timeout.
   *
   * transportError() will be called if the timeout fires before it is
   * cancelled.
   */
  virtual void startHandshakeTimeout(std::chrono::milliseconds);
  virtual void cancelHandshakeTimeout();

  /**
   * Interfaces for the derived class to interact with the app level read
   * callback.
   */
  virtual void deliverAppData(std::unique_ptr<folly::IOBuf> buf);
  virtual void deliverError(
      const folly::AsyncSocketException& ex,
      bool closeTransport = true);

  /**
   * Interface for the derived class to implement to receive app data from the
   * app layer.
   */
  virtual void writeAppData(
      folly::AsyncTransportWrapper::WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) = 0;

  /**
   * Alert the derived class that a transport error occured.
   */
  virtual void transportError(const folly::AsyncSocketException& ex) = 0;

  /**
   * Alert the derived class that additional data is available in
   * transportReadBuf_.
   */
  virtual void transportDataAvailable() = 0;

  /**
   * Alert the derived class that new event processing should be paused/resumed.
   */
  virtual void pauseEvents() = 0;
  virtual void resumeEvents() = 0;

  /**
   * Allows the derived class to give a derived secret to the secret callback.
   */
  virtual void secretAvailable(const DerivedSecret& secret) noexcept;

  /**
   * Signal end of tls connection by a graceful shutdown.
   */
  virtual void endOfTLS(std::unique_ptr<folly::IOBuf> endOfData) noexcept;

  /**
   * Called by derived classes to control the size of the next read from the
   * underlying transport (if using the readDataAvailable() API) when
   * the transport performs record aligned reads.
   *
   * Record aligned reads are not the default; it must be explicitly enabled
   * through AsyncFizzBase::setHandshakeRecordAlignedReads()
   *
   * setting hint=0 disables this functionality. All subsequent updateReadHint()
   * values will be ignored.
   */
  void updateReadHint(size_t hint) {
    if (readSizeHint_ > 0) {
      readSizeHint_ = hint;
    }
  }

  /**
   * Called by derived classes after an AppWrite of `bytesWritten` bytes is
   * performed.
   */
  void wroteApplicationBytes(size_t bytesWritten) {
    appByteProcessedUnderKey_ += bytesWritten;
    if (keyUpdateThreshold_ &&
        appByteProcessedUnderKey_ >= keyUpdateThreshold_) {
      appByteProcessedUnderKey_ = 0;
      initiateKeyUpdate(KeyUpdateRequest::update_not_requested);
    }
  }

  folly::IOBufQueue transportReadBuf_{folly::IOBufQueue::cacheChainLength()};
  Aead::AeadOptions readAeadOptions_;
  Aead::AeadOptions writeAeadOptions_;

  size_t appByteProcessedUnderKey_{0};
  size_t keyUpdateThreshold_{0};

 private:
  class QueuedWriteRequest
      : private folly::AsyncTransportWrapper::WriteCallback {
   public:
    QueuedWriteRequest(
        AsyncFizzBase* base,
        folly::AsyncTransportWrapper::WriteCallback* callback,
        std::unique_ptr<folly::IOBuf> data,
        folly::WriteFlags flags);

    void startWriting();

    void append(QueuedWriteRequest* request);

    void unlinkFromBase();

    void fail(const folly::AsyncSocketException&);

    size_t getEntireChainBytesBuffered() {
      DCHECK(!next_);
      return entireChainBytesBuffered;
    }

   private:
    void writeSuccess() noexcept override;

    void writeErr(size_t, const folly::AsyncSocketException&) noexcept override;

    QueuedWriteRequest* deliverSingleWriteErr(
        const folly::AsyncSocketException&);

    void advanceOnBase();

    AsyncFizzBase* asyncFizzBase_;
    folly::AsyncTransportWrapper::WriteCallback* callback_;
    folly::IOBufQueue data_{folly::IOBufQueue::cacheChainLength()};
    folly::WriteFlags flags_;

    size_t dataWritten_{0};
    // Data length of the entire chain. Only valid at the tail node
    // of the chain, i.e. when next_ is null.
    size_t entireChainBytesBuffered;

    QueuedWriteRequest* next_{nullptr};
  };

  class FizzMsgHdr;

  /**
   * EventRecvmsgCallback implementation
   */
  folly::EventRecvmsgCallback::MsgHdr* allocateData() noexcept override;
  void eventRecvmsgCallback(FizzMsgHdr* msgHdr, int res);

  /**
   * ReadCallback implementation.
   */
  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void getReadBuffers(folly::IOBufIovecBuilder::IoVecVec& iovs) override;
  void readDataAvailable(size_t len) noexcept override;
  bool isBufferMovable() noexcept override;
  void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> data) noexcept override;
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException& ex) noexcept override;
  /* ZC RX related*/
  ZeroCopyMemStore* readZeroCopyEnabled() noexcept override;
  void getZeroCopyFallbackBuffer(
      void** /*bufReturn*/,
      size_t* /*lenReturn*/) noexcept override;
  void readZeroCopyDataAvailable(
      std::unique_ptr<folly::IOBuf>&& /*zeroCopyData*/,
      size_t /*additionalBytes*/) noexcept override;

  /**
   * WriteCallback implementation, for use with handshake messages.
   */
  void writeSuccess() noexcept override;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept override;

  void checkBufLen();

  void handshakeTimeoutExpired() noexcept;

  void
  getReadBuffer(folly::IOBufQueue& buf, void** bufReturn, size_t* lenReturn);

  ReadCallback* readCallback_{nullptr};
  std::unique_ptr<folly::IOBuf> appDataBuf_;

  size_t appBytesWritten_{0};
  size_t appBytesReceived_{0};

  size_t readSizeHint_{0};

  QueuedWriteRequest* tailWriteRequest_{nullptr};
  QueuedWriteRequest* immediatelyPendingWriteRequest_{nullptr};

  HandshakeTimeout handshakeTimeout_;

  SecretCallback* secretCallback_{nullptr};
  EndOfTLSCallback* endOfTLSCallback_{nullptr};

  TransportOptions transportOptions_;
  std::unique_ptr<FizzMsgHdr> msgHdr_;

  folly::IOBufQueue zeroCopyFallbackReadBuf_{
      folly::IOBufQueue::cacheChainLength()};
};
} // namespace fizz
