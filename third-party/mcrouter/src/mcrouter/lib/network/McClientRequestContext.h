/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <typeindex>

#include <boost/intrusive/unordered_set.hpp>

#include <folly/IntrusiveList.h>
#include <folly/fibers/Baton.h>

#include "mcrouter/lib/network/ClientMcParser.h"
#include "mcrouter/lib/network/FBTrace.h"
#include "mcrouter/lib/network/McSerializedRequest.h"
#include "mcrouter/lib/network/RpcStatsContext.h"

namespace facebook {
namespace memcache {

class AsyncMcClientImpl;
struct CodecIdRange;
class McClientRequestContextQueue;

/**
 * Class for storing per request data that is required for proper requests
 * processing inside of AsyncMcClient.
 */
class McClientRequestContextBase
    : public boost::intrusive::unordered_set_base_hook<> {
 public:
  using InitializerFuncPtr = void (*)(ClientMcParser<AsyncMcClientImpl>&);

  McSerializedRequest reqContext;
  uint64_t id;
  bool isBatchTail{false};

  McClientRequestContextBase(const McClientRequestContextBase&) = delete;
  McClientRequestContextBase& operator=(
      const McClientRequestContextBase& other) = delete;
  McClientRequestContextBase(McClientRequestContextBase&&) = delete;
  McClientRequestContextBase& operator=(McClientRequestContextBase&& other) =
      delete;

  /**
   * Get string representation of a type of the context.
   * (E.g. operation + request type).
   */
  virtual std::string getContextTypeStr() const = 0;

  /**
   * Propagate an error to the user.
   *
   * Should be called only when the request is not in a queue.
   */
  void replyError(carbon::Result result, folly::StringPiece errorMessage);

  /**
   * Schedule a timeout so that the request does not wait
   * indefinitely for a reply.
   */
  void scheduleTimeout();

  void setRpcStatsContext(RpcStatsContext value) {
    rpcStatsContext_ = value;
    rpcStatsContext_.requestBodySize = reqContext.getBodySize();
  }

  RpcStatsContext getRpcStatsContext() const {
    return rpcStatsContext_;
  }

 protected:
  enum class ReqState {
    NONE,
    PENDING_QUEUE,
    WRITE_QUEUE,
    PENDING_REPLY_QUEUE,
    REPLIED_QUEUE,
    COMPLETE,
  };

  virtual ~McClientRequestContextBase();

  template <class Request>
  McClientRequestContextBase(
      const Request& request,
      uint64_t reqid,
      mc_protocol_t protocol,
      folly::Optional<ReplyT<Request>>& replyStorage,
      McClientRequestContextQueue& queue,
      InitializerFuncPtr initializer,
      const std::function<void(int pendingDiff, int inflightDiff)>&
          onStateChange,
      const CodecIdRange& supportedCodecs);

  virtual void replyErrorImpl(
      carbon::Result result,
      folly::StringPiece errorMessage) = 0;

  ReqState state() const {
    return state_;
  }

  void setState(ReqState newState) {
    fireStateChangeCallbacks(state_, newState);
    state_ = newState;
  }

  folly::fibers::Baton baton_;
  McClientRequestContextQueue& queue_;

  folly::fibers::Baton::TimeoutHandler batonTimeoutHandler_;
  std::chrono::milliseconds batonWaitTimeout_{0};

 private:
  friend class McClientRequestContextQueue;

  std::type_index replyType_;
  folly::SafeIntrusiveListHook hook_;
  void* replyStorage_;
  InitializerFuncPtr initializer_;

  ReqState state_{ReqState::NONE};

  RpcStatsContext rpcStatsContext_;

  const std::function<void(int pendingDiff, int inflightDiff)>& onStateChange_;

  /**
   * Fire the request state change callbacks.
   */
  void fireStateChangeCallbacks(ReqState old, ReqState current) const;

  /**
   * Entry point for propagating reply to the user.
   *
   * Typechecks the reply and propagates it to the proper subclass.
   * If the reply type doesn't match the expected one, replies the request with
   * an error
   */
  template <class Reply>
  void reply(Reply&& r);

 public:
  struct Equal {
    bool operator()(
        const McClientRequestContextBase& a,
        const McClientRequestContextBase& b) const {
      return a.id == b.id;
    }
  };

  struct Hash {
    size_t operator()(const McClientRequestContextBase& ctx) const {
      return std::hash<uint64_t>()(ctx.id);
    }
  };

  using Queue = folly::CountedIntrusiveList<
      McClientRequestContextBase,
      &McClientRequestContextBase::hook_>;
  using UnorderedSet = boost::intrusive::unordered_set<
      McClientRequestContextBase,
      boost::intrusive::equal<Equal>,
      boost::intrusive::hash<Hash>>;
};

template <class Request>
class McClientRequestContext : public McClientRequestContextBase {
 public:
  using Reply = ReplyT<Request>;

  McClientRequestContext(
      const Request& request,
      uint64_t reqid,
      mc_protocol_t protocol,
      McClientRequestContextQueue& queue,
      McClientRequestContextBase::InitializerFuncPtr,
      const std::function<void(int pendingDiff, int inflightDiff)>&
          onStateChange,
      const CodecIdRange& supportedCodecs);

