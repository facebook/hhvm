/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <string>

#include <folly/fibers/Baton.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/VirtualEventBase.h>

#include "mcrouter/lib/CompressionCodecManager.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/debug/ConnectionFifo.h"
#include "mcrouter/lib/fbi/cpp/ObjectPool.h"
#include "mcrouter/lib/network/ClientMcParser.h"
#include "mcrouter/lib/network/ConnectionDownReason.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/McClientRequestContext.h"
#include "mcrouter/lib/network/RpcStatsContext.h"
#include "mcrouter/lib/network/SocketUtil.h"
#include "mcrouter/lib/network/Transport.h"

namespace facebook {
namespace memcache {

/**
 * A base class for network communication with memcache protocol.
 *
 * This is an impl class, user should use AsyncMcClient.
 */
class AsyncMcClientImpl : public folly::DelayedDestruction,
                          private folly::AsyncSocket::ConnectCallback,
                          private folly::AsyncTransportWrapper::ReadCallback,
                          private folly::AsyncTransportWrapper::WriteCallback,
                          private folly::AsyncTransport::BufferCallback {
 public:
  using FlushList = Transport::FlushList;
  using ConnectionStatusCallbacks = Transport::ConnectionStatusCallbacks;
  using RequestStatusCallbacks = Transport::RequestStatusCallbacks;
  using AuthorizationCallbacks = Transport::AuthorizationCallbacks;
  using RequestQueueStats = Transport::RequestQueueStats;

  static std::shared_ptr<AsyncMcClientImpl> create(
      folly::VirtualEventBase& eventBase,
      ConnectionOptions options);

  AsyncMcClientImpl(const AsyncMcClientImpl&) = delete;
  AsyncMcClientImpl& operator=(const AsyncMcClientImpl&) = delete;

  // Fail all requests and close connection.
  void closeNow();

  void setConnectionStatusCallbacks(ConnectionStatusCallbacks callbacks);

  void setRequestStatusCallbacks(RequestStatusCallbacks callbacks);

  void setAuthorizationCallbacks(AuthorizationCallbacks callbacks);

  template <class Request>
  ReplyT<Request> sendSync(
      const Request& request,
      std::chrono::milliseconds timeout,
      RpcStatsContext* rpcContext);

  void setThrottle(size_t maxInflight, size_t maxPending);

  RequestQueueStats getRequestQueueStats() const;

  void updateTimeoutsIfShorter(
      std::chrono::milliseconds connectTimeout,
      std::chrono::milliseconds writeTimeout);

  /**
   * @return        The transport used to manage socket
   */
  const folly::AsyncTransportWrapper* getTransport() const {
    return socket_.get();
  }

  double getRetransmitsPerKb();

  void setFlushList(FlushList* flushList) {
    flushList_ = flushList;
  }

 private:
  using ParserT = ClientMcParser<AsyncMcClientImpl>;
  friend ParserT;

  folly::EventBase& eventBase_;
  std::unique_ptr<ParserT> parser_;

  // Pointer to current buffer. Updated by getReadBuffer()
  std::pair<void*, size_t> curBuffer_{nullptr, 0};

  // Socket related variables.
  ConnectionState connectionState_{ConnectionState::Down};
  folly::AsyncTransportWrapper::UniquePtr socket_;
  ConnectionStatusCallbacks connectionCallbacks_;
  RequestStatusCallbacks requestStatusCallbacks_;
  AuthorizationCallbacks authorizationCallbacks_;
  int32_t numConnectTimeoutRetriesLeft_{0};

  // Debug pipe.
  ConnectionFifo debugFifo_;

  CodecIdRange supportedCompressionCodecs_ = CodecIdRange::Empty;

  McClientRequestContextQueue queue_;

  // Id for the next message that will be used by the next sendMsg() call.
  uint32_t nextMsgId_{1};

  bool outOfOrder_{false};
  bool pendingGoAwayReply_{false};

  // Throttle options (disabled by default).
  size_t maxPending_{0};
  size_t maxInflight_{0};

  // Writer loop related variables.
  class WriterLoop : public folly::EventBase::LoopCallback {
   public:
    explicit WriterLoop(AsyncMcClientImpl& client) : client_(client) {}
    ~WriterLoop() override {}
    void runLoopCallback() noexcept final;

   private:
    bool rescheduled_{false};
    AsyncMcClientImpl& client_;
  } writer_;
  FlushList* flushList_{nullptr};

  // Retransmission values
  uint32_t lastRetrans_{0}; // last known value of the no. of retransmissions
  uint64_t lastKBytes_{0}; // last known number of kBs sent

  bool isAborting_{false};

  ConnectionOptions connectionOptions_;

  std::unique_ptr<folly::EventBase::OnDestructionCallback>
      eventBaseDestructionCallback_;

  // We need to be able to get shared_ptr to ourself and shared_from_this()
  // doesn't work correctly with DelayedDestruction.
  std::weak_ptr<AsyncMcClientImpl> selfPtr_;

  AsyncMcClientImpl(
      folly::VirtualEventBase& eventBase,
      ConnectionOptions options);

  ~AsyncMcClientImpl() override;

  // Common part for send/sendSync.
  void sendCommon(McClientRequestContextBase& req);

  void sendGoAwayReply();

  // Write some requests from sendQueue_ to the socket, until max inflight limit
  // is reached or queue is empty.
  void pushMessages();
  // Schedule next writer loop if it's not scheduled.
  void scheduleNextWriterLoop();
  void cancelWriterCallback();
  size_t getNumToSend() const;

  void attemptConnection();

  // Log error with additional diagnostic information.
  void logErrorWithContext(folly::StringPiece reason);
  folly::StringPiece clientStateToStr() const;

  // AsyncSocket::ConnectCallback overrides
  void connectSuccess() noexcept final;
  void connectErr(const folly::AsyncSocketException& ex) noexcept final;

  // We've have encountered some error or we're shutting down the client.
  // It goes to DOWN state.
  void processShutdown(folly::StringPiece errorMessage);

  // AsyncTransportWrapper::ReadCallback overrides
  void getReadBuffer(void** bufReturn, size_t* lenReturn) final;
  void readDataAvailable(size_t len) noexcept final;
  void readEOF() noexcept final;
  void readErr(const folly::AsyncSocketException& ex) noexcept final;

  // AsyncTransportWrapper::WriteCallback overrides
  void writeSuccess() noexcept final;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept final;

  // AsyncTransport::BufferCallback overrides
  void onEgressBuffered() override final;
  void onEgressBufferCleared() override final;

  int64_t getNumConnectRetries() noexcept;

  // Callbacks for McParser.
  template <class Reply>
  void
  replyReady(Reply&& reply, uint64_t reqId, RpcStatsContext rpcStatsContext);
  void handleConnectionControlMessage(const CaretMessageInfo& headerInfo);
  void parseError(carbon::Result result, folly::StringPiece reason);
  bool nextReplyAvailable(uint64_t reqId);

  static void incMsgId(uint32_t& msgId);
};
} // namespace memcache
} // namespace facebook

#include "AsyncMcClientImpl-inl.h"
