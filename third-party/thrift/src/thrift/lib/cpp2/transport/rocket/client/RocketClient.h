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

#pragma once

#include <chrono>
#include <memory>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/Try.h>
#include <folly/container/F14Set.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseLocal.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/client/RequestContext.h>
#include <thrift/lib/cpp2/transport/rocket/client/RequestContextQueue.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketStreamServerCallback.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace folly {
class IOBuf;
} // namespace folly

namespace apache {
namespace thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::optional<TransportMetadataPush>,
    getTransportMetadataPush,
    folly::AsyncTransport*);
} // namespace detail

namespace rocket {

class RocketClient : public virtual folly::DelayedDestruction,
                     private folly::AsyncTransport::WriteCallback {
 public:
  using FlushList = boost::intrusive::list<
      folly::EventBase::LoopCallback,
      boost::intrusive::constant_time_size<false>>;

  RocketClient(const RocketClient&) = delete;
  RocketClient(RocketClient&&) = delete;
  RocketClient& operator=(const RocketClient&) = delete;
  RocketClient& operator=(RocketClient&&) = delete;

  ~RocketClient() override;

  void handleFrame(std::unique_ptr<folly::IOBuf> frame);

  using Ptr =
      std::unique_ptr<RocketClient, folly::DelayedDestruction::Destructor>;
  static Ptr create(
      folly::EventBase& evb,
      folly::AsyncTransport::UniquePtr socket,
      std::unique_ptr<SetupFrame> setupFrame);

  using WriteSuccessCallback = RequestContext::WriteSuccessCallback;
  class RequestResponseCallback : public WriteSuccessCallback {
   public:
    virtual void onResponsePayload(
        folly::AsyncTransport* transport,
        folly::Try<Payload>&& response) noexcept = 0;
  };

  FOLLY_NODISCARD folly::Try<Payload> sendRequestResponseSync(
      Payload&& request,
      std::chrono::milliseconds timeout,
      WriteSuccessCallback* callback);

  void sendRequestResponse(
      Payload&& request,
      std::chrono::milliseconds timeout,
      std::unique_ptr<RequestResponseCallback> callback);

  class RequestFnfCallback {
   public:
    virtual ~RequestFnfCallback() = default;
    virtual void onWrite(folly::Try<void> writeResult) noexcept = 0;
  };

  FOLLY_NODISCARD folly::Try<void> sendRequestFnfSync(Payload&& request);

  void sendRequestFnf(
      Payload&& request, std::unique_ptr<RequestFnfCallback> callback);

  void sendRequestStream(
      Payload&& request,
      std::chrono::milliseconds firstResponseTimeout,
      std::chrono::milliseconds chunkTimeout,
      int32_t initialRequestN,
      StreamClientCallback* clientCallback);

  void sendRequestSink(
      Payload&& request,
      std::chrono::milliseconds firstResponseTimeout,
      SinkClientCallback* clientCallback,
      folly::Optional<CompressionConfig> compressionConfig = folly::none);

  FOLLY_NODISCARD bool sendRequestN(StreamId streamId, int32_t n);
  void cancelStream(StreamId streamId);
  FOLLY_NODISCARD bool sendPayload(
      StreamId streamId, StreamPayload&& payload, Flags flags);
  void sendError(StreamId streamId, RocketException&& rex);
  void sendComplete(StreamId streamId, bool closeStream);
  FOLLY_NODISCARD bool sendHeadersPush(
      StreamId streamId, HeadersPayload&& payload);
  // sink error can use different frames depends on server version
  FOLLY_NODISCARD bool sendSinkError(
      StreamId streamId, StreamPayload&& payload);

  bool streamExists(StreamId streamId) const;

  // AsyncTransport::WriteCallback implementation
  void writeSuccess() noexcept override;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& e) noexcept override;

  void setCloseCallback(folly::Function<void()> closeCallback) {
    closeCallback_ = std::move(closeCallback);
  }

  bool isAlive() const {
    return clientState_.connState == ConnectionState::CONNECTED;
  }

