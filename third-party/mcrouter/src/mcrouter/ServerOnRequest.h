/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <cassert>
#include <memory>

#ifndef MCROUTER_OSS_BUILD
#include "core_infra_security/thrift_authentication_module/ClientIdentifierHelper.h"
#endif

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/RequestAclChecker.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CaretHeader.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/McThriftContext.h"
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
      ExternalStatsHandler& statsHandler,
      bool requestAclCheckerEnable,
      bool enableKeyClientBinding = false)
      : client_(client),
        eventBase_(eventBase),
        retainSourceIp_(retainSourceIp),
        enablePassThroughMode_(enablePassThroughMode),
        remoteThread_(remoteThread),
        enableKeyClientBinding_(enableKeyClientBinding) {
    if constexpr (RouterInfo::useRequestAclChecker) {
      aclChecker_ = std::make_unique<RequestAclChecker>(
          statsHandler, requestAclCheckerEnable);
    }
  }

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
  void onRequestThrift(
      apache::thrift::HandlerCallbackPtr<typename Request::reply_type>&&
          callback,
      Request&& req) {
    if (HasKeyTrait<Request>::value) {
      req.key_ref()->update();
    }
    McThriftContext<ReplyT<Request>> ctx =
        McThriftContext<typename Request::reply_type>(std::move(callback));
    send(
        std::move(ctx),
        std::move(req),
        &McThriftContext<ReplyT<Request>>::reply);
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
    if constexpr (
        RouterInfo::useRequestAclChecker &&
        !folly::IsOneOf<Request, McDeleteRequest>::value) {
      if (aclChecker_->shouldReply(ctx, req)) {
        aclChecker_->template reply<Request>(std::forward<Callback>(ctx));
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

#ifndef MCROUTER_OSS_BUILD
    // Set hashed TLS client identities on request to propagate from proxy ->
    // memcache server only IF enableKeyClientBinding_ is enabled.
    if (FOLLY_UNLIKELY(enableKeyClientBinding_) &&
        ctxRef.getThriftRequestContext()) {
      auto mayBeHashedIdentities =
          core_infra_security::thrift_authentication_module::
              ClientIdentifierHelper::getTlsClientIdentifier(
                  *ctxRef.getThriftRequestContext());
      // if has valid hashed identity string, set it on the request
      if (mayBeHashedIdentities.hasValue() &&
          std::holds_alternative<std::string>(mayBeHashedIdentities.value())) {
        reqRef.setClientIdentifier(
            std::get<std::string>(mayBeHashedIdentities.value()));
      }
    }
#endif

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

    std::optional<folly::IPAddress> peerIp;
    if (retainSourceIp_ && (peerIp = ctxRef.getPeerSocketIPAddress())) {
      reqRef.setSourceIpAddr(*peerIp);
    }
    client_.send(reqRef, std::move(cb));
  }

 private:
  CarbonRouterClient<RouterInfo>& client_;
  folly::EventBase& eventBase_;
  const bool retainSourceIp_{false};
  const bool enablePassThroughMode_{false};
  const bool remoteThread_{false};
  std::unique_ptr<RequestAclChecker> aclChecker_;
  const bool enableKeyClientBinding_{false};
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
