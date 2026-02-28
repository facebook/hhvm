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

class RequestAclChecker {
 public:
  explicit RequestAclChecker(
      ExternalStatsHandler& statsHandler,
      bool requestAclCheckerEnable);

  /* Do not apply ACL checks on DELETE */
  template <class Request, class Callback>
  bool shouldReply(const Callback& ctx, const Request& req) const {
    static_assert(
        !folly::IsOneOf<Request, McDeleteRequest>::value,
        "RequestAclChecker should not be used for McDeleteRequest");
    if constexpr (folly::IsOneOf<Request, McExecRequest>::value) {
      return requestAclCheckerEnable_ &&
          isRefreshCommand(req.key_ref()->routingKey());
    } else {
      return requestAclCheckerEnable_ &&
          !requestAclCheckCb_(
                 ctx.getThriftRequestContext(), req.key_ref()->routingKey());
    }
  }

  template <class Request, class Callback>
  void reply(Callback&& ctx) const {
    static_assert(
        !folly::IsOneOf<Request, McDeleteRequest>::value,
        "RequestAclChecker should not be used for McDeleteRequest");
    if constexpr (folly::IsOneOf<Request, McExecRequest>::value) {
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
    } else {
      // TODO: Change this error code when T67679592 is done
      auto reply = ReplyT<Request>{carbon::Result::BAD_FLAGS};
      reply.message_ref() = "Permission Denied";
      Callback::reply(std::forward<Callback>(ctx), std::move(reply));
    }
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
