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

#include <wangle/acceptor/EvbHandshakeHelper.h>

namespace wangle {

void EvbHandshakeHelper::start(
    folly::AsyncSSLSocket::UniquePtr sock,
    AcceptorHandshakeHelper::Callback* callback) noexcept {
  auto transition =
      tryTransition(HandshakeState::Invalid, HandshakeState::Started);
  if (!transition.first) {
    VLOG(5) << "Ignoring call to start(), since state is currently "
            << static_cast<unsigned>(transition.second);
  }

  callback_ = callback;
  originalEvb_ = sock->getEventBase();
  CHECK(originalEvb_);

  sock->detachEventBase();
  originalEvb_->runInLoop(
      [this, sock = std::move(sock)]() mutable {
        handshakeEvb_->runInEventBaseThread(
            [this, sock = std::move(sock)]() mutable {
              sock->attachEventBase(handshakeEvb_);
              helper_->start(std::move(sock), this);
            });
      },
      /* thisIteration = */ true);
}

void EvbHandshakeHelper::dropConnection(SSLErrorEnum reason) {
  CHECK(originalEvb_);
  originalEvb_->dcheckIsInEventBaseThread();

  auto transition =
      tryTransition(HandshakeState::Started, HandshakeState::Dropped);

  // Regardless of whether or not we win the race or not, we will set
  // dropConnectionGuard_ (see case 2) to let a potential C' know that the
  // connection was dropped. C', seeing this, is responsible for clearing
  // the dropConnectionGuard.
  dropConnectionGuard_.emplace(this);
  callback_->connectionError(
      nullptr,
      folly::make_exception_wrapper<std::runtime_error>("connection dropped"),
      reason);

  if (transition.first) {
    // If we win the race to update the state, though, we are responsible
    // for ensuring that dropConnection() is called on the handshake thread,
    // and we are also the one responsible for clearing the dropConnectionGuard_
    // that we set earlier.
    handshakeEvb_->runInEventBaseThread([this, reason] {
      VLOG(5) << "callback has not been received. dropConnection "
              << "calling underlying helper";

      helper_->dropConnection(reason);

      // We need to ensure that the transport is destroyed in the handshake
      // thread since the transport is currently attached to the handshake's
      // event base.
      helper_.reset();

      originalEvb_->runInEventBaseThread(
          [this] { dropConnectionGuard_.reset(); });
    });
  }
}

void EvbHandshakeHelper::connectionReady(
    folly::AsyncTransport::UniquePtr transport,
    std::string nextProtocol,
    SecureTransportType secureTransportType,
    folly::Optional<SSLErrorEnum> sslErr) noexcept {
  DCHECK_EQ(transport->getEventBase(), handshakeEvb_);

  auto transition =
      tryTransition(HandshakeState::Started, HandshakeState::Callback);
  if (!transition.first) {
    VLOG(5) << "Ignoring call to connectionReady(), expected state to be "
            << static_cast<unsigned>(HandshakeState::Started)
            << " but actual state was "
            << static_cast<unsigned>(transition.second);
    return;
  }

  transport->detachEventBase();

  handshakeEvb_->runInLoop(
      [this,
       secureTransportType,
       sslErr,
       transport = std::move(transport),
       nextProtocol = std::move(nextProtocol)]() mutable {
        originalEvb_->runInEventBaseThread(
            [this,
             secureTransportType,
             sslErr,
             transport = std::move(transport),
             nextProtocol = std::move(nextProtocol)]() mutable {
              DCHECK(callback_);
              VLOG(5) << "calling underlying callback connectionReady";
              transport->attachEventBase(originalEvb_);

              // If a dropConnection call occured by the time this lambda runs,
              // we don't want to fire the callback. (See Case 2)
              if (dropConnectionGuard_.has_value()) {
                dropConnectionGuard_.reset();
                return;
              }

              callback_->connectionReady(
                  std::move(transport),
                  std::move(nextProtocol),
                  secureTransportType,
                  sslErr);
            });
      },
      /* thisIteration = */ true);
}

void EvbHandshakeHelper::connectionError(
    folly::AsyncTransport* transport,
    folly::exception_wrapper ex,
    folly::Optional<SSLErrorEnum> sslErr) noexcept {
  DCHECK(transport->getEventBase() == handshakeEvb_);

  auto transition =
      tryTransition(HandshakeState::Started, HandshakeState::Callback);
  if (!transition.first) {
    VLOG(5) << "Ignoring call to connectionError(), expected state to be "
            << static_cast<unsigned>(HandshakeState::Started)
            << " but actual state was "
            << static_cast<unsigned>(transition.second);
    return;
  }

  helper_.reset();
  originalEvb_->runInEventBaseThread(
      [this, sslErr, ex = std::move(ex)]() mutable {
        DCHECK(callback_);
        VLOG(5) << "calling underlying callback connectionError";

        // If a dropConnection call occured by the time this lambda runs, we
        // don't want to fire the callback. (See Case 2)
        if (dropConnectionGuard_.has_value()) {
          dropConnectionGuard_.reset();
          return;
        }
        callback_->connectionError(nullptr, std::move(ex), sslErr);
      });
}

std::pair<bool, EvbHandshakeHelper::HandshakeState>
EvbHandshakeHelper::tryTransition(
    HandshakeState expected,
    HandshakeState next) {
  bool res = state_.compare_exchange_strong(expected, next);
  return std::make_pair(res, expected);
}

EvbHandshakeHelper::~EvbHandshakeHelper() {
  VLOG(5) << "evbhandshakehelper is destroyed";
}
} // namespace wangle
