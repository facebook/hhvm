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
#include <folly/fibers/FiberManager.h>
#include <folly/fibers/WhenN.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

/**
 * This route handle changes behavior based on Migration mode. Each request is
 * assigned a migration time based on the hash of its key in order to migrate
 * smoothly over time. A key's migration time is calculated as start_time +
 * hash(key) % (2 * interval).
 * 1. Before the migration starts, sends all requests to from_ route
 * handle.
 * 2. Between start time and the key's migration time, sends all requests except
 * for deletes to from_ route handle and sends all delete requests to both from_
 * and to_ route handle. For delete requests, returns reply from
 * worst among two replies.
 * 3. Between the key's migration time and (start_time + 2*interval), sends all
 * requests except for deletes to to_ route handle and sends all delete requests
 * to both from_ and to_ route handle. For delete requests, returns
 * reply from worst among two replies.
 * 4. After (start_time + 2*interval), sends all requests to to_ route handle.
 */
template <class RouteHandleIf, typename TimeProvider>
class MigrateRoute {
 public:
  static std::string routeName() {
    return "migrate";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    auto mask = routeMask(tp_(), req);
    if (mask & kFromMask) {
      if (t(*from_, req)) {
        return true;
      }
    }
    if (mask & kToMask) {
      if (t(*to_, req)) {
        return true;
      }
    }
    return false;
  }

  MigrateRoute(
      std::shared_ptr<RouteHandleIf> fh,
      std::shared_ptr<RouteHandleIf> th,
      time_t start_time_sec,
      time_t interval_sec,
      TimeProvider tp)
      : from_(std::move(fh)),
        to_(std::move(th)),
        startTimeSec_(start_time_sec),
        intervalSec_(interval_sec),
        tp_(tp) {
    assert(from_ != nullptr);
    assert(to_ != nullptr);
  }

  McLeaseSetReply route(const McLeaseSetRequest& req) const {
    const time_t now = tp_();
    auto mask = routeMask(now, req);
    switch (mask) {
      case kFromMask:
        return from_->route(req);
      case kToMask:
      default:
        McLeaseSetReply reply = to_->route(req);
        if (*reply.result_ref() != carbon::Result::STORED &&
            now < (migrationTime(req) + 10)) {
          // Send a lease invalidation to from_ if the lease-set failed and we
          // recently migrated to to_. This helps ensure that servers in the old
          // pool don't accumulate unfulfilled lease tokens.
          auto leaseInvalidation =
              std::make_unique<McLeaseSetRequest>(req.key_ref()->fullKey());
          leaseInvalidation->exptime_ref() = -1;
          leaseInvalidation->leaseToken_ref() = *req.leaseToken_ref();
          folly::fibers::addTask(
              [rh = from_, leaseInvalidation = std::move(leaseInvalidation)]() {
                rh->route(*leaseInvalidation);
              });
        }
        return reply;
    }
  }

  template <class Request>
  ReplyT<Request> route(
      const Request& req,
      carbon::OtherThanT<Request, McLeaseSetRequest> = 0) const {
    using Reply = ReplyT<Request>;

    auto mask = routeMask(tp_(), req);

    switch (mask) {
      case kFromMask:
        return from_->route(req);
      case kToMask:
        return to_->route(req);
      default: {
        auto& from = from_;
        auto& to = to_;
        std::function<Reply()> fs[2]{
            [&req, &from]() { return from->route(req); },
            [&req, &to]() { return to->route(req); }};

        folly::Optional<Reply> reply;
        folly::fibers::forEach(fs, fs + 2, [&reply](size_t, Reply newReply) {
          if (!reply ||
              worseThan(*newReply.result_ref(), *reply.value().result_ref())) {
            reply = std::move(newReply);
          }
        });
        return std::move(reply.value());
      }
    }
  }

 private:
  static constexpr int kFromMask = 1;
  static constexpr int kToMask = 2;

  const std::shared_ptr<RouteHandleIf> from_;
  const std::shared_ptr<RouteHandleIf> to_;
  time_t startTimeSec_;
  time_t intervalSec_;
  const TimeProvider tp_;

  template <class Request>
  int routeMask(time_t now, const Request&, carbon::DeleteLikeT<Request> = 0)
      const {
    if (now < startTimeSec_) {
      return kFromMask;
    }

    if (now < (startTimeSec_ + 2 * intervalSec_)) {
      return kFromMask | kToMask;
    }

    /* else */
    return kToMask;
  }

  template <class Request>
  int routeMask(
      time_t now,
      const Request& req,
      carbon::OtherThanT<Request, carbon::DeleteLike<>> = 0) const {
    if (now < migrationTime(req)) {
      return kFromMask;
    } else {
      return kToMask;
    }
  }

  // Returns the timestamp when traffic switches between from_ and to_.
  template <class Request>
  time_t migrationTime(const Request& req) const {
    return startTimeSec_ + intervalSec_ +
        req.key_ref()->routingKeyHash() % intervalSec_;
  }
};
} // namespace memcache
} // namespace facebook
