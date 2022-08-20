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

#include <memory>
#include <utility>

#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <wangle/acceptor/AcceptorHandshakeManager.h>

namespace wangle {
/**
 * Wraps an existing AcceptorHandshakeHelper and callback. Performs handshakes
 * under a specified handshake eventbase and delivers callback notifications
 * in the original EventBase.
 *
 * An example use case of this class would be to offload TLS handshakes under
 * a separate EventBase thread rather than the Acceptor thread.
 */
class EvbHandshakeHelper : public AcceptorHandshakeHelper,
                           public AcceptorHandshakeHelper::Callback {
 public:
  explicit EvbHandshakeHelper(
      AcceptorHandshakeHelper::UniquePtr helper,
      folly::EventBase* handshakeEvb)
      : helper_(std::move(helper)),
        originalEvb_(nullptr),
        handshakeEvb_(handshakeEvb) {}

  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      AcceptorHandshakeHelper::Callback* callback) noexcept override;

  void dropConnection(SSLErrorEnum reason = SSLErrorEnum::NO_ERROR) override;

  // Exposing these for tests, do not directly call these callbacks.
  virtual void connectionReady(
      folly::AsyncTransport::UniquePtr transport,
      std::string nextProtocol,
      SecureTransportType secureTransportType,
      folly::Optional<SSLErrorEnum> sslErr) noexcept override;

  virtual void connectionError(
      folly::AsyncTransport* transport,
      folly::exception_wrapper ex,
      folly::Optional<SSLErrorEnum> sslErr) noexcept override;

 protected:
  enum class HandshakeState : unsigned {
    Invalid,
    Started,
    Dropped,
    Callback,
  };

  ~EvbHandshakeHelper();

  AcceptorHandshakeHelper::UniquePtr helper_;
  AcceptorHandshakeHelper::Callback* callback_;

  folly::EventBase* originalEvb_;
  folly::EventBase* handshakeEvb_;

  // state_ is used to prevent dropConnection(), connectionReady(),
  // and connectionError() from being called at the same time.
  //
  // State machine diagram:
  //                                dropConnection()
  //                               .--------> Dropped
  //          start()             /
  // Invalid  ------> Started ---+
  //                              \
  //                               `--------> Callback
  //                               connectionReady()
  //                               connectionError()
  //
  //
  // We primarily rely on the atomic updating of state_ to prevent the following
  // dangerous scenario:
  //
  // ---------------D--C'------------   Acceptor Thread (originalEvb_)
  //                 \/
  //                 /\
  // ---------------C--D'------------   Handshake Thread (handshakeEvb_)
  //
  // Notation:
  //  D is the EvbHandshakeHelper::dropConnection() call
  //  D' is the helper->dropConnection() call scheduled by D to run on the
  //     handshake thread.
  //  C is an incoming connectionReady()/connectionError() callback
  //  C' is the call to callback_->connectionReady()/connectionError() on the
  //     acceptor thread.
  //
  // We explicitly disallow this scenario: if this scenario were to happen, then
  // D' would call helper->dropConnection() on the handshake thread when the
  // underlying transport's eventbase is no longer attached to the handshake
  // thread.
  //
  // Because dropConnection(), connectionReady()/connectionError() will race
  // to update this atomic with cmpxchg; only one path will "win". So, there
  // only TWO cases to consider.
  //
  // (Remember that in all of these cases, the handshake thread is invisible to
  // an observer. As far as wangle::Acceptor is concerned, there is only one
  // acceptor thread. This means that from an observer's point of view, as long
  // as some event has not occured on the originalEvb_ thread, then that event
  // has never occured at all. (Trees falling in forests when the observer
  // doesn't live in the forest))
  //
  // # Case 1: D and C race, and D wins:
  //
  // --------------D-----------------   Acceptor Thread (originalEvb_)
  //                 \
  //                  \
  // ---------------C--D'------------   Handshake Thread (handshakeEvb_)
  //
  // From an observer's point of view, dropConnection() is the only call that
  // happens on the acceptor thread; C occurs "later" than D (w.r.t updating
  // the atomic state), so C will never schedule anything to run on
  // the acceptor thread.
  //
  //
  // # Case 2: D and C race; C wins, but D comes first on the acceptor thread
  //
  // -----------------D-C'-----------   Acceptor Thread (originalEvb_)
  //                   /
  //                  /
  // ---------------C----------------   Handshake Thread (handshakeEvb_)
  //
  // In this case, C won the race to update the state, so it schedules a
  // callback to occur on the acceptor thread. HOWEVER, before that callback
  // actually runs on the acceptor thread, the observer calls dropConnection().
  //
  // From the observer's point of view, at the time it made the dropConnection()
  // call, it had never seen a callback before.
  //
  // Therefore, in this case, in order to ensure that the state of the world
  // is consistent with the observer, we need to ensure that when C' actually
  // runs, *it doesn't fire the callback*.
  //
  // So, D realizes it lost the race against C for updating the state.
  // Therefore, D will not schedule anything to run in the handshake thread.
  // HOWEVER, it will set dropConnectionGuard_, which will serve as an
  // indicator to C' that dropConnection() was called first, so C' should not
  // run.
  //
  // These are the only two valid cases that can happen. In a sense, state_'s
  // primary purpose is to ensure that only one path can mutate the transport's
  // eventbase through attachEventBase() and detachEventBase().
  //
  // Here are some other states that we do not have to consider, for
  // completeness:
  //
  // # Invalid CASE 1: D comes after C'.
  //
  // -------------------C'-D---------   Acceptor Thread (originalEvb_)
  //                   /
  //                  /
  // ---------------C----------------   Handshake Thread (handshakeEvb_)
  //
  // What if we reverse the order of events on the acceptor thread in Case 2,
  // so that C' runs before D?
  //
  // This is an invalid case. When C' runs, it will notice that no
  // dropConnectionGuard_ was set, which means that from the point of view
  // of an "observer", C' occurs first, and D never happened.
  //
  // When C' runs, it will call the callback's connectionReady, which will
  // delete the entire helper. D cannot possibly run, because any upstream
  // callers will see that this helper doesn't exist.
  //
  //
  // # Invalid CASE 2: D occurs much earlier than C
  //
  // ---D----------------------------   Acceptor Thread (originalEvb_)
  //     \
  //      \
  // ------D'-------C----------------   Handshake Thread (handshakeEvb_)
  //
  // In this case, D will obviously be able to update the state_ to Dropped.
  // Therefore, it will be able to schedule D' to run on the handshake thread.
  //
  // When D' runs, the transport will be closed and cleaned up, so it is
  // impossible for a C to ever arrive on the handshake thread after D' (the
  // handshake callback will never fire!). Of course, in the process of running
  // D', the underlying handshake helper may synchronously call
  // connectionError. But since connectionError will fail to update the state_,
  // it does nothing and immediately returns.
  //
  std::atomic<HandshakeState> state_{HandshakeState::Invalid};

  // dropConnectionGuard_ is used to keep the EvbHandshakeHelper alive when
  // dropConnection() is called until after the underlying helper's
  // dropConnection is called.
  folly::Optional<folly::DelayedDestruction::DestructorGuard>
      dropConnectionGuard_;

 private:
  std::pair<bool, HandshakeState> tryTransition(
      HandshakeState expected,
      HandshakeState next);
};
} // namespace wangle
