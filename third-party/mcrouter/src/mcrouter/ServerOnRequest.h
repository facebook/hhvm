/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <cassert>

#include <mcrouter/lib/network/gen/Memcache.h>
#include <mcrouter/lib/network/gen/gen-cpp2/Memcache.h>
#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/RequestAclChecker.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CaretHeader.h"
#include "mcrouter/lib/network/McCallbackUtils.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class Callback, class Request>
struct ServerRequestContext {
  Callback ctx;
  Request req;
  folly::IOBuf reqBuffer;

  ServerRequestContext(
      Callback&& ctx_,
      Request&& req_,
      const folly::IOBuf* reqBuffer_)
      : ctx(std::move(ctx_)),
        req(std::move(req_)),
        reqBuffer(reqBuffer_ ? reqBuffer_->cloneAsValue() : folly::IOBuf()) {}
};

template <class RouterInfo>
class ServerOnRequest {
 public:
  template <class Callback, class Request>
  using ReplyFunction =
      void (*)(Callback&& ctx, ReplyT<Request>&& reply, bool flush);

  template <class... Args>
  ServerOnRequest(
      CarbonRouterClient<RouterInfo>& client,
      folly::EventBase& eventBase,
      bool retainSourceIp,
      bool enablePassThroughMode,
      bool remoteThread,
      Args&&... args)
      : client_(client),
        eventBase_(eventBase),
        retainSourceIp_(retainSourceIp),
        enablePassThroughMode_(enablePassThroughMode),
        remoteThread_(remoteThread),
        aclChecker_(std::forward<Args>(args)...) {}