  const folly::exception_wrapper& getLastError() const { return error_; }

  size_t streams() const { return streams_.size(); }

  size_t requests() const { return clientRequests_; }

  const folly::AsyncTransport* getTransportWrapper() const {
    return socket_.get();
  }

  folly::AsyncTransport* getTransportWrapper() { return socket_.get(); }

  bool isDetachable() const {
    if (!evb_) {
      return false;
    }
    evb_->dcheckIsInEventBaseThread();

    // Client is only detachable if there are no inflight requests, no active
    // streams, and if the underlying transport is detachable, i.e., has no
    // inflight writes of its own.
    return !writeLoopCallback_.isLoopCallbackScheduled() && !requests_ &&
        streams_.empty() && (!socket_ || socket_->isDetachable()) &&
        parser_.getReadBufLength() == 0 && !interactions_;
  }

  void attachEventBase(folly::EventBase& evb);
  void detachEventBase();

  void setOnDetachable(folly::Function<void()> onDetachable) {
    onDetachable_ = std::move(onDetachable);
  }

  void notifyIfDetachable() {
    if (clientState_.connState == ConnectionState::CLOSING && !requests_ &&
        streams_.empty()) {
      close(transport::TTransportException(
          transport::TTransportException::END_OF_FILE,
          "Connection closed by server"));
    }
    if (!onDetachable_ || !isDetachable()) {
      return;
    }
    if (detachableLoopCallback_.isLoopCallbackScheduled()) {
      return;
    }
    evb_->runInLoop(&detachableLoopCallback_);
  }

  /**
   * Set a centrilized list for flushing all pending writes.
   *
   * By default EventBase itself is used to schedule flush callback at the end
   * of the next event loop. If flushList is not null, RocketClient would attach
   * callbacks to it intstead, thus granting control over when writes happen to
   * the application.
   *
   * Note: the caller is responsible for making sure that the list outlives the
   * client.
   *
   * Note2: call to detachEventBase() would reset the list back to nullptr.
   */
  void setFlushList(FlushList* flushList) { flushList_ = flushList; }

  class FlushManager : private folly::EventBase::LoopCallback,
                       public folly::AsyncTimeout {
   public:
    explicit FlushManager(folly::EventBase& evb) : evb_(evb) {
      attachEventBase(&evb_);
    }
    static FlushManager& getInstance(folly::EventBase& evb) {
      return getEventBaseLocal().try_emplace(evb, evb);
    }
    void enqueueFlush(RocketClient& client);
    // has time complexity linear to number of elements in flush list
    size_t getNumPendingClients() const { return flushList_.size(); }

    void timeoutExpired() noexcept override;

    /*
     * When not using setFlushList to manage flushes, this sets the flush
     * policy for the FlushManager. maxPendingFlushes is the number of client
     * flushes which will be batched before scheduling a flush in the next
     * loop callback. maxFlushLatency is the amount of time to wait for
     * maxPendingFlushes before scheduling a loop callback. I.e., it is the
     * latency tolerance for a RocketClient's flush.
     */
    void setFlushPolicy(
        size_t maxPendingFlushes, std::chrono::microseconds maxFlushLatency) {
      flushPolicy_.emplace(maxPendingFlushes, maxFlushLatency);
    }

    /*
     * Reset the flush policy to no policy. Also act as if the timeout elapsed
     * immediately.
     */
    void resetFlushPolicy();

   private:
    void runLoopCallback() noexcept override final;

    folly::EventBase& evb_;
    FlushList flushList_;
    bool rescheduled_{false};
    size_t pendingFlushes_{0};
    struct FlushPolicy {
      FlushPolicy(size_t m, std::chrono::microseconds f)
          : maxPendingFlushes(m), maxFlushLatency(f) {}
      size_t maxPendingFlushes{0};
      std::chrono::microseconds maxFlushLatency{0};
    };
    std::optional<FlushPolicy> flushPolicy_;
  };

