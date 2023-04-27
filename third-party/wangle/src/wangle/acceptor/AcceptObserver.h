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

namespace folly {
class AsyncTransport;
}

namespace wangle {

class Acceptor;

/**
 * Observer of events related to connection acceptance.
 *
 * This observer can be combined with AsyncTransport::LifecycleObserver and
 * other observers to enable instrumentation to be installed when a connection
 * is accepted. For instance, a sampling algorithm can be executed in accept()
 * to sample and install instrumentation on a subset of connections.
 */
class AcceptObserver {
 public:
  virtual ~AcceptObserver() = default;

  /**
   * accept() is invoked after a connection has been accepted and an
   * AsyncTransport has been instantiated to manage the socket fd / connection.
   *
   * @param transport   Transport of accepted connection.
   */
  virtual void accept(folly::AsyncTransport* transport) noexcept = 0;

  /**
   * ready() is invoked after a connection has been accepted and SSL
   * handshakes (if any) have completed, right before onNewConnection is called.
   *
   * @param transport   Transport of ready connection.
   */
  virtual void ready(folly::AsyncTransport* transport) noexcept = 0;

  /**
   * acceptorDestroy() is invoked when the worker (acceptor) is destroyed.
   *
   * No further events will be invoked after acceptorDestroy().
   *
   * @param acceptor    Acceptor that was destroyed.
   */
  virtual void acceptorDestroy(Acceptor* acceptor) noexcept = 0;

  /**
   * observerAttached() is invoked when the observer is installed.
   *
   * @param acceptor    Acceptor that the observer is attached to.
   */
  virtual void observerAttach(Acceptor* acceptor) noexcept = 0;

  /**
   * observerDetached() is invoked if the observer is uninstalled prior to
   * worker (acceptor) destruction.
   *
   * No further events will be invoked after observerDetached().
   *
   * @param acceptor    Acceptor that the observer was removed from.
   */
  virtual void observerDetach(Acceptor* acceptor) noexcept = 0;
};

} // namespace wangle
