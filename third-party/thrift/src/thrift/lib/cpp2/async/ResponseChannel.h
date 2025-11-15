/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_ASYNC_RESPONSECHANNEL_H_
#define THRIFT_ASYNC_RESPONSECHANNEL_H_ 1

#include <chrono>
#include <limits>
#include <memory>

#include <folly/Portability.h>

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/async/ServerBiDiStreamFactory.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>

namespace folly {
class IOBuf;
}

extern const std::string kUnknownErrorCode;
extern const std::string kOverloadedErrorCode;
extern const std::string kAppOverloadedErrorCode;
extern const std::string kAppClientErrorCode;
extern const std::string kAppServerErrorCode;
extern const std::string kTaskExpiredErrorCode;
extern const std::string kProxyTransportExceptionErrorCode;
extern const std::string kProxyClientProtocolExceptionErrorCode;
extern const std::string kQueueOverloadedErrorCode;
extern const std::string kInjectedFailureErrorCode;
extern const std::string kServerQueueTimeoutErrorCode;
extern const std::string kResponseTooBigErrorCode;
extern const std::string kRequestTypeDoesntMatchServiceFunctionType;
extern const std::string kMethodUnknownErrorCode;
extern const std::string kInteractionIdUnknownErrorCode;
extern const std::string kInteractionConstructorErrorErrorCode;
extern const std::string kConnectionClosingErrorCode;
extern const std::string kRequestParsingErrorCode;
extern const std::string kChecksumMismatchErrorCode;
extern const std::string kUnimplementedMethodErrorCode;
extern const std::string kTenantQuotaExceededErrorCode;
extern const std::string kInteractionLoadsheddedErrorCode;
extern const std::string kInteractionLoadsheddedQueueTimeoutErrorCode;
extern const std::string kInteractionLoadsheddedOverloadErrorCode;
extern const std::string kInteractionLoadsheddedAppOverloadErrorCode;

namespace apache::thrift {

class ResponseChannelRequest {
 public:
  using UniquePtr =
      std::unique_ptr<ResponseChannelRequest, RequestsRegistry::Deleter>;

  virtual bool isActive() const = 0;

  virtual bool isOneway() const = 0;

  virtual bool isStream() const { return false; }

  virtual bool isSink() const { return false; }

  virtual bool isBiDiStream() const { return false; }

  virtual bool includeEnvelope() const = 0;

  apache::thrift::RpcKind rpcKind() const {
    if (isStream()) {
      return apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE;
    }
    if (isSink()) {
      return apache::thrift::RpcKind::SINK;
    }
    if (isOneway()) {
      return apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE;
    }
    if (isBiDiStream()) {
      return apache::thrift::RpcKind::BIDIRECTIONAL_STREAM;
    }
    return apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  }

  virtual bool isReplyChecksumNeeded() const { return false; }

  virtual void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback* cb = nullptr,
      folly::Optional<uint32_t> crc32 = folly::none) = 0;

  virtual void sendStreamReply(
      ResponsePayload&&,
      apache::thrift::detail::ServerStreamFactory&&,
      folly::Optional<uint32_t> = folly::none) {
    throw std::logic_error("unimplemented");
  }

  [[nodiscard]] static bool sendStreamReply(
      ResponseChannelRequest::UniquePtr request,
      folly::EventBase* eb,
      ResponsePayload&& payload,
      StreamServerCallbackPtr callback,
      folly::Optional<uint32_t> crc32 = folly::none) {
    // Destroying request can call onStreamCancel inline, which would be a
    // contract violation if we did it inline and returned true.
    SCOPE_EXIT {
      eb->runInEventBaseThread([request = std::move(request)] {});
    };
    return request->sendStreamReply(
        std::move(payload), std::move(callback), crc32);
  }

#if FOLLY_HAS_COROUTINES
  virtual void sendSinkReply(
      ResponsePayload&&,
      apache::thrift::detail::SinkConsumerImpl&&,
      folly::Optional<uint32_t> = folly::none) {
    throw std::logic_error("unimplemented");
  }

