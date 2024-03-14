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

#ifndef THRIFT_ASYNC_CPP2CONNECTION_H_
#define THRIFT_ASYNC_CPP2CONNECTION_H_ 1

#include <memory>
#include <unordered_set>

#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/HHWheelTimer.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/HeaderServerChannel.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/core/RequestStateMachine.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <wangle/acceptor/ManagedConnection.h>

namespace apache {
namespace thrift {

/**
 * Represents a connection that is handled via libevent. This connection
 * essentially encapsulates a socket that has some associated libevent state.
 */
class Cpp2Connection : public HeaderServerChannel::Callback,
                       public wangle::ManagedConnection {
 public:
  /**
   * Constructor for Cpp2Connection.
   *
   * @param asyncSocket shared pointer to the async socket
   * @param address the peer address of this connection
   * @param worker the worker instance that is handling this connection
   */
  Cpp2Connection(
      const std::shared_ptr<folly::AsyncTransport>& transport,
      const folly::SocketAddress* address,
      std::shared_ptr<Cpp2Worker> worker);

  /// Destructor -- close down the connection.
  ~Cpp2Connection() override;

  // HeaderServerChannel callbacks
  void requestReceived(
      std::unique_ptr<HeaderServerChannel::HeaderRequest>&&) override;
  void channelClosed(folly::exception_wrapper&&) override;

  void start() { channel_->setCallback(this); }

  void stop();

  void timeoutExpired() noexcept override;

  void requestTimeoutExpired();

  void queueTimeoutExpired();

  bool pending();

  // Managed Connection callbacks
  void describe(std::ostream&) const override {}
  bool isBusy() const override { return activeRequests_.empty(); }
  void notifyPendingShutdown() override {}
  void closeWhenIdle() override { stop(); }
  void dropConnection(const std::string& /* errorMsg */ = "") override {
    stop();
  }
  void dumpConnectionState(uint8_t /* loglevel */) override {}
  void addConnection(
      std::shared_ptr<Cpp2Connection> conn,
      std::optional<std::reference_wrapper<const wangle::TransportInfo>> tinfo =
          std::nullopt) {
    this_ = conn;
    if (tinfo) {
      if (auto* observer = worker_->getServer()->getObserver()) {
        observer->connAccepted(*tinfo);
        connectionAdded_ = true;
      }
    }
  }

  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return *(context_.getPeerAddress());
  }

  typedef apache::thrift::ThriftPresult<true>
      RocketUpgrade_upgradeToRocket_presult;
  template <class ProtocolWriter>
  ResponsePayload upgradeToRocketReply(int32_t protoSeqId) {
    folly::IOBufQueue queue;
    ProtocolWriter w;
    w.setOutput(&queue);
    w.writeMessageBegin("upgradeToRocket", MessageType::T_REPLY, protoSeqId);
    RocketUpgrade_upgradeToRocket_presult result;
    apache::thrift::detail::serializeResponseBody(&w, &result);
    w.writeMessageEnd();
    return ResponsePayload::create(queue.move());
  }

 protected:
  std::shared_ptr<apache::thrift::AsyncProcessorFactory>
      processorFactoryOverride_;
  apache::thrift::AsyncProcessorFactory& processorFactory_;
  Cpp2Worker::PerServiceMetadata& serviceMetadata_;
  std::unique_ptr<apache::thrift::AsyncProcessor> processor_;
  std::shared_ptr<apache::thrift::HeaderServerChannel> channel_;

  std::shared_ptr<Cpp2Worker> worker_;
  Cpp2Worker* getWorker() { return worker_.get(); }
  Cpp2ConnContext context_;

  std::shared_ptr<folly::AsyncTransport> transport_;
  std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager_;
  folly::Executor* executor_;

  /**
   * Wrap the request in our own request.  This is done for 2 reasons:
   * a) To have task timeouts for all requests,
   * b) To ensure the channel is not destroyed before callback is called
   */
  class Cpp2Request final : public ResponseChannelRequest {
   public:
    friend class Cpp2Connection;

    class QueueTimeout : public folly::HHWheelTimer::Callback {
      Cpp2Request* request_;
      void timeoutExpired() noexcept override;
      friend class Cpp2Request;
    };
    class TaskTimeout : public folly::HHWheelTimer::Callback {
      Cpp2Request* request_;
      void timeoutExpired() noexcept override;
      friend class Cpp2Request;
    };
    friend class QueueTimeout;
    friend class TaskTimeout;

