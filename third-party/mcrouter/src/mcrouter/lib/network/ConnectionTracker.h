/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/VirtualEventBase.h>

#include "mcrouter/lib/network/McServerSession.h"

namespace facebook {
namespace memcache {

/**
 * Single threaded list of connections with LRU eviction logic.
 */
class ConnectionTracker : public McServerSession::StateCallback {
 public:
  /**
   * Creates a new tracker with `maxConns` connections. Once there are
   * more than `maxConns` connections (sessions), ConnectionTracker will close
   * the oldest one. If `maxConns` is 0, it will not close connections.
   */
  explicit ConnectionTracker(size_t maxConns = 0);

  // See AsyncMcServerWorker.h for details about the callbacks
  void setOnConnectionAccepted(std::function<void(McServerSession&)> cb) {
    onAccepted_ = std::move(cb);
  }

  void setOnWriteQuiescence(std::function<void(McServerSession&)> cb) {
    onWriteQuiescence_ = std::move(cb);
  }

  void setOnConnectionCloseStart(std::function<void(McServerSession&)> cb) {
    onCloseStart_ = std::move(cb);
  }

  void setOnConnectionCloseFinish(
      std::function<void(McServerSession&, bool onAcceptedCalled)> cb) {
    onCloseFinish_ = std::move(cb);
  }

  void setOnShutdownOperation(std::function<void()> cb) {
    onShutdown_ = std::move(cb);
  }

  /**
   * Creates a new entry in the LRU and places the connection at the front.
   *
   * @return reference to the created session.
   * @throws std::runtime_exception when fails to create a session.
   */
  McServerSession& add(
      folly::AsyncTransportWrapper::UniquePtr transport,
      std::shared_ptr<McServerOnRequest> cb,
      const AsyncMcServerWorkerOptions& options,
      void* userCtxt,
      const CompressionCodecMap* compressionCodecMap,
      McServerSession::KeepAlive keepAlive = nullptr);

  /**
   * Close all connections (sessions)
   */
  void closeAll();

  /**
   * Check if we have pending writes on any connection (session)
   */
  bool writesPending() const;

 private:
  McServerSession::Queue sessions_;
  std::function<void(McServerSession&)> onAccepted_;
  std::function<void(McServerSession&)> onWriteQuiescence_;
  std::function<void(McServerSession&)> onCloseStart_;
  std::function<void(McServerSession&, bool onAcceptedCalled)> onCloseFinish_;
  std::function<void()> onShutdown_;
  size_t maxConns_{0};

  void touch(McServerSession& session);

  void evict();

  // McServerSession::StateCallback API
  void onAccepted(McServerSession& session) final;
  void onWriteQuiescence(McServerSession& session) final;
  void onCloseStart(McServerSession& session) final;
  void onCloseFinish(McServerSession& session, bool onAcceptedCalled) final;
  void onShutdown() final;

  int64_t numCalls_{0};
};
} // namespace memcache
} // namespace facebook
