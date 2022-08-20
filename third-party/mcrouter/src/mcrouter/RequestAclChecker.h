/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/ExternalStatsHandler.h"
#include "mcrouter/StandaloneConfig.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/carbon/Result.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/* Not specialized for all routers. Does nothing */
template <class RouterInfo>
class RequestAclChecker : public std::false_type {
 public:
  template <class... Args>
  explicit RequestAclChecker(Args&&...) {}
};

/* Memcache specialization. Initializes ACL checker */
template <>
class RequestAclChecker<MemcacheRouterInfo> : public std::true_type {
 public:
  explicit RequestAclChecker(
      ExternalStatsHandler& statsHandler,
      bool requestAclCheckerEnable);

  /*
   * If the status is REPLIED, the caller in ServerOnRequest.h will stop further
   * processing of the request.
   */
  enum Status { REPLIED = true, NOT_REPLIED = false };

  /* Do not apply ACL checks on DELETE */
  template <class Request, class Callback>
  typename std::
      enable_if_t<folly::IsOneOf<Request, McDeleteRequest>::value, bool>
      operator()(Callback&&, Request&&) const {
    return Status::NOT_REPLIED;
  }

  /* An exec command to refresh ACL checker */
  template <class Request, class Callback>
  typename std::enable_if_t<folly::IsOneOf<Request, McExecRequest>::value, bool>
  operator()(Callback&& ctx, Request&& req) const {
    if (requestAclCheckerEnable_ &&
        isRefreshCommand(req.key_ref()->routingKey())) {
      McExecReply reply(carbon::Result::BAD_COMMAND);
      /* Only allow requests from localhost */
      if (isLocalRequest(ctx.getPeerSocketAddress())) {
        refreshMemcacheServerRequestAclChecker();
        reply.result_ref() = carbon::Result::OK;
        reply.response_ref() = "Initializing new Prefix ACL handler.";
      } else {
        reply.response_ref() =
            "Prefix ACL refresh can only be called from localhost";
      }
      Callback::reply(std::forward<Callback>(ctx), std::move(reply));
      return Status::REPLIED;
    }
    return Status::NOT_REPLIED;
  }

  /* Anything but DELETE and EXEC, apply ACL checks on the key */
  template <class Request, class Callback>
  typename std::enable_if_t<
      !folly::IsOneOf<Request, McDeleteRequest, McExecRequest>::value,
      bool>
  operator()(Callback&& ctx, Request&& req) const {
    if (requestAclCheckerEnable_ &&
        !requestAclCheckCb_(
            ctx.getThriftRequestContext(), req.key_ref()->routingKey())) {
      // TODO: Change this error code when T67679592 is done
      auto reply = ReplyT<Request>{carbon::Result::BAD_FLAGS};
      reply.message_ref() = "Permission Denied";
      Callback::reply(std::move(ctx), std::move(reply));
      return Status::REPLIED;
    }
    return Status::NOT_REPLIED;
  }

 private:
  MemcacheRequestAclCheckerCallback initRequestAclCheckCbIfEnabled(
      ExternalStatsHandler& statsHandler) const noexcept;

  /* Determines if the command is of the form "refresh prefix-acl" */
  static bool isRefreshCommand(const folly::StringPiece cmd) noexcept;

  /* Determines if this request is coming from localhost */
  static bool isLocalRequest(
      const folly::Optional<struct sockaddr_storage>& address) noexcept;

  const bool requestAclCheckerEnable_;
  const MemcacheRequestAclCheckerCallback requestAclCheckCb_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
