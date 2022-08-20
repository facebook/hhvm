/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <utility>

#include <folly/Optional.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include "mcrouter/lib/Operation.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {

template <class OnRequest, class RequestList>
class McServerOnRequestWrapper;
class McServerSession;
class MultiOpParent;

/**
 * API for users of McServer to send back a reply for a request.
 *
 * Each onRequest callback is provided a context object,
 * which must eventually be surrendered back via a reply() call.
 */
class McServerRequestContext {
 public:
  using DestructorFunc = void (*)(void*);

  template <class Reply>
  static void
  reply(McServerRequestContext&& ctx, Reply&& reply, bool flush = false);

  template <class Reply>
  static void reply(
      McServerRequestContext&& ctx,
      Reply&& reply,
      DestructorFunc destructor,
      void* toDestruct);

  ~McServerRequestContext();

  McServerRequestContext(McServerRequestContext&& other) noexcept;
  McServerRequestContext& operator=(McServerRequestContext&& other);

  /**
   * Get the associated McServerSession
   */
  McServerSession& session();

  ServerLoad getServerLoad() const noexcept;

  folly::Optional<struct sockaddr_storage> getPeerSocketAddress();
  folly::Optional<std::string> getPeerSocketAddressStr();

  folly::EventBase& getSessionEventBase() const noexcept;
  const apache::thrift::Cpp2RequestContext* getThriftRequestContext()
      const noexcept;

  const folly::AsyncTransportWrapper* getTransport() const noexcept;

  void markAsTraced();

  void* getConnectionUserData();

 private:
  McServerSession* session_;

  /* Pack these together, operation + flags takes one word */
  bool isEndContext_{false}; // Used to mark end of ASCII multi-get request
  bool noReply_;
  bool replied_{false};
  bool isTraced_{false};

  uint64_t reqid_;
  struct AsciiState {
    std::shared_ptr<MultiOpParent> parent_;
    folly::Optional<folly::IOBuf> key_;
  };
  std::unique_ptr<AsciiState> asciiState_;

  template <class Reply>
  bool noReply(const Reply& r) const;
  bool noReply(const McLeaseGetReply& r) const;

  template <class Reply, class... Args>
  static typename std::enable_if<carbon::GetLike<
      RequestFromReplyType<Reply, RequestReplyPairs>>::value>::type
  replyImpl(McServerRequestContext&& ctx, Reply&& reply, Args&&... args);

  template <class Reply, class... Args>
  static typename std::enable_if<carbon::OtherThan<
      RequestFromReplyType<Reply, RequestReplyPairs>,
      carbon::GetLike<>>::value>::type
  replyImpl(McServerRequestContext&& ctx, Reply&& reply, Args&&... args);

  template <class Reply, class SessionType = McServerSession>
  static void replyImpl2(
      McServerRequestContext&& ctx,
      Reply&& reply,
      DestructorFunc destructor = nullptr,
      void* toDestruct = nullptr,
      bool flush = false);

  folly::Optional<folly::IOBuf>& asciiKey() {
    if (!asciiState_) {
      asciiState_ = std::make_unique<AsciiState>();
    }
    return asciiState_->key_;
  }
  bool hasParent() const {
    return asciiState_ && asciiState_->parent_;
  }
  MultiOpParent& parent() const {
    assert(hasParent());
    return *asciiState_->parent_;
  }
  bool isParentError() const;

  // Whether or not *this is used to mark the end of a multi-get request
  bool isEndContext() const {
    return isEndContext_;
  }

  /**
   * If reply is error, multi-op parent may inform this context that it will
   * assume responsibility for reporting the error. If so, this context should
   * not call McServerSession::reply. Returns true iff parent assumes
   * responsibility for reporting error. If true is returned, errorMessage is
   * moved to parent.
   */
  bool moveReplyToParent(
      carbon::Result result,
      uint32_t errorCode,
      std::string&& errorMessage) const;

  McServerRequestContext(const McServerRequestContext&) = delete;
  const McServerRequestContext& operator=(const McServerRequestContext&) =
      delete;

  /* Only McServerSession can create these */
  friend class McServerSession;
  friend class MultiOpParent;
  friend class WriteBuffer;
  McServerRequestContext(
      McServerSession& s,
      uint64_t r,
      bool nr = false,
      std::shared_ptr<MultiOpParent> parent = nullptr,
      bool isEndContext = false);
};

void markContextAsTraced(McServerRequestContext& ctx);

