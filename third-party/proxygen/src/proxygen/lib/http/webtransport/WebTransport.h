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
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <proxygen/lib/http/HTTPMessage.h>

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
    SEND_ERROR
  };

  static bool isConnectMessage(const proxygen::HTTPMessage& msg) {
    static const std::string kWebTransport{"webtransport"};
    return msg.isRequest() &&
           msg.getMethod() == proxygen::HTTPMethod::CONNECT &&
           msg.getUpgradeProtocol() &&
           *msg.getUpgradeProtocol() == kWebTransport;
  }

  static constexpr uint64_t kFirstErrorCode = 0x52e4a40fa8db;
  static constexpr uint64_t kLastErrorCode = 0x52e5ac983162;

  static uint64_t toHTTPErrorCode(uint32_t n) {
    return kFirstErrorCode + n + (n / 0x1e);
  }

  static bool isEncodedApplicationErrorCode(uint64_t x) {
    return x >= kFirstErrorCode && x <= kLastErrorCode &&
           ((x - 0x21) % 0x1f) != 0;
  }

  static folly::Expected<uint32_t, WebTransport::ErrorCode>
  toApplicationErrorCode(uint64_t h) {
    if (!isEncodedApplicationErrorCode(h)) {
      // This is not for us
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
    uint64_t shifted = h - kFirstErrorCode;
    uint64_t appErrorCode = shifted - (shifted / 0x1f);
    DCHECK_LE(appErrorCode, std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(appErrorCode);
  }

  static constexpr uint32_t kInternalError =
      std::numeric_limits<uint32_t>::max();

  class Exception : public std::runtime_error {
   public:
    explicit Exception(uint32_t inError)
        : std::runtime_error(folly::to<std::string>(
              "Peer reset or abandoned stream with error=", inError)),
          error(inError) {
    }
    uint32_t error;
  };

  // The result of a read() operation
  struct StreamData {
    std::unique_ptr<folly::IOBuf> data;
    bool fin;
  };

  // Base class for StreamReadHandle / StreamWriteHandle
  class StreamHandleBase {
   public:
    virtual ~StreamHandleBase() = default;

    virtual uint64_t getID() = 0;

    // The caller may register a CancellationCallback on this token to be
    // notified of asynchronous cancellation of the stream by the peer.
    //
    // For StreamWriteHandle in particular, the handle is still valid in a
    // CancellationCallback, but not after that.  If the app doesn't terminate
    // the stream from the callback, the stream will be reset automatically.
    virtual folly::CancellationToken getCancelToken() = 0;
  };

  // Handle for read streams
  class StreamReadHandle : public StreamHandleBase {
   public:
    ~StreamReadHandle() override = default;

    // Wait for data to be delivered on the stream.  If the stream is reset by
    // the peer, a StreamReadHandle::Exception will be raised in the Future with
    // the error code.  The Future may observe other exceptions such as
    // folly::OperationCancelled if the session was closed, etc.
    //
    // The StreamReadHandle is invalid after reading StreamData with fin=true,
    // or an exception.
    virtual folly::SemiFuture<StreamData> readStreamData() = 0;

    using ReadStreamDataFn =
        std::function<void(StreamReadHandle*, folly::Try<StreamData>)>;
    void awaitNextRead(
        folly::Executor* exec,
        const ReadStreamDataFn& readCb,
        folly::Optional<std::chrono::milliseconds> timeout = folly::none) {
      auto fut = readStreamData();
      if (timeout) {
        fut = std::move(fut).within(*timeout);
      }
      std::move(fut).via(exec).thenTry([this, readCb](auto streamData) {
        readCb(this, std::move(streamData));
      });
    }

    // Notify the peer to stop sending data.  The StreamReadHandle is invalid
    // after this API call.
    virtual folly::Expected<folly::Unit, ErrorCode> stopSending(
        uint32_t error) = 0;
  };

  // Handle for write streams
  class StreamWriteHandle : public StreamHandleBase {
   public:
    ~StreamWriteHandle() override = default;

    // Write the data and optional fin to the stream.  The returned Future will
    // complete when the stream is available for more writes.
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
    virtual folly::Expected<folly::SemiFuture<folly::Unit>, ErrorCode>
    writeStreamData(std::unique_ptr<folly::IOBuf> data, bool fin) = 0;

    // Reset the stream with the given error
    virtual folly::Expected<folly::Unit, ErrorCode> resetStream(
        uint32_t error) = 0;

    // Error code from the peer's STOP_SENDING message
    folly::Optional<uint32_t> stopSendingErrorCode() {
      return stopSendingErrorCode_;
    }

   protected:
    folly::Optional<uint32_t> stopSendingErrorCode_;
  };

  // Handle for bidirectional streams
  struct BidiStreamHandle {
    StreamReadHandle* readHandle;
    StreamWriteHandle* writeHandle;
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
  virtual folly::Expected<folly::SemiFuture<folly::Unit>, ErrorCode>
  writeStreamData(uint64_t id,
                  std::unique_ptr<folly::IOBuf> data,
                  bool fin) = 0;
  virtual folly::Expected<folly::Unit, ErrorCode> resetStream(
      uint64_t streamId, uint32_t error) = 0;
  virtual folly::Expected<folly::Unit, ErrorCode> stopSending(
      uint64_t streamId, uint32_t error) = 0;

  // Sends the buffer as a datagram
  virtual folly::Expected<folly::Unit, ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) = 0;

  // Close the WebTransport session, with an optional error
  //
  // Any pending futures will complete with a folly::OperationCancelled
  // exception
  virtual folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error = folly::none) = 0;
};

} // namespace proxygen