  void scheduleTimeout(
      folly::HHWheelTimer::Callback* callback,
      const std::chrono::milliseconds& timeout) {
    if (timeout != std::chrono::milliseconds::zero()) {
      evb_->timer().scheduleTimeout(callback, timeout);
    }
  }

  void addInteraction() { ++interactions_; }
  void terminateInteraction(int64_t id);

  // Request connection close and fail all the requests.
  void close(folly::exception_wrapper ex) noexcept;

  bool incMemoryUsage(uint32_t) { return true; }

  void decMemoryUsage(uint32_t) {}

  int32_t getServerVersion() const { return serverVersion_; }

  bool getServerZstdSupported() const { return serverZstdSupported_; }

  folly::EventBase* getEventBase() { return evb_; }

 private:
  enum class ConnectionState : uint8_t {
    CONNECTED, // New requests are allowed
    CLOSING, // New requests are not allowed, there are active requests or close
             // callback is scheduled.
    CLOSED, // New requests are not allowed, there are no active requests.
    ERROR, // New requests are not allowed, error is stored in error_, close
           // callback is scheduled.
  };

  struct ClientState {
    ConnectionState connState : 2;
    bool hitMaxStreamId : 1;
    bool hasPendingSetupFrame : 1;

    ClientState() {
      // Client must be constructed with an already open socket
      connState = ConnectionState::CONNECTED;
      hitMaxStreamId = false;
      hasPendingSetupFrame = true;
    }
  } clientState_;
  int16_t serverVersion_{-1};
  bool serverZstdSupported_{false};
  StreamId nextStreamId_{1};

  // Client requests + internal requests (requestN, cancel, etc).
  uint32_t requests_{0};
  // Client facing requests (singleRequest(Single|No)Response)
  uint32_t clientRequests_{0};

  struct ServerCallbackUniquePtr {
    explicit ServerCallbackUniquePtr(
        std::unique_ptr<RocketStreamServerCallback> ptr) noexcept
        : storage_(
              reinterpret_cast<intptr_t>(ptr.release()) |
              static_cast<intptr_t>(CallbackType::STREAM)) {}
    explicit ServerCallbackUniquePtr(
        std::unique_ptr<RocketStreamServerCallbackWithChunkTimeout>
            ptr) noexcept
        : storage_(
              reinterpret_cast<intptr_t>(ptr.release()) |
              static_cast<intptr_t>(CallbackType::STREAM_WITH_CHUNK_TIMEOUT)) {}
    explicit ServerCallbackUniquePtr(
        std::unique_ptr<RocketSinkServerCallback> ptr) noexcept
        : storage_(
              reinterpret_cast<intptr_t>(ptr.release()) |
              static_cast<intptr_t>(CallbackType::SINK)) {}

    ServerCallbackUniquePtr(ServerCallbackUniquePtr&& other) noexcept
        : storage_(std::exchange(other.storage_, 0)) {}

    template <typename F>
    auto match(F&& f) const {
      switch (static_cast<CallbackType>(storage_ & kTypeMask)) {
        case CallbackType::STREAM:
          return f(reinterpret_cast<RocketStreamServerCallback*>(
              storage_ & kPointerMask));
        case CallbackType::STREAM_WITH_CHUNK_TIMEOUT:
          return f(
              reinterpret_cast<RocketStreamServerCallbackWithChunkTimeout*>(
                  storage_ & kPointerMask));
        case CallbackType::SINK:
          return f(reinterpret_cast<RocketSinkServerCallback*>(
              storage_ & kPointerMask));
        default:
          folly::assume_unreachable();
      };
    }

    ServerCallbackUniquePtr& operator=(
        ServerCallbackUniquePtr&& other) noexcept {
      match([](auto* ptr) { delete ptr; });
      storage_ = std::exchange(other.storage_, 0);

      return *this;
    }

    ~ServerCallbackUniquePtr() {
      match([](auto* ptr) { delete ptr; });
    }

   private:
    enum class CallbackType {
      STREAM,
      STREAM_WITH_CHUNK_TIMEOUT,
      SINK,
    };