  template <class Reply, class Callback>
  void sendReply(Callback&& ctx, Reply&& reply) {
    if (remoteThread_) {
      return eventBase_.runInEventBaseThread(
          [ctx = std::move(ctx), reply = std::move(reply)]() mutable {
            Callback::reply(std::move(ctx), std::move(reply));
          });
    } else {
      return Callback::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Request, class Callback>
  void onRequest(
      Callback&& ctx,
      Request&& req,
      const CaretMessageInfo* headerInfo,
      const folly::IOBuf* reqBuffer) {
    using Reply = ReplyT<Request>;
    send(
        std::move(ctx),
        std::move(req),
        &Callback::template reply<Reply>,
        headerInfo,
        reqBuffer);
  }

  template <class Request>
  void onRequestThrift(McThriftCallback<ReplyT<Request>>&& ctx, Request&& req) {
    if (HasKeyTrait<Request>::value) {
      req.key_ref()->update();
    }
    send(
        std::move(ctx),
        std::move(req),
        &McThriftCallback<ReplyT<Request>>::reply);
  }

  template <class Request, class Callback>
  void onRequest(Callback&& ctx, Request&& req) {
    using Reply = ReplyT<Request>;
    send(std::move(ctx), std::move(req), &Callback::template reply<Reply>);
  }

  template <class Callback>
  void onRequest(Callback&& ctx, McVersionRequest&&) {
    McVersionReply reply(carbon::Result::OK);
    reply.value_ref() =
        folly::IOBuf(folly::IOBuf::COPY_BUFFER, MCROUTER_PACKAGE_STRING);

    sendReply(std::move(ctx), std::move(reply));
  }

  template <class Callback>
  void onRequest(Callback&& ctx, McQuitRequest&&) {
    sendReply(std::move(ctx), McQuitReply(carbon::Result::OK));
  }

  template <class Callback>
  void onRequest(Callback&& ctx, McShutdownRequest&&) {
    sendReply(std::move(ctx), McShutdownReply(carbon::Result::OK));
  }

  template <class Callback, class Request>
  void send(
      Callback&& ctx,
      Request&& req,
      ReplyFunction<Callback, Request> replyFn,
      const CaretMessageInfo* headerInfo = nullptr,
      const folly::IOBuf* reqBuffer = nullptr) {
    /*
     * If we don't have an AclChecker specialized
     * for this router, don't bother running Acl checks
     */
    if constexpr (decltype(aclChecker_)::value) {
      if (aclChecker_(std::move(ctx), std::move(req))) {
        return;
      }
    }
    // We just reuse buffers iff:
    //  1) enablePassThroughMode_ is true.
    //  2) headerInfo is not NULL.
    //  3) reqBuffer is not NULL.
    const folly::IOBuf* reusableRequestBuffer =
        (enablePassThroughMode_ && headerInfo) ? reqBuffer : nullptr;

    auto rctx = std::make_unique<ServerRequestContext<Callback, Request>>(
        std::move(ctx), std::move(req), reusableRequestBuffer);
    auto& reqRef = rctx->req;
    auto& ctxRef = rctx->ctx;

    // if we are reusing the request buffer, adjust the start offset and set
    // it to the request.
    if (reusableRequestBuffer) {
      auto& reqBufferRef = rctx->reqBuffer;
      reqBufferRef.trimStart(headerInfo->headerSize);
      reqRef.setSerializedBuffer(reqBufferRef);
    }

    auto cb = [this, sctx = std::move(rctx), replyFn = std::move(replyFn)](
                  const Request&, ReplyT<Request>&& reply) mutable {
      if (remoteThread_) {
        eventBase_.runInEventBaseThread([sctx = std::move(sctx),
                                         replyFn = std::move(replyFn),
                                         reply = std::move(reply)]() mutable {
          replyFn(std::move(sctx->ctx), std::move(reply), false /* flush */);
        });
      } else {
        replyFn(std::move(sctx->ctx), std::move(reply), false /* flush */);
      }
    };

    folly::Optional<std::string> peerIp;
    if (retainSourceIp_ && (peerIp = ctxRef.getPeerSocketAddressStr())) {
      client_.send(reqRef, std::move(cb), *peerIp);
    } else {
      client_.send(reqRef, std::move(cb));
    }
  }

 private:
  CarbonRouterClient<RouterInfo>& client_;
  folly::EventBase& eventBase_;
  const bool retainSourceIp_{false};
  const bool enablePassThroughMode_{false};
  const bool remoteThread_{false};
  const RequestAclChecker<RouterInfo> aclChecker_;
};

template <class RouterInfo>
class ServerOnRequestThrift : public thrift::MemcacheSvIf {
 public:
  ServerOnRequestThrift(
      std::unordered_map<
          folly::EventBase*,
          std::shared_ptr<ServerOnRequest<RouterInfo>>> serverOnRequestMap)
      : serverOnRequestMap_(std::move(serverOnRequestMap)) {}

  // thrift
  void async_eb_mcVersion(
      std::unique_ptr<apache::thrift::HandlerCallback<McVersionReply>> callback,
      const McVersionRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }
  void async_eb_mcGet(
      std::unique_ptr<apache::thrift::HandlerCallback<McGetReply>> callback,
      const McGetRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcSet(
      std::unique_ptr<apache::thrift::HandlerCallback<McSetReply>> callback,
      const McSetRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcDelete(
      std::unique_ptr<apache::thrift::HandlerCallback<McDeleteReply>> callback,
      const McDeleteRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcLeaseGet(
      std::unique_ptr<apache::thrift::HandlerCallback<McLeaseGetReply>>
          callback,
      const McLeaseGetRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcLeaseSet(
      std::unique_ptr<apache::thrift::HandlerCallback<McLeaseSetReply>>
          callback,
      const McLeaseSetRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcAdd(
      std::unique_ptr<apache::thrift::HandlerCallback<McAddReply>> callback,
      const McAddRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcReplace(
      std::unique_ptr<apache::thrift::HandlerCallback<McReplaceReply>> callback,
      const McReplaceRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcGets(
      std::unique_ptr<apache::thrift::HandlerCallback<McGetsReply>> callback,
      const McGetsRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcCas(
      std::unique_ptr<apache::thrift::HandlerCallback<McCasReply>> callback,
      const McCasRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcIncr(
      std::unique_ptr<apache::thrift::HandlerCallback<McIncrReply>> callback,
      const McIncrRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcDecr(
      std::unique_ptr<apache::thrift::HandlerCallback<McDecrReply>> callback,
      const McDecrRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcMetaget(
      std::unique_ptr<apache::thrift::HandlerCallback<McMetagetReply>> callback,
      const McMetagetRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcAppend(
      std::unique_ptr<apache::thrift::HandlerCallback<McAppendReply>> callback,
      const McAppendRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcPrepend(
      std::unique_ptr<apache::thrift::HandlerCallback<McPrependReply>> callback,
      const McPrependRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcTouch(
      std::unique_ptr<apache::thrift::HandlerCallback<McTouchReply>> callback,
      const McTouchRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcFlushRe(
      std::unique_ptr<apache::thrift::HandlerCallback<McFlushReReply>> callback,
      const McFlushReRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcFlushAll(
      std::unique_ptr<apache::thrift::HandlerCallback<McFlushAllReply>>
          callback,
      const McFlushAllRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcGat(
      std::unique_ptr<apache::thrift::HandlerCallback<McGatReply>> callback,
      const McGatRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcGats(
      std::unique_ptr<apache::thrift::HandlerCallback<McGatsReply>> callback,
      const McGatsRequest& request) override final {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  void async_eb_mcExec(
      std::unique_ptr<apache::thrift::HandlerCallback<McExecReply>> callback,
      const McExecRequest& request) {
    onRequestThriftHelper<std::remove_reference_t<decltype(request)>>(
        request, std::move(callback));
  }

  // Return this factory instead of MemcacheAsyncProcessor from getProcessor(),
  // so that we don't use the default statically registered handlers
  class MemcacheAsyncProcessorCustomHandlers
      : public thrift::MemcacheAsyncProcessor {
   public:
    explicit MemcacheAsyncProcessorCustomHandlers(thrift::MemcacheSvIf* svif)
        : MemcacheAsyncProcessor(svif) {
      clearEventHandlers();
    }
  };
  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor()
      override final {
    return std::make_unique<MemcacheAsyncProcessorCustomHandlers>(this);
  }

 private:
  std::unordered_map<
      folly::EventBase*,
      std::shared_ptr<ServerOnRequest<RouterInfo>>>
      serverOnRequestMap_;
  static thread_local ServerOnRequest<RouterInfo>* serverOnRequest_;

  template <class Request>
  void onRequestThriftHelper(
      const Request& request,
      std::unique_ptr<apache::thrift::HandlerCallback<ReplyT<Request>>>
          callback) {
    getServerOnRequest(callback->getEventBase())
        ->onRequestThrift(
            McThriftCallback<ReplyT<Request>>(std::move(callback)),
            std::move(const_cast<std::remove_const_t<Request>&>(request)));
  }

  // Returns the ServerOnRequest* associated with this evb
  ServerOnRequest<RouterInfo>* getServerOnRequest(folly::EventBase* evb) {
    if (serverOnRequest_ == nullptr) {
      auto it = serverOnRequestMap_.find(evb);
      CHECK(it != serverOnRequestMap_.end());
      serverOnRequest_ = it->second.get();
    }
    return serverOnRequest_;
  }
};

template <class RouterInfo>
thread_local ServerOnRequest<RouterInfo>*
    ServerOnRequestThrift<RouterInfo>::serverOnRequest_{nullptr};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
