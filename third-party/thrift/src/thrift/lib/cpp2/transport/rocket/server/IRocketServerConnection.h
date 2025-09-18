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

#include <folly/ObserverContainer.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/core/ManagedConnectionIf.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>

THRIFT_FLAG_DECLARE_bool(enable_rocket_connection_observers);

namespace apache::thrift {

class Cpp2ConnContext;

namespace rocket {

class IRocketServerConnection;
class RocketServerConnection;
class RocketBiDiClientCallback;
class RocketSinkClientCallback;
class RocketStreamClientCallback;

using ClientCallbackUniquePtr = std::variant<
    std::unique_ptr<RocketStreamClientCallback>,
    std::unique_ptr<RocketSinkClientCallback>,
    std::unique_ptr<RocketBiDiClientCallback>>;
using ClientCallbackPtr = std::variant<
    RocketStreamClientCallback*,
    RocketSinkClientCallback*,
    RocketBiDiClientCallback*>;

// Forward declare the factory method implementation
struct ChannelRequestCallbackFactory {
  ChannelRequestCallbackFactory(
      ClientCallbackUniquePtr& callback,
      StreamId streamId,
      IRocketServerConnection& connection,
      uint32_t initialRequestN);

  template <typename T>
  T* create();

 private:
  ClientCallbackUniquePtr* callback_;
  StreamId streamId_;
  IRocketServerConnection* connection_;
  uint32_t initialRequestN_;
};

class IRocketServerConnection : public ManagedConnectionIf,
                                public folly::AsyncTransport::WriteCallback,
                                public folly::AsyncTransport::BufferCallback {
 public:
  using IRocketServerConnectionObserverContainer = folly::ObserverContainer<
      RocketServerConnectionObserver,
      IRocketServerConnection,
      folly::ObserverContainerBasePolicyDefault<
          RocketServerConnectionObserver::Events /* EventEnum */,
          32 /* BitsetSize (max number of interface events) */>>;

  // (configuration parameters mostly inherited from the server)
  struct Config {
    Config() {}

    std::chrono::milliseconds socketWriteTimeout{
        std::chrono::milliseconds::zero()};
    std::chrono::milliseconds streamStarvationTimeout{std::chrono::seconds{60}};
    std::chrono::milliseconds writeBatchingInterval{
        std::chrono::milliseconds::zero()};
    size_t writeBatchingSize{0};
    size_t writeBatchingByteSize{0};
    size_t egressBufferBackpressureThreshold{0};
    double egressBufferBackpressureRecoveryFactor{0.0};
    const folly::SocketOptionMap* socketOptions{nullptr};
    std::shared_ptr<rocket::ParserAllocatorType> parserAllocator{nullptr};
  };

  IRocketServerConnection()
      : enableObservers_(THRIFT_FLAG(enable_rocket_connection_observers)),
        observerContainer_(this) {}
  ~IRocketServerConnection() = default;

  class ReadResumableHandle {
   public:
    explicit ReadResumableHandle(IRocketServerConnection* connection);
    ~ReadResumableHandle();

    ReadResumableHandle(ReadResumableHandle&&) noexcept;
    ReadResumableHandle(const ReadResumableHandle&) = delete;
    ReadResumableHandle& operator=(const ReadResumableHandle&) = delete;
    ReadResumableHandle& operator=(ReadResumableHandle&&) = delete;

    void resume() &&;
    folly::EventBase& getEventBase() { return connection_->getEventBase(); }

    Cpp2ConnContext* getCpp2ConnContext() {
      return connection_->getCpp2ConnContext();
    }

   private:
    IRocketServerConnection* connection_;
  };

  class ReadPausableHandle {
   public:
    explicit ReadPausableHandle(IRocketServerConnection* connection);
    ~ReadPausableHandle();

    ReadPausableHandle(ReadPausableHandle&&) noexcept;
    ReadPausableHandle(const ReadPausableHandle&) = delete;
    ReadPausableHandle& operator=(const ReadPausableHandle&) = delete;
    ReadPausableHandle& operator=(ReadPausableHandle&&) = delete;

    ReadResumableHandle pause() &&;
    folly::EventBase& getEventBase() { return connection_->getEventBase(); }

    Cpp2ConnContext* getCpp2ConnContext() {
      return connection_->getCpp2ConnContext();
    }

   private:
    IRocketServerConnection* connection_;
  };

  virtual void send(
      std::unique_ptr<folly::IOBuf> data,
      MessageChannel::SendCallbackPtr cb = nullptr,
      StreamId streamId = StreamId(),
      folly::SocketFds fds = folly::SocketFds{}) = 0;

  virtual void sendErrorAfterDrain(
      StreamId streamId, RocketException&& rex) = 0;

  // does not create callback and returns nullptr if streamId is already in use
  virtual RocketStreamClientCallback* FOLLY_NULLABLE createStreamClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
      uint32_t initialRequestN) = 0;

