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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#include <folly/Executor.h>
#include <folly/SocketAddress.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>
#include <folly/observer/Observer.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionFactory.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionInstaller.h>
#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionListener.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/ConnectionAcceptCallbackHandler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/ConnectionBuilderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/ConnectionTLSHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>

namespace apache::thrift::fast_thrift::connection {

// Disambiguate from sibling apache::thrift::fast_thrift::connection::security
// namespace (the inner TLS pipeline). Bare `security::` resolves to the
// sibling inside this namespace.
namespace fast_security = ::apache::thrift::fast_thrift::security;

// Pipeline handler tags. Must be at namespace scope (HANDLER_TAG expands to
// an inline constexpr variable, which can't live in function bodies).
HANDLER_TAG(connection_tls);
HANDLER_TAG(connection_builder_handler);
HANDLER_TAG(connection_accept_callback_handler);

/**
 * ConnectionHandler — per-EventBase unit. Generic on the connection type:
 * setConnectionFactory<F>() is a templated setter that wires the acceptance
 * pipeline with a typed ConnectionBuilderHandler<F> + ConnectionInstaller<Conn>
 * (and an optional ConnectionAcceptCallbackHandler<Conn> middleware). The
 * resulting connections are stored type-erased so this class itself stays
 * non-templated.
 *
 * Pipeline shape (constructed lazily by setConnectionFactory):
 *
 *   ConnectionListener (head)
 *     → [ConnectionTLSHandler]                       // PERMITTED + REQUIRED;
 *                                                    //   owns the inner TLS
 *                                                    //   pipeline (classifier,
 *                                                    //   fizz, stoptls)
 *     → ConnectionBuilderHandler<F>
 *     → [ConnectionAcceptCallbackHandler<Conn>]      // only if onAccept is set
 *     → ConnectionInstaller<Conn>  (tail)
 *
 * Shutdown is single-call: stop(timeout) tears the acceptance pipeline
 * down, drains every live connection (drain() then waits on the close
 * callback to remove each entry), and force-closes any stragglers when
 * the timeout elapses. Must NOT be called from the owning EVB thread —
 * the wait would block the same loop that fires the close callbacks.
 *
 * Hot-reload of TLS state is pull-based: ConnectionHandler holds the
 * Observer and threads it into the TLS pipeline at construction time.
 */
class ConnectionHandler {
 public:
  using Ptr = std::unique_ptr<ConnectionHandler>;

  ConnectionHandler(
      folly::EventBase& evb,
      folly::SocketAddress address,
      fast_security::SSLPolicy sslPolicy,
      folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
          tlsParamsObserver,
      SocketOptions socketOptions,
      bool enableReusePortBpfSpread);

  ~ConnectionHandler();

  ConnectionHandler(const ConnectionHandler&) = delete;
  ConnectionHandler& operator=(const ConnectionHandler&) = delete;
  ConnectionHandler(ConnectionHandler&&) = delete;
  ConnectionHandler& operator=(ConnectionHandler&&) = delete;

  /**
   * Build and start the acceptance pipeline using `factory`. Optional
   * `onAccept` callback fires once per accepted connection with typed
   * access to the freshly-built Connection, between creation and storage.
   *
   * `factory` is borrowed — caller must keep it alive until this
   * handler's pipeline is torn down (stop() / dtor). Typically owned by
   * ConnectionManager and outlives every per-EVB handler.
   *
   * Must be called exactly once on the owning EVB before any accepts can
   * be processed.
   */
  template <ConnectionFactory F>
  void setConnectionFactory(
      F& factory, std::function<void(FactoryConnectionType<F>&)> onAccept = {});

  /**
   * Graceful shutdown. Stops accept, drains every live connection, waits
   * up to `drainTimeout` for the drain to complete, then force-closes any
   * stragglers. Must be called off-EVB.
   */
  void stop(std::chrono::milliseconds drainTimeout = std::chrono::seconds{30});

  folly::SocketAddress getAddress() const;

  // Thread-safe: backed by an atomic kept in sync with connections_ on the
  // EVB. Plain connections_.size() would race with off-EVB readers
  // (test threads, observability).
  size_t connectionCount() const noexcept {
    return connectionCount_.load(std::memory_order_relaxed);
  }

 private:
  // Type-erased per-connection storage. The function-pointer-erasure
  // pattern keeps the connection layer free of vtables on our types.
  struct AnyConnection {
    std::unique_ptr<void, void (*)(void*) noexcept> conn;
    void (*startFn)(void*) noexcept;
    void (*closeFn)(void*) noexcept;
    void (*drainFn)(void*) noexcept;

    void start() noexcept { startFn(conn.get()); }
    void close() noexcept { closeFn(conn.get()); }
    void drain() noexcept { drainFn(conn.get()); }
  };

  template <typename C>
  static AnyConnection wrap(C c) {
    return AnyConnection{
        .conn = std::unique_ptr<void, void (*)(void*) noexcept>(
            new C(std::move(c)),
            [](void* p) noexcept { delete static_cast<C*>(p); }),
        .startFn = [](void* p) noexcept { static_cast<C*>(p)->start(); },
        .closeFn = [](void* p) noexcept { static_cast<C*>(p)->close(); },
        .drainFn = [](void* p) noexcept { static_cast<C*>(p)->drain(); },
    };
  }

