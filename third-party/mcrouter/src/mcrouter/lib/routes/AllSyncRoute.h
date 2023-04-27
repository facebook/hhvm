/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <folly/Optional.h>
#include <folly/fibers/ForEach.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/fbi/cpp/FuncGenerator.h"

namespace facebook {
namespace memcache {

/**
 * Sends the same request to all child route handles.
 * Collects all the replies and responds with the "most awful" reply.
 */
template <class RouteHandleIf>
class AllSyncRoute {
 public:
  static std::string routeName() {
    return "all-sync";
  }

  explicit AllSyncRoute(std::vector<std::shared_ptr<RouteHandleIf>> rh)
      : children_(std::move(rh)) {
    assert(!children_.empty());
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(children_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    using Reply = ReplyT<Request>;

    const auto& children = children_;
    auto fs = makeFuncGenerator(
        [&req, &children](size_t id) { return children[id]->route(req); },
        children_.size());

    folly::Optional<Reply> reply;
    folly::fibers::forEach(
        fs.begin(), fs.end(), [&reply](size_t /* id */, Reply newReply) {
          if (!reply ||
              worseThan(*newReply.result_ref(), *reply.value().result_ref())) {
            reply = std::move(newReply);
          }
        });
    return std::move(reply.value());
  }

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> children_;
};
} // namespace memcache
} // namespace facebook