  [[nodiscard]] static bool sendSinkReply(
      ResponseChannelRequest::UniquePtr request,
      folly::EventBase* eb,
      ResponsePayload&& payload,
      SinkServerCallbackPtr callback,
      folly::Optional<uint32_t> crc32 = folly::none) {
    SCOPE_EXIT {
      eb->runInEventBaseThread([request = std::move(request)] {});
    };
    return request->sendSinkReply(
        std::move(payload), std::move(callback), crc32);
  }
#endif

  virtual void sendBiDiReply(
      ResponsePayload&&,
      detail::ServerBiDiStreamFactory&&,
      folly::Optional<uint32_t> = folly::none) {
    throw std::logic_error("unimplemented");
  }

  [[nodiscard]] static bool sendBiDiReply(
      ResponseChannelRequest::UniquePtr request,
      folly::EventBase* eb,
      ResponsePayload&& payload,
      BiDiServerCallbackPtr callback,
      folly::Optional<uint32_t> crc32 = folly::none) {
    SCOPE_EXIT {
      eb->runInEventBaseThread([request = std::move(request)] {});
    };
    return request->sendBiDiReply(
        std::move(payload), std::move(callback), crc32);
  }

  virtual void sendException(
      ResponsePayload&& response, MessageChannel::SendCallback* cb = nullptr) {
    // Until we start requesting payloads without the envelope we can pass any
    // sendException calls to sendReply
    sendReply(std::move(response), cb, folly::none);
  }

  virtual void sendErrorWrapped(
      folly::exception_wrapper ex, std::string exCode) = 0;

  virtual void sendQueueTimeoutResponse(
      bool /*interactionIsTerminated*/ = false) {}

  virtual ~ResponseChannelRequest() = default;

  // Get queue timeout for this request. So any owner who wants to enqueue this
  // request before processing can schedule timeout.
  virtual std::chrono::milliseconds getQueueTimeoutMs() const {
    return std::chrono::milliseconds(0);
  }

  bool getShouldStartProcessing() {
    if (!tryStartProcessing()) {
      return false;
    }
    return true;
  }

 protected:
  // callTryStartProcessing is a helper method used in ResponseChannelRequest
  // wrapper subclasses to delegate tryStartProcessing() calls to the wrapped
  // ResponseChannelRequest. This is necessary due to the protected nature of
  // tryStartProcessing().
  static bool callTryStartProcessing(ResponseChannelRequest* request) {
    return request->tryStartProcessing();
  }
  virtual bool tryStartProcessing() = 0;

  [[nodiscard]] virtual bool sendStreamReply(
      ResponsePayload&&,
      StreamServerCallbackPtr,
      folly::Optional<uint32_t> = folly::none) {
    throw std::logic_error("unimplemented");
  }
  [[nodiscard]] virtual bool sendSinkReply(
      ResponsePayload&&,
      SinkServerCallbackPtr,
      folly::Optional<uint32_t> = folly::none) {
    throw std::logic_error("unimplemented");
  }

  [[nodiscard]] virtual bool sendBiDiReply(
      ResponsePayload&&,
      BiDiServerCallbackPtr,
      folly::Optional<uint32_t> = folly::none) {
    throw std::logic_error("unimplemented");
  }

  bool startedProcessing_{false};
};

/**
 * ResponseChannel defines an asynchronous API for servers.
 */
class ResponseChannel : virtual public folly::DelayedDestruction {
 public:
  static const uint32_t ONEWAY_REQUEST_ID =
      std::numeric_limits<uint32_t>::max();

  class Callback {
   public:
    /**
     * reason is empty if closed due to EOF, or a pointer to an exception
     * if closed due to some sort of error.
     */
    virtual void channelClosed(folly::exception_wrapper&&) = 0;

    virtual ~Callback() {}
  };

  /**
   * The callback will be invoked on each new request.
   * It will remain installed until explicitly uninstalled, or until
   * channelClosed() is called.
   */
  virtual void setCallback(Callback*) = 0;

 protected:
  ~ResponseChannel() override {}
};

} // namespace apache::thrift

#endif // #ifndef THRIFT_ASYNC_RESPONSECHANNEL_H_
