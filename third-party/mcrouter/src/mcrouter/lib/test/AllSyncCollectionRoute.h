/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/routes/CollectionRoute.h"

namespace facebook {
namespace memcache {

template <class Request, class... Args>
class AllSyncCollector : public Collector<Request, AllSyncCollector, Args...> {
  using Reply = ReplyT<Request>;
  using ParentCollector = Collector<Request, AllSyncCollector>;

 public:
  AllSyncCollector() {}

  folly::Optional<Reply> initialReplyImpl() const {
    return folly::none;
  }

  folly::Optional<Reply> iterImpl(const Reply& reply) {
    if (!finalReply_ ||
        worseThan(*reply.result_ref(), *finalReply_.value().result_ref())) {
      finalReply_ = reply;
    }

    return folly::none;
  }

  Reply finalReplyImpl() const {
    return finalReply_.value();
  }

 private:
  folly::Optional<Reply> finalReply_;
};

template <class RouterInfo, class... Args>
class AllSyncCollectionRoute
    : public CollectionRoute<RouterInfo, AllSyncCollector, Args...> {
 public:
  std::string routeName() const {
    return "AllSyncCollectionRoute";
  }
  explicit AllSyncCollectionRoute(
      const std::vector<typename RouterInfo::RouteHandlePtr> children,
      const Args&... arg)
      : CollectionRoute<RouterInfo, AllSyncCollector, Args...>(
            children,
            arg...) {
    assert(!children.empty());
  }
};

} // end namespace memcache
} // end namespace facebook
