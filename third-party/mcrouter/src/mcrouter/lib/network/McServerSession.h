/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/server/AsyncFizzServer.h>
#include <folly/IntrusiveList.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/CompressionCodecManager.h"
#include "mcrouter/lib/debug/ConnectionFifo.h"
#include "mcrouter/lib/network/AsyncMcServerWorkerOptions.h"
#include "mcrouter/lib/network/McServerThriftRequestContext.h"
#include "mcrouter/lib/network/SecurityOptions.h"
#include "mcrouter/lib/network/ServerMcParser.h"
#include "mcrouter/lib/network/WriteBuffer.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

class McServerOnRequest;
class McServerRequestContext;
class MultiOpParent;
class WriteBuffer;
class WriteBufferIntrusiveList;
class WriteBufferQueue;

/**
 * A session owns a single transport, and processes the request/reply stream.
 */
class McServerSession
    : public folly::DelayedDestruction,
      private fizz::server::AsyncFizzServer::HandshakeCallback,
      private folly::AsyncSSLSocket::HandshakeCB,
      private folly::AsyncTransportWrapper::ReadCallback,
      private folly::AsyncTransportWrapper::WriteCallback {
 private:
  folly::SafeIntrusiveListHook hook_;

 public:
  using KeepAlive = folly::Executor::KeepAlive<folly::VirtualEventBase>;
  using Queue =
      folly::CountedIntrusiveList<McServerSession, &McServerSession::hook_>;

  class StateCallback {
   public:
    virtual ~StateCallback() {}
    virtual void onAccepted(McServerSession& session) = 0;
    virtual void onWriteQuiescence(McServerSession&) = 0;
    virtual void onCloseStart(McServerSession&) = 0;
    virtual void onCloseFinish(McServerSession&, bool onAcceptedCalled) = 0;
    virtual void onShutdown() = 0;
  };

  class ZeroCopySessionCB : public folly::AsyncTransportWrapper::WriteCallback {
   public:
    explicit ZeroCopySessionCB(McServerSession& session) : session_(session) {}

    ZeroCopySessionCB(const ZeroCopySessionCB&) = delete;
    ZeroCopySessionCB& operator=(const ZeroCopySessionCB&) = delete;

    /*
     * CB invoked on successful write of zero copy buffer, this does
     * not however indicate transmission.
     */
    void writeSuccess() noexcept final {
      // Zero Copy write still in progress until async notification
      assert(numCallbackPending_ > 0);
      if (--numCallbackPending_ == 0 && session_.state_ == STREAMING) {
        session_.stateCb_.onWriteQuiescence(session_);
        // No-op if not paused
        session_.resume(PAUSE_WRITE);
      }
    }

    /**
     * CB invoked on unsuccessful write of zero copy buffer.
     */
    void writeErr(
        size_t /* bytesWritten */,
        const folly::AsyncSocketException&) noexcept final {
      assert(numCallbackPending_ > 0);
      // Buffers will be freed by FreeFn in ~IOBuf
      --numCallbackPending_;
      session_.close();
    }

    void incCallbackPending() {
      ++numCallbackPending_;
    }

    uint64_t getCallbackPending() const {
      return numCallbackPending_;
    }

   private:
    McServerSession& session_;
    uint64_t numCallbackPending_{0};
  };

  /**
   * Returns true if this object is a part of an intrusive list.
   */
  bool isLinked() const noexcept {
    return hook_.is_linked();
  }

  /**
   * Creates a new session.
   * Sessions manage their own lifetime.
   * A session will self-destruct right after an onCloseFinish() callback
   * call, by which point all of the following must have occured:
   *   1) All outstanding requests have been replied and pending
   *      writes have been completed/errored out.
   *   2) The outgoing connection is closed, either via an explicit close()
   *      call or due to some error.
   *
   * onCloseStart() callback marks the beggining of session
   * teardown. The application can inititate any cleanup process. After
   * onCloseStart() the socket is no longer readable, and the application
   * should try to flush out all outstanding requests so that session
   * can be closed.
   *
   * The application may use onCloseFinish() callback as the point in
   * time after which the session is considered done, and no event loop
   * iteration is necessary.
   *
   * The session will be alive for the duration of onCloseFinish callback,
   * but this is the last time the application can safely assume that
   * the session is alive.
   *
   * The onWriteQuiescence() callback is invoked when all pending writes are
   * done, rather than invoking it for each write.
   *
   * @param transport Connected transport; transfers ownership inside
   *                  this session.
   *
   * @param queue     If a queue is provided, the session will be linked to it.
   *                  Otherwise (if queue is nullptr), it will remain unlinked.
   *
   * @throw           std::runtime_error if we fail to create McServerSession
   *                  object
   */
  static McServerSession& create(
      folly::AsyncTransportWrapper::UniquePtr transport,
      std::shared_ptr<McServerOnRequest> cb,
      StateCallback& stateCb,
      const AsyncMcServerWorkerOptions& options,
      void* userCtxt,
      McServerSession::Queue* queue,
      const CompressionCodecMap* codecMap = nullptr,
      KeepAlive keepAlive = nullptr);
  //      folly::VirtualEventBase* virtualEventBase = nullptr);

  /**
   * Set appropriate socket options on an AsyncSocket
   */
  static void applySocketOptions(
      folly::AsyncSocket& socket,
      const AsyncMcServerWorkerOptions& opts);

  /**
   * Eventually closes the transport. All pending writes will still be drained.
   * Please refer create() for info about the callbacks invoked.
   */
  void close();

  /**
   * Same as close(), but if GoAway is enabled, will send the message to the
   * client and wait for acknowledgement before actually closing.
   */
  void beginClose(folly::StringPiece reason);

  /**
   * Returns true if there are some unfinished writes pending to the transport.
   */
  bool writesPending() const {
    return inFlight_ > 0;
  }

  /**
   * Allow clients to pause and resume reading form the sockets.
   * See pause(PauseReason) and resume(PauseReason) below.
   */
  void pause() {
    pause(PAUSE_USER);
  }
  void resume() {
    resume(PAUSE_USER);
  }

  /**
   * @returns true iff reading from the socket has been paused by the user
   */
  bool paused() const noexcept {
    return (pauseState_ & static_cast<uint64_t>(PAUSE_USER)) != 0;
  }

  /**
   * Get the user context associated with this session.
   */
  void* userContext() {
    return userCtxt_;
  }

  /**
   * Set the user context associated with this session.
   */
  void setUserContext(void* userCtx) {
    userCtxt_ = userCtx;
  }

  /**
   * Get the peer's socket address
   */
  const folly::SocketAddress& getSocketAddress() const noexcept {
    return socketAddress_;
  }

  /**
   * Get the socket's local address
   */
  const folly::SocketAddress getLocalAddress() const noexcept {
    return transport_->getLocalAddress();
  }

  /**
   * @return  the client's common name obtained from the
   *          SSL cert if this is an SSL session. Else it
   *          returns empty string.
   */
  folly::StringPiece getClientCommonName() const noexcept {
    return clientCommonName_;
  }

  /**
   * @return the EventBase for this thread
   */
  folly::EventBase& getEventBase() const noexcept {
    return eventBase_;
  }

  std::shared_ptr<CpuController> getCpuController() const noexcept {
    return options_.cpuController;
  }

  const folly::AsyncTransportWrapper* getTransport() const noexcept {
    return transport_.get();
  }
  /* In order to save memory in McServerRequestContext, we store the actual
   * request context here since it is the same for every request for this
   * McServerSession
   */
  const apache::thrift::Cpp2RequestContext*
  getConnectionLevelThriftRequestContext() const noexcept;

  /**
   * Called to create a chained IOBuf from iovecs which has a free function that
   * manages the lifetime of the WriteBuffer.
   */
  void sendZeroCopyIOBuf(
      WriteBuffer& wb,
      const struct iovec* iovs,
      size_t iovsCount);

  bool isZeroCopyEnabled() const {
    return options_.tcpZeroCopyThresholdBytes > 0;
  }

  /**
   * The security mechanism being used in thos connection.
   */
  SecurityMech securityMech() const noexcept;

  /**
   * Flush pending writes to the transport.
   */
  inline void flushWrites() {
    if (writeScheduled_) {
      sendWritesCallback_.cancelLoopCallback();
    }
    sendWrites();
  }

 private:
  const AsyncMcServerWorkerOptions& options_;

  folly::AsyncTransportWrapper::UniquePtr transport_;
  /* A thin bridge in Carbon to be able to have the same request context
   * interface as Thrift */
  std::unique_ptr<const McServerThriftRequestContext> thriftRequestContext_;
  folly::EventBase& eventBase_;
  // When using the virtual event base mode, McServerSession is kept
  // alive by the KeepAlive
  KeepAlive keepAlive_;
  std::shared_ptr<McServerOnRequest> onRequest_;
  StateCallback& stateCb_;

  // Debug fifo fields
  ConnectionFifo debugFifo_;
  bool hasPendingMultiOp_{false};

  bool onAcceptedCalled_{false};

  enum State {
    STREAMING, /* close() was not called */
    CLOSING, /* close() was called, waiting on pending requests */
    CLOSED, /* close() was called and connection was torn down.
               This is a short lived state to prevent another close()
               between the first close() and McServerSession destruction
               from doing anything */
  };
  State state_{STREAMING};

  // Pointer to current buffer. Updated by getReadBuffer()
  std::pair<void*, size_t> curBuffer_;

  // All writes to be written at the end of the loop in a single batch.
  WriteBuffer::List pendingWrites_;

  /**
   * Queue of write buffers.
   */
  WriteBufferQueue writeBufs_;

  /**
   * True iff SendWritesCallback has been scheduled.
   */
  bool writeScheduled_{false};

  /**
   * True iff the next write batch should be a zero copy write.
   */
  bool isNextWriteBatchZeroCopy_{false};

  /**
   * Total number of alive McTransactions in the system.
   */
  size_t inFlight_{0};

  /**
   * Total number of alive McTransactions, excluding subrequests.
   * Used to make throttling decisions.
   * The intention is to count metagets as one request as far as
   * throttling is concerned.
   */
  size_t realRequestsInFlight_{0};

  struct SendWritesCallback : public folly::EventBase::LoopCallback {
    explicit SendWritesCallback(McServerSession& session) : session_(session) {}
    void runLoopCallback() noexcept final {
      session_.sendWrites();
    }
    McServerSession& session_;
  };

  SendWritesCallback sendWritesCallback_;

  /* OR-able bits of pauseState_ */
  enum PauseReason : uint64_t {
    PAUSE_THROTTLED = 1 << 0,
    PAUSE_WRITE = 1 << 1,
    PAUSE_USER = 1 << 2,
  };

  /* Reads are enabled iff pauseState_ == 0 */
  uint64_t pauseState_{0};

  // Compression
  const CompressionCodecMap* compressionCodecMap_{nullptr};
  CodecIdRange codecIdRange_ = CodecIdRange::Empty;

  ServerMcParser<McServerSession> parser_;

  /* In-order protocol state */

  /* headReqid_ <= tailReqid_.  Since we must output replies sequentially,
     headReqid_ tracks the last reply id we're allowed to sent out.
     Out of order replies are stalled in the blockedReplies_ queue. */
  uint64_t headReqid_{0}; /**< Id of next unblocked reply */
  uint64_t tailReqid_{0}; /**< Id to assign to next request */
  std::unordered_map<uint64_t, std::unique_ptr<WriteBuffer>> blockedReplies_;

  /* If non-null, a multi-op operation is being parsed.*/
  std::shared_ptr<MultiOpParent> currentMultiop_;

  folly::SocketAddress socketAddress_;

  /**
   * If this session corresponds to an SSL session then
   * this is set to the common name from client cert
   */
  std::string clientCommonName_;

  void* userCtxt_{nullptr};

  std::unique_ptr<folly::AsyncTimeout> goAwayTimeout_;

  ZeroCopySessionCB zeroCopySessionCB_;
  SecurityMech negotiatedMech_{SecurityMech::NONE};

  /**
   * pause()/resume() reads from the socket (TODO: does not affect the already
   * read buffer - requests in it will still be processed).
   *
   * We stop reading from the socket if at least one reason has a pause()
   * without a corresponding resume().
   */
  void pause(PauseReason reason);
  void resume(PauseReason reason);

  /**
   * Flush pending writes to the transport.
   */
  void sendWrites();

  /**
   * Check if no outstanding transactions, and close socket and
   * call onCloseFinish_() if so.
   */
  void checkClosed();

  void reply(std::unique_ptr<WriteBuffer> wb, uint64_t reqid);

  /**
   * Called when end context is seen (for multi-op requests) or connection
   * close to close out an in flight multi-op request.
   */
  void processMultiOpEnd();

  /* AsyncTransport's readCallback */
  void getReadBuffer(void** bufReturn, size_t* lenReturn) final;
  void readDataAvailable(size_t len) noexcept final;
  void readEOF() noexcept final;
  void readErr(const folly::AsyncSocketException& ex) noexcept final;

  /* McParser's callback if ASCII request is read into a typed request */
  template <class Request>
  void asciiRequestReady(Request&& req, carbon::Result result, bool noreply);

  void caretRequestReady(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBody);

  void processConnectionControlMessage(const CaretMessageInfo& headerInfo);

  void parseError(carbon::Result result, folly::StringPiece reason);

  /* Ascii parser callbacks */
  template <class Request>
  void onRequest(Request&& req, bool noreply) {
    carbon::Result result = carbon::Result::UNKNOWN;
    if (req.key_ref()->fullKey().size() > MC_KEY_MAX_LEN_ASCII) {
      result = carbon::Result::BAD_KEY;
    }
    asciiRequestReady(std::move(req), result, noreply);
  }

  /* ASCII parser callbacks for special commands */
  void onRequest(McVersionRequest&& req, bool noreply);

  void onRequest(McShutdownRequest&& req, bool noreply);

  void onRequest(McQuitRequest&& req, bool noreply);

  void multiOpEnd();

  void queueWrite(std::unique_ptr<WriteBuffer> wb);

  void completeWrite();

  /* AsyncTransport's writeCallback */
  void writeSuccess() noexcept final;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept final;

  /* AsyncSSLSocket::HandshakeCB interface */
  bool handshakeVer(
      folly::AsyncSSLSocket* sock,
      bool preverifyOk,
      X509_STORE_CTX* ctx) noexcept final;
  void handshakeSuc(folly::AsyncSSLSocket* sock) noexcept final;
  void handshakeErr(
      folly::AsyncSSLSocket* sock,
      const folly::AsyncSocketException& ex) noexcept final;

  void fizzHandshakeSuccess(
      fizz::server::AsyncFizzServer* transport) noexcept final;

  void fizzHandshakeError(
      fizz::server::AsyncFizzServer* transport,
      folly::exception_wrapper ex) noexcept final;

  void fizzHandshakeAttemptFallback(
      fizz::server::AttemptVersionFallback fallback) final;

  void onTransactionStarted(bool isSubRequest);
  void onTransactionCompleted(bool isSubRequest);

  void writeToDebugFifo(const WriteBuffer* wb) noexcept;

  /**
   * Update the connection's valid range of codec ids that may be used
   * to compress the reply.  Any requests that are still in flight will be
   * replied assuming this newly updated range.
   */
  void updateCompressionCodecIdRange(
      const CaretMessageInfo& headerInfo) noexcept;

  McServerSession(
      folly::AsyncTransportWrapper::UniquePtr transport,
      std::shared_ptr<McServerOnRequest> cb,
      StateCallback& stateCb,
      const AsyncMcServerWorkerOptions& options,
      void* userCtxt,
      const CompressionCodecMap* codecMap,
      KeepAlive keepAlive = nullptr);
  //     folly::VirtualEventBase* virtualEventBase = nullptr);

  McServerSession(const McServerSession&) = delete;
  McServerSession& operator=(const McServerSession&) = delete;

  // wrappers around the onState_ callbacks.
  void onAccepted();
  void onCloseStart();
  void onCloseFinish();
  void onWriteQuiescence();

  friend class McServerRequestContext;
  friend class ServerMcParser<McServerSession>;
  friend class ZeroCopySessionCB;
};
} // namespace memcache
} // namespace facebook

#include "McServerSession-inl.h"
