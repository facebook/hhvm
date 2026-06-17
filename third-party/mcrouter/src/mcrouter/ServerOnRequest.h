/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>
#include <cassert>
#include <memory>

#ifndef MCROUTER_OSS_BUILD
#include "configerator/structs/infrasec/if/gen-cpp2/acl_constants.h"
#include "core_infra_security/thrift_authentication_module/ClientIdentifierHelper.h"
#include "crypto/cat/cpp/protocol/CryptoAuthTokenRetriever.h"
#include "infrasec/authorization/IdentityUtil.h"
#include "mcrouter/facebook/granular_acl/detail/CryptoAuthTokenHelper.h"
#include "ucache/protocol/UcacheReplySourceTypes.h"
#endif

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/RequestAclChecker.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/carbon/MessageCommon.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CaretHeader.h"
#include "mcrouter/lib/network/McThriftContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

#ifndef MCROUTER_OSS_BUILD
namespace detail {
// Non-lookaside KCB: returns the kcb_identity thrift header value iff the
// *verified* request CAT vouches a MEMCACHE_ID identity equal to it (mirrors
// ucache KeyClientBinder).

inline std::optional<std::string> nonLookasideKcbIdentity(
    const apache::thrift::Cpp2RequestContext& ctx,
    const ::facebook::mcrouter::granular_acl::detail::CryptoAuthTokenHelperBase&
        catHelper) {
  const auto* header = ctx.getHeader();
  if (!header) {
    return std::nullopt;
  }
  // THeader's read-header map holds a single value per key, so find() yields
  // the one stored value. There is no multi-value ambiguity to exploit: the
  // returned identity is only ever bound below after a verified CAT MEMCACHE_ID
  // matches it, so an unvouched header value can never be propagated.
  const auto& headers = header->getHeaders();
  const auto it =
      headers.find(std::string(carbon::MessageCommon::kKcbIdentityHeader));
  if (it == headers.end() || it->second.empty()) {
    return std::nullopt;
  }

  // Verify the CAT and pull out the identities it actually vouches for.
  ::facebook::mcrouter::granular_acl::detail::CryptoAuthTokenHelperBase::
      MaybeIdentitiesT catIds;
  try {
    catIds = catHelper.getIdentitiesFromCtx(&ctx);
  } catch (const std::exception& e) {
    XLOG_EVERY_MS(ERR, 1000)
        << "Non-lookaside KCB: CAT verification threw: " << e.what();
    return std::nullopt;
  }
  if (!catIds.has_value()) {
    return std::nullopt;
  }

  const auto& memcacheIdType =
      infrasec::authorization::acl_constants::MEMCACHE_ID();
  for (const auto& id : catIds.value()) {
    if (id.id_type_ref().value() == memcacheIdType &&
        id.id_data_ref().value() == it->second) {
      return it->second;
    }
  }
  return std::nullopt;
}
} // namespace detail
#endif

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
      bool enableKeyClientBinding = false,
      bool enableReplySource = false,
      bool enableKeyClientBindingNonLookaside = false,
      std::string keyClientBindingNonLookasideVerifier = "ucache")
      : client_(client),
        eventBase_(eventBase),
        retainSourceIp_(retainSourceIp),
        enablePassThroughMode_(enablePassThroughMode),
        remoteThread_(remoteThread),
        enableKeyClientBinding_(enableKeyClientBinding),
        enableReplySource_(enableReplySource),
        enableKeyClientBindingNonLookaside_(enableKeyClientBindingNonLookaside),
        keyClientBindingNonLookasideVerifier_(
            std::move(keyClientBindingNonLookasideVerifier)) {
    if constexpr (RouterInfo::useRequestAclChecker) {
      aclChecker_ = std::make_unique<RequestAclChecker>(
          statsHandler, requestAclCheckerEnable);
    }
#ifndef MCROUTER_OSS_BUILD
    // Non-lookaside KCB needs to verify the request CAT to surface its
    // MEMCACHE_ID identities (the proxy only has the TLS peer identity
    // otherwise). The verifier identity must match the identity tokens are
    // minted for (the proxy's own SR identity).
    if (enableKeyClientBindingNonLookaside_) {
      catHelper_ = std::make_unique<
          ::facebook::mcrouter::granular_acl::detail::CryptoAuthTokenHelper>(
          infrasec::authorization::IdentityUtil::makeServiceIdentity(
              keyClientBindingNonLookasideVerifier_));
    }
#endif
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
    reply.value() =
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
    // Propagate client identity from proxy -> memcache server when KCB enabled.
    if (FOLLY_UNLIKELY(enableKeyClientBinding_) &&
        ctxRef.getThriftRequestContext()) {
      const auto* thriftCtx = ctxRef.getThriftRequestContext();

      if (enableKeyClientBindingNonLookaside_) {
        // Non-lookaside KCB is authoritative: bind the client identifier only
        // when a verified MEMCACHE_ID vouches for the kcb_identity header. Do
        // NOT fall back to the hashed TLS identity path.
        if (catHelper_) {
          if (auto kcbIdentity =
                  detail::nonLookasideKcbIdentity(*thriftCtx, *catHelper_)) {
            reqRef.setClientIdentifier(*kcbIdentity);
          }
        }
      } else {
        // Legacy KCB: hashed TLS client identities.
        auto mayBeHashedIdentities =
            core_infra_security::thrift_authentication_module::
                ClientIdentifierHelper::getClientIdentityHash(*thriftCtx);
        if (mayBeHashedIdentities.has_value() &&
            mayBeHashedIdentities->hasValue() &&
            std::holds_alternative<std::string>(
                mayBeHashedIdentities->value())) {
          reqRef.setClientIdentifier(
              std::get<std::string>(mayBeHashedIdentities->value()));
        }
      }

      auto serializedCat =
          facebook::cryptocat::protocol::CryptoAuthTokenRetriever::
              getUnverifiedSerializedTokensFromHeader(*thriftCtx);
      if (serializedCat.has_value()) {
        reqRef.setCryptoAuthToken(std::string{serializedCat.value()});
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
#ifndef MCROUTER_OSS_BUILD
      if constexpr (HasReplySourceBitMaskTrait<ReplyT<Request>>::value) {
        if (enableReplySource_) {
          auto replySourceBitMask = *reply.replySourceBitMask_ref();
          if (replySourceBitMask) {
            reply.replySourceBitMask_ref() =
                (reply.replySourceBitMask_ref().value() |
                 (1U << static_cast<uint32_t>(
                      facebook::ucache::proto::UcacheReplySourceTypes::
                          McrouterStandalone)));
          } else {
            reply.replySourceBitMask_ref() =
                (1U << static_cast<uint32_t>(
                     facebook::ucache::proto::UcacheReplySourceTypes::
                         McrouterStandalone));
          }
        }
      }
#endif
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
  const bool enableReplySource_{false};
  const bool enableKeyClientBindingNonLookaside_{false};
  const std::string keyClientBindingNonLookasideVerifier_{"ucache"};
#ifndef MCROUTER_OSS_BUILD
  // Verifies request CATs for the non-lookaside KCB path. Null unless
  // enableKeyClientBindingNonLookaside_ is set.
  std::unique_ptr<
      ::facebook::mcrouter::granular_acl::detail::CryptoAuthTokenHelper>
      catHelper_;
#endif
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
