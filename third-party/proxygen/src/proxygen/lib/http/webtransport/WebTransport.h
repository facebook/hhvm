/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/CancellationToken.h>
#include <folly/Expected.h>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <quic/api/QuicCallbacks.h>
#include <quic/api/TransportInfo.h>

namespace proxygen {

// Generic WebTransport interface
//
// Principles:
//
// 1. It should be easy to write simple applications
// 2. The backpressure and error handling APIs should be understandable
// 3. The same generic API should work for proxygen/lib and proxygen::coro
//
// Futures is the best way to implement #3 because they can be easily
// awaited in a coroutine.
//
// Note there are no APIs to await new streams opened by the peer,
// datagrams sent by the peer, or session closure.
//  => These signals are delivered via the HTTP API

class WebTransport {
 public:
  virtual ~WebTransport() = default;

  // Errors that can be returned from API
  enum class ErrorCode {
    GENERIC_ERROR = 0x00,
    INVALID_STREAM_ID,
    STREAM_CREATION_ERROR,
    SEND_ERROR,
    STOP_SENDING,
    SESSION_TERMINATED
  };

  static constexpr uint64_t kFirstErrorCode = 0x52e4a40fa8db;
  static constexpr uint64_t kLastErrorCode = 0x52e5ac983162;
  static constexpr uint32_t kSessionGone = 0x170d7b68;
  static constexpr uint32_t kInternalError =
      std::numeric_limits<uint32_t>::max();

  static uint64_t toHTTPErrorCode(uint32_t n) {
    return kFirstErrorCode + n + (n / 0x1e);
  }

  static bool isEncodedApplicationErrorCode(uint64_t x) {
    return x >= kFirstErrorCode && x <= kLastErrorCode &&
           ((x - 0x21) % 0x1f) != 0;
  }

  static folly::Expected<uint32_t, WebTransport::ErrorCode>
  toApplicationErrorCode(uint64_t h);

  class Exception : public std::runtime_error {
   public:
    explicit Exception(uint32_t inError);
    Exception(uint32_t inError, const std::string& msg);
    uint32_t error;
  };

  // The result of a read() operation
  struct StreamData {
    std::unique_ptr<folly::IOBuf> data{nullptr};
    bool fin{false};
  };

  // Base class for StreamReadHandle / StreamWriteHandle
  class StreamHandleBase {
   public:
    virtual ~StreamHandleBase() = default;

    uint64_t getID() const {
      return id_;
    }

    // The caller may register a CancellationCallback on this token to be
    // notified of asynchronous cancellation of the stream by the peer.
    //
    // For StreamWriteHandle in particular, the handle is still valid in a
    // CancellationCallback, but not after that.  If the app doesn't terminate
    // the stream from the callback, the stream will be reset automatically.
    folly::CancellationToken getCancelToken() const {
      return cs_.getToken();
    }

   protected:
    StreamHandleBase(uint64_t id) : id_(id) {
    }

    const uint64_t id_;
    folly::CancellationSource cs_;
  };

  // Handle for read streams
  class StreamReadHandle : public StreamHandleBase {
   public:
    using StreamHandleBase::StreamHandleBase;
    ~StreamReadHandle() override = default;

    // Wait for data to be delivered on the stream.  If the stream is reset by
    // the peer, a StreamReadHandle::Exception will be raised in the Future with
    // the error code.  The Future may observe other exceptions such as
    // folly::OperationCancelled if the session was closed, etc.
    //
    // The StreamReadHandle is invalid after reading StreamData with fin=true,
    // or an exception.
    virtual folly::SemiFuture<StreamData> readStreamData() = 0;

    using ReadStreamDataFn = std::function<void(
        StreamReadHandle*, uint64_t streamId, folly::Try<StreamData>)>;
    void awaitNextRead(
        folly::Executor* exec,
        ReadStreamDataFn readCb,
        folly::Optional<std::chrono::milliseconds> timeout = folly::none);

    // Notify the peer to stop sending data.  The StreamReadHandle is invalid
    // after this API call.
    virtual folly::Expected<folly::Unit, ErrorCode> stopSending(
        uint32_t error) = 0;
  };

  class ByteEventCallback : public quic::ByteEventCallback {
   public:
    ByteEventCallback() = default;

    ~ByteEventCallback() override = default;

    virtual void onByteEvent(quic::StreamId id, uint64_t offset) noexcept = 0;

    virtual void onByteEventCanceled(quic::StreamId id,
                                     uint64_t offset) noexcept = 0;

    // setWritePrefaceSize is called by the WebTransport layer, and doesn't need
    // to be called by the user.
    // The reason this is present is that some WebTransport implementations
    // write a preface to the stream once it is created. We want to pass the
    // correct value of offset to the onByteEvent and onByteEventCanceled
    // callbacks.
    void setWritePrefaceSize(uint32_t writePrefaceSize) {
      writePrefaceSize_ = writePrefaceSize;
    }

   private:
    void onByteEvent(quic::ByteEvent byteEvent) final {
      onByteEvent(byteEvent.id, byteEvent.offset - writePrefaceSize_);
    }

    void onByteEventCanceled(quic::ByteEventCancellation cancellation) final {
      onByteEventCanceled(cancellation.id,
                          cancellation.offset - writePrefaceSize_);
    }