  // does not create callback and returns nullptr if streamId is already in use
  virtual RocketSinkClientCallback* FOLLY_NULLABLE createSinkClientCallback(
      StreamId streamId, IRocketServerConnection& connection) = 0;

  // does not create callback and returns nullptr if streamId is already in use
  virtual std::optional<ChannelRequestCallbackFactory>
  createChannelClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
      uint32_t initialRequestN) = 0;

  // Parser callbacks
  virtual void handleFrame(std::unique_ptr<folly::IOBuf> frame) = 0;
  virtual void close(folly::exception_wrapper ew) = 0;

  // AsyncTransport::WriteCallback implementation
  virtual void writeStarting() noexcept = 0;
  virtual void writeSuccess() noexcept = 0;
  virtual void writeErr(
      size_t bytesWritten, const folly::AsyncSocketException& ex) noexcept = 0;

  // AsyncTransport::BufferCallback implementation
  virtual void onEgressBuffered() = 0;
  virtual void onEgressBufferCleared() = 0;

  virtual folly::EventBase& getEventBase() const = 0;
  virtual size_t getNumStreams() const = 0;

  virtual void sendPayload(
      StreamId streamId,
      Payload&& payload,
      Flags flags,
      apache::thrift::MessageChannel::SendCallbackPtr cb = nullptr) = 0;
  virtual void sendError(
      StreamId streamId,
      RocketException&& rex,
      apache::thrift::MessageChannel::SendCallbackPtr cb = nullptr) = 0;
  virtual void sendRequestN(StreamId streamId, int32_t n) = 0;
  virtual void sendCancel(StreamId streamId) = 0;
  virtual void sendMetadataPush(std::unique_ptr<folly::IOBuf> metadata) = 0;

  virtual void freeStream(StreamId streamId, bool markRequestComplete) = 0;

  virtual void scheduleStreamTimeout(folly::HHWheelTimer::Callback*) = 0;
  virtual void scheduleSinkTimeout(
      folly::HHWheelTimer::Callback*, std::chrono::milliseconds timeout) = 0;

  virtual void incInflightFinalResponse() = 0;
  virtual void decInflightFinalResponse() = 0;

  virtual void applyQosMarking(const RequestSetupMetadata& setupMetadata) = 0;

  virtual Cpp2ConnContext* getCpp2ConnContext() = 0;

  virtual void setOnWriteQuiescenceCallback(
      folly::Function<void(ReadPausableHandle)> cb) = 0;

  virtual bool isDecodingMetadataUsingBinaryProtocol() = 0;

  virtual bool incMemoryUsage(uint32_t memSize) = 0;

  virtual void decMemoryUsage(uint32_t memSize) = 0;

  virtual int32_t getVersion() const = 0;

  virtual bool areStreamsPaused() const noexcept = 0;

  virtual size_t getNumActiveRequests() const override = 0;

  virtual size_t getNumPendingWrites() const override = 0;

  virtual const folly::SocketAddress& getPeerAddress()
      const noexcept override = 0;

  virtual folly::AsyncSocket* getRawSocket() const = 0;