  std::string getContextTypeStr() const final;

  Reply waitForReply(std::chrono::milliseconds timeout);

 private:
  folly::Optional<Reply> replyStorage_;

  // tracing fields
  const std::string& requestTraceContext_;

  void replyErrorImpl(carbon::Result result, folly::StringPiece errorMessage)
      final;
};

class McClientRequestContextQueue {
 public:
  explicit McClientRequestContextQueue(bool outOfOrder) noexcept;

  McClientRequestContextQueue(const McClientRequestContextQueue&) = delete;
  McClientRequestContextQueue& operator=(
      const McClientRequestContextQueue& other) = delete;
  McClientRequestContextQueue(McClientRequestContextQueue&&) = delete;
  McClientRequestContextQueue& operator=(McClientRequestContextQueue&& other) =
      delete;

  size_t getPendingRequestCount() const noexcept;
  size_t getInflightRequestCount() const noexcept;

  /**
   * Fails all requests that were already sent (i.e. pending reply) with a given
   * error code.
   */
  void failAllSent(carbon::Result error, folly::StringPiece errorMessage);

  /**
   * Fails all requests that were not sent yet (i.e. pending) with a given error
   * code.
   */
  void failAllPending(carbon::Result error, folly::StringPiece errorMessage);

  /**
   * Return an id of the first pending request.
   *
   * Note: it's the caller responsibility to ensure that there's at least one
   *       pending request.
   */
  size_t getFirstId() const;

  /**
   * Adds request into pending queue.
   */
  void markAsPending(McClientRequestContextBase& req);

  /**
   * Peek next request that we're about to send.
   *
   * @return a reference to the next request in the pending queue.
   */
  McClientRequestContextBase& peekNextPending();

  /**
   * Moves the first request from pending queue into sending queue.
   *
   * @return a reference to the request that was marked as sending.
   */
  McClientRequestContextBase& markNextAsSending();

  /**
   * Marks the first request from sending queue as sent.
   *
   * May result in one of the following:
   *   - if request was cancelled before, request will be removed from queue
   *   - otherwise, request will be moved into pending reply queue.
   *
   * @return a reference to the request that was marked as sent.
   */
  McClientRequestContextBase& markNextAsSent();

  /**
   * Reply request with given id with the provided reply and
   * compression stats.
   * In case of in order protocol the id is ignored.
   *
   * Does nothing if the request was already removed from the queue.
   */
  template <class Reply>
  void reply(uint64_t id, Reply&& reply, RpcStatsContext rpcStatsContext);

  /**
   * Obtain a function that should be used to initialize parser for given
   * request.
   *
   * @param reqId  id of request to lookup, ignored for in order protocol.
   *
   * Note: for out of order may return nullptr in case a request with given id
   *       was cancelled.
   */
  McClientRequestContextBase::InitializerFuncPtr getParserInitializer(
      uint64_t reqId = 0);

  /**
   * Get a debug info about current queue state.
   */
  std::string debugInfo() const;

 private:
  static constexpr size_t kDefaultNumBuckets = 128;

  // Friend to allow access to remove* mothods.
  template <class Request>
  friend class McClientRequestContext;

  using State = McClientRequestContextBase::ReqState;

  bool outOfOrder_{false};
  // Queue of requests, that are queued to be sent.
  McClientRequestContextBase::Queue pendingQueue_;
  // Queue of requests, that are currently being written to the socket.
  McClientRequestContextBase::Queue writeQueue_;
  // Queue of requests, that are already sent and are waiting for replies.
  McClientRequestContextBase::Queue pendingReplyQueue_;
  // A special internal queue for request that were replied before it's been
  // completely written.
  McClientRequestContextBase::Queue repliedQueue_;
  // Unordered set of requests. Used only in case of out-of-order protocol
  // for fast request lookup.
  std::vector<McClientRequestContextBase::UnorderedSet::bucket_type> buckets_;
  McClientRequestContextBase::UnorderedSet set_;

  // Storage for parser initializers for timed out requests.
  std::queue<McClientRequestContextBase::InitializerFuncPtr>
      timedOutInitializers_;

  void failQueue(
      McClientRequestContextBase::Queue& queue,
      carbon::Result error,
      folly::StringPiece errorMessage);

  McClientRequestContextBase::UnorderedSet::iterator getContextById(
      uint64_t id);
  void removeFromSet(McClientRequestContextBase& req);

  /**
   * Removes given request from pending queue and id map.
   */
  void removePending(McClientRequestContextBase& req);

  /**
   * Removes given request from pending reply queue and from id map.
   *
   * Calling this method indicates that this request wasn't replied, but
   * we should expect a reply from network.
   */
  void removePendingReply(McClientRequestContextBase& req);

  /**
   * Should be called whenever the network communication channel gets closed.
   */
  void clearStoredInitializers();

  void growBucketsArray();

  std::string getFirstAliveRequestInfo() const;
};
} // namespace memcache
} // namespace facebook

#include "McClientRequestContext-inl.h"