    static constexpr intptr_t kTypeMask = 3;
    static constexpr intptr_t kPointerMask = ~kTypeMask;

    intptr_t storage_;
  };
  struct StreamMapHasher : private folly::f14::DefaultHasher<StreamId> {
    template <typename K>
    size_t operator()(const K& key) const {
      StreamIdResolver resolver;
      return folly::f14::DefaultHasher<StreamId>::operator()(resolver(key));
    }
  };
  struct StreamMapEquals {
    template <typename A, typename B>
    bool operator()(const A& a, const B& b) const {
      StreamIdResolver resolver;
      return resolver(a) == resolver(b);
    }
  };

  using StreamMap = folly::F14FastSet<
      ServerCallbackUniquePtr,
      folly::transparent<StreamMapHasher>,
      folly::transparent<StreamMapEquals>>;
  StreamMap streams_;
  folly::EventBase* evb_;
  RequestContextQueue queue_;

  class WriteLoopCallback : public folly::EventBase::LoopCallback {
   public:
    explicit WriteLoopCallback(RocketClient& client) : client_(client) {}
    ~WriteLoopCallback() override = default;
    void runLoopCallback() noexcept override;

   private:
    RocketClient& client_;
  };
  WriteLoopCallback writeLoopCallback_;
  void scheduleWriteLoopCallback();
  FlushList* flushList_{nullptr};

  FlushManager* flushManager_{nullptr};
  static folly::EventBaseLocal<FlushManager>& getEventBaseLocal();

  folly::AsyncTransport::UniquePtr socket_;
  folly::Function<void()> onDetachable_;
  folly::exception_wrapper error_;

  class FirstResponseTimeout : public folly::HHWheelTimer::Callback {
   public:
    FirstResponseTimeout(RocketClient& client, StreamId streamId)
        : client_(client), streamId_(streamId) {}

    void timeoutExpired() noexcept override;

   private:
    RocketClient& client_;
    RocketClient::DestructorGuard clientDg_{&client_};
    StreamId streamId_;
  };

  struct StreamIdResolver {
    StreamId operator()(StreamId streamId) const { return streamId; }

    StreamId operator()(const ServerCallbackUniquePtr& callbackVariant) const {
      return callbackVariant.match(
          [](const auto* callback) { return callback->streamId(); });
    }
  };

  folly::F14FastMap<StreamId, Payload> bufferedFragments_;
  using FirstResponseTimeoutMap =
      folly::F14FastMap<StreamId, std::unique_ptr<FirstResponseTimeout>>;
  FirstResponseTimeoutMap firstResponseTimeouts_;

  Parser<RocketClient> parser_;

  class DetachableLoopCallback : public folly::EventBase::LoopCallback {
   public:
    explicit DetachableLoopCallback(RocketClient& client) : client_(client) {}
    void runLoopCallback() noexcept override;

   private:
    RocketClient& client_;
  };
  DetachableLoopCallback detachableLoopCallback_;
  class CloseLoopCallback : public folly::EventBase::LoopCallback {
   public:
    explicit CloseLoopCallback(RocketClient& client) : client_(client) {}
    void runLoopCallback() noexcept override;

   private:
    RocketClient& client_;
  };
  CloseLoopCallback closeLoopCallback_;
  class OnEventBaseDestructionCallback
      : public folly::EventBase::OnDestructionCallback {
   public:
    explicit OnEventBaseDestructionCallback(RocketClient& client)
        : client_(client) {}

    void onEventBaseDestruction() noexcept override final;

   private:
    RocketClient& client_;
  };
  OnEventBaseDestructionCallback eventBaseDestructionCallback_;
  folly::Function<void()> closeCallback_;

  size_t interactions_{0};
  std::unique_ptr<SetupFrame> setupFrame_;

 protected:
  RocketClient(
      folly::EventBase& evb,
      folly::AsyncTransport::UniquePtr socket,
      std::unique_ptr<SetupFrame> setupFrame);

 private:
  template <class OnError>
  class SendFrameContext;

