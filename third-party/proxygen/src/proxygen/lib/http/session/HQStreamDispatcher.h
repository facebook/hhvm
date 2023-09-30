/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <quic/api/QuicSocket.h>

#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>

namespace proxygen {

// Base class for the unidirectional stream callbacks
// holds the session and defines the shared error messages
class HQStreamDispatcherBase : public quic::QuicSocket::PeekCallback {
 public:
  // Receiver interface for the dispatched callbacks
  struct CallbackBase {
    using ReadError = quic::QuicError;
    // Avoid pulling dependent names into ostensibly innocent templates...
    using PeekData = folly::Range<quic::QuicSocket::PeekIterator>;

    [[nodiscard]] virtual folly::EventBase* getEventBase() const = 0;

    [[nodiscard]] virtual std::chrono::milliseconds getDispatchTimeout()
        const = 0;

    // Called by the dispatcher when a stream can not be recognized
    virtual void rejectStream(quic::StreamId /* id */) = 0;

   protected:
    virtual ~CallbackBase() = default;
  }; // Callback

  explicit HQStreamDispatcherBase(CallbackBase& callback,
                                  proxygen::TransportDirection direction);

  ~HQStreamDispatcherBase() override = default;

  void onDataAvailable(
      quic::StreamId /* id */,
      const CallbackBase::PeekData& /* data */) noexcept override;

  void peekError(quic::StreamId /* id */, quic::QuicError
                 /* error */) noexcept override;

  // Take the temporary ownership of the stream.
  // The ownership is released when the stream is passed
  // to the callback
  void takeTemporaryOwnership(quic::StreamId id) {
    auto res = pendingStreams_.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(id),
                                       std::forward_as_tuple(*this, id));
    callback_.getEventBase()->timer().scheduleTimeout(
        &res.first->second, callback_.getDispatchTimeout());
  }

  bool hasOwnership(quic::StreamId id) const {
    return pendingStreams_.count(id) > 0;
  }

  quic::StreamId releaseOwnership(quic::StreamId id) {
    LOG_IF(DFATAL, !hasOwnership(id))
        << "Can not release ownership on unowned streamID=" << id;
    bool found = pendingStreams_.erase(id);
    LOG_IF(DFATAL, !found) << "Inconstency detected streamID=" << id;
    return id;
  }

  size_t numberOfStreams() const {
    return pendingStreams_.size();
  }

  void invokeOnPendingStreamIDs(const std::function<void(quic::StreamId)>& fn) {
    for (auto& pendingStream : pendingStreams_) {
      fn(pendingStream.first);
    }
  }

  void dispatchTimeoutExpired(quic::StreamId id) {
    callback_.rejectStream(releaseOwnership(id));
  }

  void cleanup() {
    for (auto& pendingStream : pendingStreams_) {
      callback_.rejectStream(pendingStream.first);
    }
    pendingStreams_.clear();
  }

 private:
  class DispatchTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit DispatchTimeout(HQStreamDispatcherBase& dispatcher,
                             quic::StreamId id)
        : dispatcher_(dispatcher), id_(id) {
    }

    ~DispatchTimeout() override = default;
    void timeoutExpired() noexcept override {
      dispatcher_.dispatchTimeoutExpired(id_);
    }

   private:
    HQStreamDispatcherBase& dispatcher_;
    quic::StreamId id_;
  };

  std::unordered_map<quic::StreamId, DispatchTimeout> pendingStreams_;

 protected:
  enum class HandleStreamResult { DISPATCHED, REJECT, PENDING };
  virtual HandleStreamResult handleStream(quic::StreamId id,
                                          folly::io::Cursor& cursor,
                                          uint64_t preface,
                                          size_t consumed) = 0;

  CallbackBase& callback_;
  proxygen::TransportDirection direction_;
};

class HQUniStreamDispatcher : public HQStreamDispatcherBase {
 public:
  struct Callback : public HQStreamDispatcherBase::CallbackBase {
    ~Callback() override = default;

    // Called by the dispatcher to identify a stream preface
    virtual folly::Optional<hq::UnidirectionalStreamType> parseUniStreamPreface(
        uint64_t preface) = 0;

    // Called by the dispatcher when a correct *peek* callback is identified
    // for the stream id.
    virtual void dispatchControlStream(quic::StreamId /* id */,
                                       hq::UnidirectionalStreamType /* type */,
                                       size_t /* to consume */) = 0;

    // Called by the dispatcher when a push stream is identified by
    // the dispatcher.
    virtual void dispatchPushStream(quic::StreamId /* streamId */,
                                    hq::PushId /* pushId */,
                                    size_t /* to consume */) = 0;
    virtual void dispatchUniWTStream(quic::StreamId /* streamId */,
                                     quic::StreamId /* sessionId */,
                                     size_t /* to consume */) = 0;
  };

  HQUniStreamDispatcher(Callback& callback,
                        proxygen::TransportDirection direction)
      : HQStreamDispatcherBase(callback, direction), callback_(callback) {
  }

 private:
  HandleStreamResult handleStream(quic::StreamId id,
                                  folly::io::Cursor& cursor,
                                  uint64_t preface,
                                  size_t consumed) override;

  Callback& callback_;
};

class HQBidiStreamDispatcher : public HQStreamDispatcherBase {
 public:
  struct Callback : public HQStreamDispatcherBase::CallbackBase {
    ~Callback() override = default;

    // Called by the dispatcher to identify a stream preface
    virtual folly::Optional<hq::BidirectionalStreamType> parseBidiStreamPreface(
        uint64_t preface) = 0;

    virtual void dispatchRequestStream(quic::StreamId /* streamId */) = 0;

    virtual void dispatchBidiWTStream(quic::StreamId /* streamId */,
                                      quic::StreamId /* sessionId */,
                                      size_t /* to consume */) = 0;
  };

  HQBidiStreamDispatcher(Callback& callback,
                         proxygen::TransportDirection direction)
      : HQStreamDispatcherBase(callback, direction), callback_(callback) {
  }

 private:
  HandleStreamResult handleStream(quic::StreamId id,
                                  folly::io::Cursor& cursor,
                                  uint64_t preface,
                                  size_t consumed) override;
  Callback& callback_;
};

} // namespace proxygen