    Cpp2Request(
        RequestsRegistry::DebugStub* debugStubToInit,
        std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
        std::shared_ptr<folly::RequestContext> rctx,
        std::shared_ptr<Cpp2Connection> con,
        rocket::Payload&& debugPayload,
        std::string&& methodName);

    bool isActive() const final { return stateMachine_.isActive(); }

    bool tryCancel() {
      return stateMachine_.tryCancel(connection_->getWorker()->getEventBase());
    }

    bool isOneway() const override { return req_->isOneway(); }

    bool isStream() const override { return req_->isStream(); }

    bool includeEnvelope() const override { return req_->includeEnvelope(); }

    void sendReply(
        ResponsePayload&& response,
        MessageChannel::SendCallback* notUsed = nullptr,
        folly::Optional<uint32_t> crc32c = folly::none) override;
    void sendException(
        ResponsePayload&& response,
        MessageChannel::SendCallback* notUsed = nullptr) override;
    void sendErrorWrapped(
        folly::exception_wrapper ew, std::string exCode) override;
    void sendQueueTimeoutResponse() override;
    void sendTimeoutResponse(
        apache::thrift::HeaderServerChannel::HeaderRequest::TimeoutResponseType
            responseType);

    ~Cpp2Request() override;

    // Cancel request is ususally called from a different thread than sendReply.
    virtual void cancelRequest();

    Cpp2RequestContext* getContext() { return &reqContext_; }

    server::TServerObserver::CallTimestamps& getTimestamps() {
      return static_cast<server::TServerObserver::CallTimestamps&>(
          reqContext_.getTimestamps());
    }

   protected:
    bool tryStartProcessing() final {
      return stateMachine_.tryStartProcessing();
    }

   private:
    MessageChannel::SendCallback* prepareSendCallback(
        MessageChannel::SendCallback* sendCallback,
        apache::thrift::server::TServerObserver* observer);

    std::unique_ptr<HeaderServerChannel::HeaderRequest> req_;

    // The order of these two fields matters; to save a shared_ptr operation, we
    // move into connection_ first and then use the pointer in connection_ to
    // initialize reqContext_; since field initialization happens in order of
    // definition, connection_ needs to appear before reqContext_.
    std::shared_ptr<Cpp2Connection> connection_;
    Cpp2RequestContext reqContext_;

    QueueTimeout queueTimeout_;
    TaskTimeout taskTimeout_;

    RequestStateMachine stateMachine_;

    Cpp2Worker::ActiveRequestsGuard activeRequestsGuard_;

    void cancelTimeout() {
      queueTimeout_.cancelTimeout();
      taskTimeout_.cancelTimeout();
    }
    void markProcessEnd();
    void setLatencyHeader(
        const std::string& key,
        const std::string& value,
        transport::THeader::StringToStringMap* newHeaders = nullptr) const;
  };

  class Cpp2Sample : public MessageChannel::SendCallback {
   public:
    Cpp2Sample(
        apache::thrift::server::TServerObserver::CallTimestamps& timestamps,
        apache::thrift::server::TServerObserver* observer,
        MessageChannel::SendCallback* chainedCallback = nullptr);

    void sendQueued() override;
    void messageSent() override;
    void messageSendError(folly::exception_wrapper&& e) override;
    ~Cpp2Sample() override;

   private:
    apache::thrift::server::TServerObserver::CallTimestamps timestamps_;
    apache::thrift::server::TServerObserver* observer_;
    MessageChannel::SendCallback* chainedCallback_;
  };

  folly::once_flag setupLoggingFlag_;
  folly::once_flag clientInfoFlag_;

  std::unordered_set<Cpp2Request*> activeRequests_;

  void removeRequest(Cpp2Request* req);
  void handleAppError(
      std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
      const std::string& name,
      const std::string& message,
      bool isClientError);
  void killRequest(
      std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
      TApplicationException::TApplicationExceptionType reason,
      const std::string& errorCode,
      const char* comment);
  void disconnect(const char* comment) noexcept;

  void setServerHeaders(transport::THeader::StringToStringMap& writeHeaders);
  void setServerHeaders(HeaderServerChannel::HeaderRequest& request);

  friend class Cpp2Request;

  std::shared_ptr<Cpp2Connection> this_;

  bool connectionAdded_{false};
};

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_ASYNC_CPP2CONNECTION_H_