  template <typename Frame, typename OnError>
  FOLLY_NODISCARD bool sendFrame(Frame&& frame, OnError&& onError);

  template <typename InitFunc, typename OnError>
  FOLLY_NODISCARD bool sendVersionDependentFrame(
      InitFunc&& initFunc, StreamId streamId, OnError&& onError);

  FOLLY_NODISCARD folly::Try<void> scheduleWrite(RequestContext& ctx);

  StreamId makeStreamId();

  template <typename ServerCallback>
  void sendRequestStreamChannel(
      const StreamId& streamId,
      Payload&& request,
      std::chrono::milliseconds firstResponseTimeout,
      int32_t initialRequestN,
      std::unique_ptr<ServerCallback> serverCallback);

  void freeStream(StreamId streamId);

  void handleRequestResponseFrame(
      RequestContext& ctx,
      FrameType frameType,
      std::unique_ptr<folly::IOBuf> frame);
  void handleStreamChannelFrame(
      StreamId streamId,
      FrameType frameType,
      std::unique_ptr<folly::IOBuf> frame);

  template <typename CallbackType>
  StreamChannelStatusResponse handlePayloadFrame(
      CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame);

  template <typename CallbackType>
  StreamChannelStatusResponse handleFirstResponse(
      CallbackType& serverCallback,
      Payload&& fullPayload,
      bool next,
      bool complete);

  template <typename CallbackType>
  StreamChannelStatusResponse handleStreamResponse(
      CallbackType& serverCallback,
      Payload&& fullPayload,
      bool next,
      bool complete);

  StreamChannelStatusResponse handleSinkResponse(
      RocketSinkServerCallback& serverCallback,
      Payload&& fullPayload,
      bool next,
      bool complete);

  template <typename CallbackType>
  StreamChannelStatusResponse handleErrorFrame(
      CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame);

  template <typename CallbackType>
  StreamChannelStatusResponse handleRequestNFrame(
      CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame);

  template <typename CallbackType>
  StreamChannelStatusResponse handleCancelFrame(
      CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame);

  template <typename CallbackType>
  StreamChannelStatusResponse handleExtFrame(
      CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame);

  void handleError(RocketException&& rex);

  void writeScheduledRequestsToSocket() noexcept;

  void maybeScheduleFirstResponseTimeout(
      StreamId streamId, std::chrono::milliseconds timeout);
  folly::Optional<Payload> bufferOrGetFullPayload(PayloadFrame&& payloadFrame);

  bool isFirstResponse(StreamId streamId) const;
  void acknowledgeFirstResponse(StreamId);

  enum class RequestType { INTERNAL, CLIENT };

  FOLLY_NODISCARD auto makeRequestCountGuard(RequestType type) {
    ++requests_;
    if (type == RequestType::CLIENT) {
      ++clientRequests_;
    }
    return folly::makeGuard([this, type] {
      if (type == RequestType::CLIENT) {
        --clientRequests_;
      }
      if (!--requests_) {
        notifyIfDetachable();
      }
    });
  }

  class ServerVersionTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit ServerVersionTimeout(RocketClient& client) : client_(client) {}

    void timeoutExpired() noexcept override {
      client_.close(transport::TTransportException(
          apache::thrift::transport::TTransportException::TIMED_OUT,
          "Server version not reported"));
    }

   private:
    RocketClient& client_;
  };
  std::unique_ptr<ServerVersionTimeout> serverVersionTimeout_;
  void onServerVersionRequired();
  void setServerVersion(int32_t serverVersion);
  void sendTransportMetadataPush();

 protected:
  // Close the connection and fail all the requests *inline*. This should not be
  // called inline from any of the callbacks triggered by RocketClient.
  void closeNow(apache::thrift::transport::TTransportException ex) noexcept;

 private:
  bool setError(apache::thrift::transport::TTransportException ex) noexcept;
  void closeNowImpl() noexcept;
  std::unique_ptr<SetupFrame> moveOutSetupFrame();

  template <class T>
  friend class Parser;
};

} // namespace rocket
} // namespace thrift
} // namespace apache
