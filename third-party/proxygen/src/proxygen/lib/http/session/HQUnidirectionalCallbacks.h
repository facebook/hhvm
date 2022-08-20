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
class HQUnidirStreamDispatcher : public quic::QuicSocket::PeekCallback {
 public:
  // Receiver interface for the dispatched callbacks
  struct Callback {
    using ReadError = quic::QuicError;
    // Avoid pulling dependent names into ostensibly innocent templates...
    using PeekData = folly::Range<quic::QuicSocket::PeekIterator>;

    // Called by the dispatcher when a correct *peek* callback is identified
    // for the stream id.
    virtual void assignPeekCallback(
        quic::StreamId /* id */,
        hq::UnidirectionalStreamType /* type */,
        size_t /* to consume */,
        quic::QuicSocket::PeekCallback* const /* new callback */) = 0;

    // Called by the dispatcher when a correct *read* callback is identified
    // for the stream id.
    virtual void assignReadCallback(
        quic::StreamId /* id */,
        hq::UnidirectionalStreamType /* type */,
        size_t /* to consume */,
        quic::QuicSocket::ReadCallback* const /* new callback */) = 0;

    // Called by the dispatcher when a push stream is identified by
    // the dispatcher.
    virtual void onNewPushStream(quic::StreamId /* streamId */,
                                 hq::PushId /* pushId */,
                                 size_t /* to consume */) = 0;

    // Called by the dispatcher when a stream can not be recognized
    virtual void rejectStream(quic::StreamId /* id */) = 0;

    // Called by the dispatcher to identify a stream preface
    virtual folly::Optional<hq::UnidirectionalStreamType> parseStreamPreface(
        uint64_t preface) = 0;

    // Data availalbe for read on the control stream
    virtual void controlStreamReadAvailable(quic::StreamId /* id */) = 0;

    // Error on the control stream
    virtual void controlStreamReadError(quic::StreamId /* id */,
                                        const ReadError& /* error */) = 0;

   protected:
    virtual ~Callback() = default;
  }; // Callback

  explicit HQUnidirStreamDispatcher(Callback& sink,
                                    proxygen::TransportDirection direction);

  virtual ~HQUnidirStreamDispatcher() override = default;

  virtual void onDataAvailable(
      quic::StreamId /* id */,
      const Callback::PeekData& /* data */) noexcept override;

  virtual void peekError(quic::StreamId /* id */, quic::QuicError
                         /* error */) noexcept override;

  quic::QuicSocket::ReadCallback* controlStreamCallback() const;

  // Take the temporary ownership of the stream.
  // The ownership is released when the stream is passed
  // to the sink
  void takeTemporaryOwnership(quic::StreamId id) {
    pendingStreams_.insert(id);
  }

  bool hasOwnership(quic::StreamId id) const {
    return pendingStreams_.count(id) > 0;
  }

  quic::StreamId releaseOwnership(quic::StreamId id) {
    DCHECK(hasOwnership(id))
        << "Can not release ownership on unowned streamID=" << id;
    bool found = pendingStreams_.erase(id);
    DCHECK(found) << "Inconstency detected streamID=" << id;
    return id;
  }

  size_t numberOfStreams() const {
    return pendingStreams_.size();
  }

  void invokeOnPendingStreamIDs(std::function<void(quic::StreamId)> fn) {
    for (auto& pendingStream : pendingStreams_) {
      fn(pendingStream);
    }
  }

 private:
  // Callback for the control stream - follows the read api
  struct ControlCallback : public quic::QuicSocket::ReadCallback {
    using Dispatcher = HQUnidirStreamDispatcher;
    explicit ControlCallback(Dispatcher::Callback& sink) : sink_(sink) {
    }
    virtual ~ControlCallback() override = default;
    void readAvailable(quic::StreamId id) noexcept override;
    void readError(quic::StreamId id,
                   Dispatcher::Callback::ReadError error) noexcept override;

   protected:
    Dispatcher::Callback& sink_;
  };

  std::unique_ptr<ControlCallback> controlStreamCallback_;
  Callback& sink_;
  std::unordered_set<quic::StreamId> pendingStreams_;
  proxygen::TransportDirection direction_;
};
} // namespace proxygen
