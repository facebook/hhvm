/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <functional>

#include <boost/intrusive/list.hpp>

#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/network/ConnectionDownReason.h"
#include "mcrouter/lib/network/ConnectionOptions.h"

// forward declarations
namespace folly {
class AsyncTransport;
}

namespace facebook {
namespace memcache {

/**
 * Abstract class that provides an API for transports.
 *
 * In practice, we don't use this abstract class in code (i.e. we use the
 * concrete transport classes like AsyncMcClient directly). So this class
 * is used to enforce the API as well and to provide documentation.
 *
 * Concrete transports, apart from implementing all the virtual methods, must
 * also implement 1 constructor and 3 other methods:
 *
 *   // Constructs the transport.
 *   Transport(folly::VirtualEventBase& eventBase, ConnectionOptions options);
 *
 *   // Provides the name of the Tranport
 *   static constexpr folly::StringPiece name();
 *
 *   // Tells if this transport is compatible with a given protocol.
 *   static constexpr bool isCompatible(mc_protocol_t protocol);
 *
 *   // Sends a request to the server using this transport
 *   template <class Request>
 *   ReplyT<Request> sendSync(
 *       const Request& request,
 *       std::chrono::milliseconds timeout,
 *       RpcStatsContext* rpcContext = nullptr);
 *
 * NOTE: Concrete transport classes should mark all methods as final to avoid
 *       unnecessary virtual function calls.
 */
class Transport : public folly::DelayedDestruction {
 public:
  /**
   * Queue used for managing flush callbacks.
   */
  using FlushList = boost::intrusive::list<
      folly::EventBase::LoopCallback,
      boost::intrusive::constant_time_size<false>>;

  using SvcIdentAuthCallbackFunc = std::function<
      bool(const folly::AsyncTransport&, const ConnectionOptions&)>;

  /**
   * Struct containing callbacks regarding connection state changes.
   */
  struct ConnectionStatusCallbacks {
    /*
     * Will be called whenever client successfully connects to the
     * server. Will be called immediately if we're already connected.
     * Can be nullptr.
     */
    std::function<void(const folly::AsyncTransport&, size_t numConnectRetries)>
        onUp;

    /**
     * Will be called whenever connection goes down. Will be passed
     * explanation about why the connection went down. Will not be called if
     * the connection is already DOWN.
     * Can be nullptr.
     */
    std::function<void(ConnectionDownReason, size_t numConnectRetires)> onDown;
  };

  /**
   * Struct containing callbacks regarding request processing state changes.
   */
  struct RequestStatusCallbacks {
    /**
     * Will be called whenever a request changes state.
     * pendingDiff and inflightDiff will hold the difference in the number of
     * pending and inflight requests, respectively.
     */
    std::function<void(int pendingDiff, int inflightDiff)> onStateChange;

    /**
     * Will be called everytime AsyncMcClient is about to write data to network.
     * The numToSend argument holds the number of requests that will be sent in
     * a single batch.
     */
    std::function<void(size_t numToSend)> onWrite;

    /**
     * Will be called everytime a partial write happened. This means we would
     * block when performing the full write, so we buffered some data to try
     * again when the transport becomes ready to be written to again.
     */
    std::function<void()> onPartialWrite;
  };

  /**
   * Struct containing callbacks regarding client authorization.
   */
  struct AuthorizationCallbacks {
    /*
     * Will be called whenever client successfully connects to the
     * server and client authorization has been configured. Returning true
     * means that the session has been authorized.
     * Can be nullptr.
     */
    SvcIdentAuthCallbackFunc onAuthorize;
  };

  /**
   * Holds information about number of requests inflight and pending.
   */
  struct RequestQueueStats {
    /**
     * Number of requests in pending queue. Those requests have not been sent
     * to the network yet, this means that in case of remote error we can still
     * try to send them.
     */
    size_t numPending;

    /**
     * Number of requests in inflight queue. This amounts for requests that are
     * currently been written to the socket and requests that were already sent
     * to the server and are waiting for replies. Those requests might be
     * already processed by the server, thus they wouldn't be retransmitted in
     * case of error.
     */
    size_t numInflight;
  };

  /**
   * Close connection and fail all outstanding requests immediately.
   */
  virtual void closeNow() = 0;

  /**
   * Set status callbacks for the underlying connection.
   *
   * NOTE: those callbacks may be called even after the client was destroyed.
   *       This will happen in case when the client is destroyed and there are
   *       some requests left, for wich reply callback wasn't called yet.
   */
  virtual void setConnectionStatusCallbacks(
      ConnectionStatusCallbacks callbacks) = 0;

  /**
   * Set callbacks for when requests state change.
   */
  virtual void setRequestStatusCallbacks(RequestStatusCallbacks callbacks) = 0;

  /**
   * Set callbacks to autorize connections.
   */
  virtual void setAuthorizationCallbacks(AuthorizationCallbacks callbacks) = 0;

  /**
   * Set throttling options.
   *
   * @param maxInflight max number of requests that can be waiting for the
   *                    network reply (0 means unlimited).
   * @param maxPending  max number of requests that can be waiting to be
   *                    sent over the network (0 means unlimited). If on attempt
   *                    to send a new request we're going to exceed this limit,
   *                    then that request would fail and the callback would be
   *                    called with a proper error code immediately
   *                    (e.g. local error).
   *                    Also user should not expect to be able to put more
   *                    than maxPending requests into the queue at once (i.e.
   *                    user should not expect to be able to send
   *                    maxPending+maxInflight requests at once).
   * Note: will not affect already sent or pending requests. None of them would
   *       be dropped.
   */
  virtual void setThrottle(size_t maxInflight, size_t maxPending) = 0;

  /**
   * Get the current stats of the requests queues.
   */
  virtual RequestQueueStats getRequestQueueStats() const = 0;

  /**
   * Update connect and write timeouts. If the new value is larger than the
   * current value, it is ignored.
   *
   * @param connectTimeout  The new connect timeout.
   * @param writeTimeout    The new write timeout.
   */
  virtual void updateTimeoutsIfShorter(
      std::chrono::milliseconds connectTimeout,
      std::chrono::milliseconds writeTimeout) = 0;

  /**
   * Get the underlying transport used by this transport.
   *
   * @return  The transport used to manage socket.
   */
  virtual const folly::AsyncTransport* getTransport() const = 0;

  /**
   * Return retransmit information about this connection.
   *
   * @return  Number of retransmits per KB of data transfered.
   */
  virtual double getRetransmitsPerKb() = 0;

  /**
   * Set external queue for managing flush callbacks.
   * By default we'll use EventBase as a manager of these callbacks.
   */
  virtual void setFlushList(FlushList* flushList) = 0;

 protected:
  ~Transport() override = default;
};

} // namespace memcache
} // namespace facebook