static_assert(
#if defined(__i386__)
    sizeof(McServerRequestContext) == 20,
#elif defined(__ARM_ARCH) && !defined(__aarch64__)
    sizeof(McServerRequestContext) == 24,
#else
    sizeof(McServerRequestContext) == 32,
#endif
    "Think twice before adding more fields to McServerRequestContext,"
    " doing so WILL have perf implications");

/**
 * McServerOnRequest is a polymorphic base class used as a callback
 * by AsyncMcServerWorker and McAsciiParser to hand off a request
 * to McrouterClient.
 *
 * The complexity in the implementation below is due to the fact that we
 * effectively need templated virtual member functions (which do not really
 * exist in C++).
 */
template <class RequestList>
class McServerOnRequestIf;

/**
 * OnRequest callback interface. This is an implementation detail.
 */
template <class Request>
class McServerOnRequestIf<List<Request>> {
 public:
  virtual void caretRequestReady(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBody,
      McServerRequestContext&& ctx) = 0;

  virtual void requestReady(McServerRequestContext&&, Request&&) {
    LOG(ERROR) << "requestReady() not implemented for request type "
               << Request::name;
  }

  virtual ~McServerOnRequestIf() = default;
};

template <class Request, class... Requests>
class McServerOnRequestIf<List<Request, Requests...>>
    : public McServerOnRequestIf<List<Requests...>> {
 public:
  using McServerOnRequestIf<List<Requests...>>::requestReady;

  virtual void requestReady(McServerRequestContext&&, Request&&) {
    LOG(ERROR) << "requestReady() not implemented for request type "
               << Request::name;
  }

  ~McServerOnRequestIf() override = default;
};

class McServerOnRequest : public McServerOnRequestIf<McRequestList> {
 public:
  explicit McServerOnRequest(std::string requestHandlerName)
      : requestHandlerName_(std::move(requestHandlerName)) {}

  /**
   * Return the name of the request handler being used in this
   * instance of McServerOnRequest.
   */
  const std::string& name() const {
    return requestHandlerName_;
  }

 private:
  std::string requestHandlerName_;
};

/**
 * Helper class to wrap user-defined callbacks in a correct virtual interface.
 * This is needed since we're mixing templates and virtual functions.
 */
template <class OnRequest, class RequestList = McRequestList>
class McServerOnRequestWrapper;

template <class OnRequest>
class McServerOnRequestWrapper<OnRequest, List<>> : public McServerOnRequest {
 public:
  using McServerOnRequest::requestReady;

  template <class... Args>
  explicit McServerOnRequestWrapper(Args&&... args)
      : McServerOnRequest(OnRequest::name),
        onRequest_(std::forward<Args>(args)...) {}

  void caretRequestReady(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBody,
      McServerRequestContext&& ctx) final;

  void dispatchTypedRequestIfDefined(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBody,
      McServerRequestContext&& ctx,
      std::true_type) {
    if (!onRequest_.dispatchTypedRequest(headerInfo, reqBody, std::move(ctx))) {
      throw std::runtime_error("dispatchTypedRequestIfDefined got bad request");
    }
  }

  void dispatchTypedRequestIfDefined(
      const CaretMessageInfo&,
      const folly::IOBuf& /* reqBody */,
      McServerRequestContext&&,
      std::false_type) {
    throw std::runtime_error("dispatchTypedRequestIfDefined got bad request");
  }

  template <class Request>
  void requestReadyImpl(
      McServerRequestContext&& ctx,
      Request&& req,
      std::true_type) {
    onRequest_.onRequest(std::move(ctx), std::move(req));
  }

  template <class Request>
  void
  requestReadyImpl(McServerRequestContext&& ctx, Request&&, std::false_type) {
    McServerRequestContext::reply(
        std::move(ctx), ReplyT<Request>(carbon::Result::LOCAL_ERROR));
  }

 protected:
  OnRequest onRequest_;
};

template <class OnRequest, class Request, class... Requests>
class McServerOnRequestWrapper<OnRequest, List<Request, Requests...>>
    : public McServerOnRequestWrapper<OnRequest, List<Requests...>> {
 public:
  using McServerOnRequestWrapper<OnRequest, List<Requests...>>::requestReady;

  template <class... Args>
  explicit McServerOnRequestWrapper(Args&&... args)
      : McServerOnRequestWrapper<OnRequest, List<Requests...>>(
            std::forward<Args>(args)...) {}

  void requestReady(McServerRequestContext&& ctx, Request&& req) final {
    this->requestReadyImpl(
        std::move(ctx),
        std::move(req),
        carbon::detail::CanHandleRequest::value<Request, OnRequest>());
  }
};

} // namespace memcache
} // namespace facebook

#include "McServerRequestContext-inl.h"