   private:
    uint32_t writePrefaceSize_{0};
  };

  enum class FCState { BLOCKED, UNBLOCKED, SESSION_CLOSED };
  // Handle for write streams
  class StreamWriteHandle : public StreamHandleBase {
   public:
    using StreamHandleBase::StreamHandleBase;
    ~StreamWriteHandle() override = default;

    // Write the data and optional fin to the stream.
    //
    // The StreamWriteHandle becomes invalid after calling writeStreamData with
    // fin=true or calling resetStream.
    //
    // If the peer sends a STOP_SENDING, the app is notified via the
    // CancellationToken for this handle, and the code can be queried via
    // stopSendingErrorCode.  The app SHOULD reset the stream from a
    // CancellationCallback.  Calling writeStreamData from the callback will
    // fail with a WebTransport::Exception with the stopSendingErrorCode.
    // After the cancellation callback, the StreamWriteHandle is invalid.
    virtual folly::Expected<FCState, ErrorCode> writeStreamData(
        std::unique_ptr<folly::IOBuf> data,
        bool fin,
        ByteEventCallback* byteEventCallback) = 0;

    // Reset the stream with the given error
    virtual folly::Expected<folly::Unit, ErrorCode> resetStream(
        uint32_t error) = 0;

    // Error code from the peer's STOP_SENDING message
    folly::Optional<uint32_t> stopSendingErrorCode() {
      return stopSendingErrorCode_;
    }

    virtual folly::Expected<folly::Unit, ErrorCode> setPriority(
        uint8_t level, uint32_t order, bool incremental) = 0;

    // The returned Future will complete when the stream is available for more
    // writes.
    virtual folly::Expected<folly::SemiFuture<uint64_t>, ErrorCode>
    awaitWritable() = 0;

   protected:
    folly::Optional<uint32_t> stopSendingErrorCode_;
  };

  // Handle for bidirectional streams
  struct BidiStreamHandle {
    StreamReadHandle* readHandle{nullptr};
    StreamWriteHandle* writeHandle{nullptr};
  };

  // Create a new unidirectional stream
  //
  // Returns a StreamWriteHandle to the new stream if successful, or ErrorCode,
  // including in cases where stream credit is exhausted
  virtual folly::Expected<StreamWriteHandle*, ErrorCode> createUniStream() = 0;

  // Create a new bididirectional stream
  //
  // Returns a BidiStreamHandle to the new stream if successful, or ErrorCode,
  // including in cases where stream credit is exhausted.  Note the application
  // needs to call readStreamData to read from the read half.
  virtual folly::Expected<BidiStreamHandle, ErrorCode> createBidiStream() = 0;

  // Wait for credit to create a stream of the given type.  If stream credit
  // is available, will immediately return a ready SemiFuture.
  virtual folly::SemiFuture<folly::Unit> awaitUniStreamCredit() = 0;
  virtual folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() = 0;

  // API using stream IDs
  // These methods may be used if the app wants to manipulate open streams
  // without holding their handles
  virtual folly::Expected<folly::SemiFuture<StreamData>,
                          WebTransport::ErrorCode>
  readStreamData(uint64_t id) = 0;
  virtual folly::Expected<FCState, ErrorCode> writeStreamData(
      uint64_t id,
      std::unique_ptr<folly::IOBuf> data,
      bool fin,
      ByteEventCallback* deliveryCallback) = 0;
  virtual folly::Expected<folly::Unit, ErrorCode> resetStream(
      uint64_t streamId, uint32_t error) = 0;
  virtual folly::Expected<folly::Unit, ErrorCode> setPriority(
      uint64_t streamId, uint8_t level, uint32_t order, bool incremental) = 0;
  virtual folly::Expected<folly::SemiFuture<uint64_t>, ErrorCode> awaitWritable(
      uint64_t streamId) = 0;

  virtual folly::Expected<folly::Unit, ErrorCode> stopSending(
      uint64_t streamId, uint32_t error) = 0;

  // Sends the buffer as a datagram
  virtual folly::Expected<folly::Unit, ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) = 0;

  // Get the local and peer socket addresses
  virtual const folly::SocketAddress& getLocalAddress() const = 0;
  virtual const folly::SocketAddress& getPeerAddress() const = 0;

  // Close the WebTransport session, with an optional error
  //
  // Any pending futures will complete with a folly::OperationCancelled
  // exception
  // Return QUIC transport statistics similar to TCPInfo
  [[nodiscard]] virtual quic::TransportInfo getTransportInfo() const = 0;

  virtual folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error = folly::none) = 0;
};

// WebTransportHandler is a virtual interface for handling events that come
// from web transport that are not tied to an existing stream
//
//  * New streams
//  * Datagrams
//  * The end of of session
class WebTransportHandler {
 public:
  virtual ~WebTransportHandler() = default;

  virtual void onNewUniStream(WebTransport::StreamReadHandle* readHandle) = 0;
  virtual void onNewBidiStream(WebTransport::BidiStreamHandle bidiHandle) = 0;
  virtual void onDatagram(std::unique_ptr<folly::IOBuf> datagram) = 0;
  virtual void onSessionEnd(folly::Optional<uint32_t> error) = 0;
};

} // namespace proxygen