  using Observer = IRocketServerConnectionObserverContainer::Observer;
  using ManagedObserver =
      IRocketServerConnectionObserverContainer::ManagedObserver;

  /**
   * Adds an observer.
   *
   * If the observer is already added, this is a no-op.
   *
   * @param observer     Observer to add.
   * @return             Whether the observer was added (fails if no list).
   */
  bool addObserver(Observer* observer) {
    if (auto list = getObserverContainer()) {
      list->addObserver(observer);
      return true;
    }
    return false;
  }

  /**
   * Removes an observer.
   *
   * @param observer     Observer to remove.
   * @return             Whether the observer was found and removed.
   */
  bool removeObserver(Observer* observer) {
    if (auto list = getObserverContainer()) {
      return list->removeObserver(observer);
    }
    return false;
  }

  /**
   * Get the number of observers.
   *
   * @return             Number of observers.
   */
  size_t numObservers() const {
    if (auto list = getObserverContainer()) {
      return list->numObservers();
    }
    return 0;
  }

  /**
   * Returns list of attached observers that are of type T.
   *
   * @return             Attached observers of type T.
   */
  template <typename T = Observer>
  std::vector<T*> findObservers() const {
    if (auto list = getObserverContainer()) {
      return list->findObservers<T>();
    }
    return {};
  }

  IRocketServerConnectionObserverContainer* getObserverContainer() const {
    if (enableObservers_) {
      return const_cast<IRocketServerConnectionObserverContainer*>(
          &observerContainer_);
    }
    return nullptr;
  }

  virtual PayloadSerializer::Ptr getPayloadSerializer() = 0;

  virtual void applyCustomCompression(
      std::shared_ptr<CustomCompressor> compressor) = 0;

 protected:
  const bool enableObservers_;
  IRocketServerConnectionObserverContainer observerContainer_;

 private:
  friend class ReadPausableHandle;
  friend class ReadResumableHandle;

  virtual void closeIfNeeded() = 0;
  virtual void incrementActivePauseHandlers() = 0;
  virtual void decrementActivePauseHandlers() = 0;
  virtual void tryResumeSocketReading() = 0;
  virtual void pauseSocketReading() = 0;
};

inline IRocketServerConnection::ReadResumableHandle::ReadResumableHandle(
    IRocketServerConnection* connection)
    : connection_(connection) {}

inline IRocketServerConnection::ReadResumableHandle::~ReadResumableHandle() {
  if (connection_ != nullptr) {
    std::move(*this).resume();
  }
}

inline void IRocketServerConnection::ReadResumableHandle::resume() && {
  DCHECK(connection_ != nullptr) << "resume() has been called on this handle";
  connection_->decrementActivePauseHandlers();
  connection_->tryResumeSocketReading();
  connection_->closeIfNeeded();
  connection_ = nullptr;
}

inline IRocketServerConnection::ReadResumableHandle::ReadResumableHandle(
    ReadResumableHandle&& handle) noexcept
    : connection_(std::exchange(handle.connection_, nullptr)) {}

inline IRocketServerConnection::ReadPausableHandle::ReadPausableHandle(
    IRocketServerConnection* connection)
    : connection_(connection) {
  DCHECK(connection_ != nullptr);
  connection_->incrementActivePauseHandlers();
}

inline IRocketServerConnection::ReadPausableHandle::~ReadPausableHandle() {
  if (connection_ != nullptr) {
    connection_->decrementActivePauseHandlers();
    connection_->closeIfNeeded();
  }
}

inline IRocketServerConnection::ReadPausableHandle::ReadPausableHandle(
    ReadPausableHandle&& handle) noexcept
    : connection_(std::exchange(handle.connection_, nullptr)) {}

inline IRocketServerConnection::ReadResumableHandle
IRocketServerConnection::ReadPausableHandle::pause() && {
  DCHECK(connection_ != nullptr) << "pause() has been called on this handle";
  connection_->pauseSocketReading();
  return ReadResumableHandle(std::exchange(connection_, nullptr));
}

} // namespace rocket

} // namespace apache::thrift