  // Acceptance pipeline teardown — invoked from stop().
  void stopAcceptingOnEvb();
  // Initiate drain on every live connection — invoked from stop() on EVB.
  void drainAllOnEvb();
  // Force-close any remaining connections — invoked from stop() on EVB.
  void closeAllConnectionsOnEvb();

  // Close-callback installed on every accepted connection. Erases the
  // entry from connections_ and posts drainedBaton_ when the last one
  // drops during drain. Runs on this handler's EVB.
  void onConnectionClosed(uint64_t connId) noexcept;

  // Post drainedBaton_ at most once. The drain-empty path in
  // drainAllOnEvb and the last-close path in onConnectionClosed both
  // race to post; folly::Baton requires a single post per cycle.
  void postDrainedOnce() noexcept;

  folly::Executor::KeepAlive<folly::EventBase> evb_;
  folly::SocketAddress address_;
  SocketOptions socketOptions_;
  bool enableReusePortBpfSpread_;
  fast_security::SSLPolicy sslPolicy_;
  folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
      tlsParamsObserver_;

  // Acceptance pipeline pieces. listener_ is constructed in the ctor so
  // getAddress() works before setConnectionFactory(); the rest is built
  // lazily in setConnectionFactory(). Declaration order ensures the
  // pipeline tears down first.
  channel_pipeline::SimpleBufferAllocator allocator_;
  ConnectionListener::Ptr listener_;
  std::unique_ptr<
      folly::DelayedDestruction,
      void (*)(folly::DelayedDestruction*) noexcept>
      installer_{nullptr, [](folly::DelayedDestruction*) noexcept {}};
  channel_pipeline::PipelineImpl::Ptr pipeline_;

  folly::F14NodeMap<uint64_t, AnyConnection> connections_;
  // Mirrors connections_.size(). Mutated on the EVB alongside the map so
  // observability calls can read it lock-free from any thread.
  std::atomic<size_t> connectionCount_{0};
  uint64_t nextConnId_{0};

  // Posted from onConnectionClosed when the last connection drops while
  // draining_ is set. stop() waits on this with a timeout. drainedPosted_
  // guards against the multiple racers (empty-at-start path + last-close
  // path) all trying to post the baton.
  std::atomic<bool> draining_{false};
  std::atomic<bool> drainedPosted_{false};
  folly::Baton<> drainedBaton_;
};

// Template definition — must live in the header.
template <ConnectionFactory F>
void ConnectionHandler::setConnectionFactory(
    F& factory, std::function<void(FactoryConnectionType<F>&)> onAccept) {
  using Conn = FactoryConnectionType<F>;
  using Builder = ConnectionBuilderHandler<F>;
  using AcceptCallback = ConnectionAcceptCallbackHandler<Conn>;
  using Installer = ConnectionInstaller<Conn>;

  CHECK(!pipeline_) << "setConnectionFactory called twice";

  // Tail. registerFn is the only place the handler's concrete Conn type
  // appears outside the pipeline. It wraps each connection into an
  // AnyConnection and installs a close callback so the connection self-
  // removes from connections_ when it tears down.
  //
  // Ordering note: start() must run AFTER the connection is registered in
  // connections_, because start() can synchronously dispatch the first
  // request (e.g. when the post-StopTLS handoff has bytes already buffered
  // at the transport), and any synchronous close fired off that dispatch
  // would invoke onConnectionClosed against an entry that doesn't yet
  // exist. The local `c` is moved into the map, so start() is invoked via
  // the stored AnyConnection.
  auto installer =
      typename Installer::Ptr(new Installer([this](Conn c) noexcept {
        auto connId = nextConnId_++;
        c.setCloseCallback(
            [this, connId]() noexcept { onConnectionClosed(connId); });
        auto it = connections_.emplace(connId, wrap<Conn>(std::move(c))).first;
        connectionCount_.fetch_add(1, std::memory_order_relaxed);
        it->second.start();
      }));
  auto* installerRaw = installer.get();
  installer_ = std::unique_ptr<
      folly::DelayedDestruction,
      void (*)(folly::DelayedDestruction*) noexcept>(
      installer.release(),
      [](folly::DelayedDestruction* p) noexcept { p->destroy(); });

  channel_pipeline::PipelineBuilder<
      ConnectionListener,
      Installer,
      channel_pipeline::SimpleBufferAllocator>
      builder;
  builder.setEventBase(evb_.get())
      .setHead(listener_.get())
      .setTail(installerRaw)
      .setAllocator(&allocator_);

  // The TLS lifecycle (peek classification under PERMITTED, fizz handshake,
  // optional StopTLS V1 downgrade) lives entirely inside ConnectionTLSHandler
  // as an inner pipeline. The outer pipeline sees one handler. The Observer
  // is forwarded in and snapshotted per accept inside that inner pipeline
  // (hot-reload safe).
  if (sslPolicy_ != fast_security::SSLPolicy::DISABLED) {
    builder.template addNextDuplex<handler::ConnectionTLSHandler>(
        connection_tls_tag, *evb_, sslPolicy_, tlsParamsObserver_, &allocator_);
  }

  builder.template addNextInbound<Builder>(
      connection_builder_handler_tag, factory);

  if (onAccept) {
    builder.template addNextInbound<AcceptCallback>(
        connection_accept_callback_handler_tag, std::move(onAccept));
  }

  pipeline_ = builder.build();
  listener_->setPipeline(pipeline_.get());
  installerRaw->setPipeline(pipeline_.get());
  listener_->start();
}

} // namespace apache::thrift::fast_thrift::connection
