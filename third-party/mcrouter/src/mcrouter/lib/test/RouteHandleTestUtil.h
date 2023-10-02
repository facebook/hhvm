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
#include <folly/Traits.h>
#include <folly/fibers/FiberManager.h>
#include <folly/fibers/SimpleLoopController.h>
#include <folly/fibers/WhenN.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleProviderIf.h"
#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {

namespace detail {

template <class M>
typename std::enable_if<HasFlagsTrait<M>::value>::type testSetFlags(
    M& message,
    uint64_t flags) {
  message.flags_ref() = flags;
}
template <class M>
typename std::enable_if<!HasFlagsTrait<M>::value>::type testSetFlags(
    M&,
    uint64_t) {}

template <class Reply>
typename std::enable_if<HasValueTrait<Reply>::value, void>::type setReplyValue(
    Reply& reply,
    const std::string& val) {
  reply.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, val);
}
template <class Reply>
typename std::enable_if<!HasValueTrait<Reply>::value, void>::type setReplyValue(
    Reply&,
    const std::string& /* val */) {}
} // namespace detail

/**
 * Very basic route handle provider to be used in unit tests only.
 */
template <class RouteHandleIf>
class SimpleRouteHandleProvider : public RouteHandleProviderIf<RouteHandleIf> {
 public:
  std::vector<std::shared_ptr<RouteHandleIf>> create(
      RouteHandleFactory<RouteHandleIf>& /* factory */,
      folly::StringPiece type,
      const folly::dynamic& json) override {
    std::vector<std::shared_ptr<RouteHandleIf>> result;

    const folly::dynamic* jsonPtr = nullptr;
    if (type == "Pool") {
      jsonPtr = json.get_ptr("servers");
    } else {
      jsonPtr = &json;
    }

    if (jsonPtr->isArray()) {
      for (const auto& child : *jsonPtr) {
        (void)child;
        result.push_back(mcrouter::createNullRoute<RouteHandleIf>());
      }
    } else {
      result.push_back(mcrouter::createNullRoute<RouteHandleIf>());
    }
    return result;
  }

  const folly::dynamic& parsePool(const folly::dynamic& json) override {
    return json;
  }
};

struct GetRouteTestData {
  carbon::Result result_;
  std::string value_;
  int64_t flags_;
  int16_t appSpecificErrorCode_;

  GetRouteTestData()
      : result_(carbon::Result::UNKNOWN),
        value_(std::string()),
        flags_(0),
        appSpecificErrorCode_(0) {}

  GetRouteTestData(
      carbon::Result result,
      const std::string& value,
      int64_t flags = 0,
      int16_t appSpecificErrorCode = 0)
      : result_(result),
        value_(value),
        flags_(flags),
        appSpecificErrorCode_(appSpecificErrorCode) {}
};

struct UpdateRouteTestData {
  carbon::Result result_;
  uint64_t flags_;

  UpdateRouteTestData() : result_(carbon::Result::UNKNOWN), flags_(0) {}

  explicit UpdateRouteTestData(carbon::Result result, uint64_t flags = 0)
      : result_(result), flags_(flags) {}
};

struct DeleteRouteTestData {
  carbon::Result result_;

  explicit DeleteRouteTestData(carbon::Result result = carbon::Result::UNKNOWN)
      : result_(result) {}
};

template <class RouteHandleIf>
struct RecordingRoute;

template <class RouteHandleIf>
struct TestHandleImpl {
  std::shared_ptr<RouteHandleIf> rh;

  std::vector<std::string> saw_keys;

  std::vector<std::string> sawValues;

  std::vector<uint32_t> sawExptimes;

  std::vector<std::string> sawOperations;

  std::vector<int64_t> sawLeaseTokensSet;

  std::vector<uint64_t> sawFlags;

  std::vector<uint64_t> sawBucketIds;

  std::vector<uint64_t> sawQueryTags;

  std::vector<std::string> distributionRegionInFiber;

  bool isTko;

  bool isPaused;

  std::vector<folly::fibers::Promise<void>> promises_;

  std::string sawLogs;

  folly::Optional<std::function<carbon::Result(std::string reqKey)>>
      resultGenerator_;
  folly::Optional<std::function<std::string()>> logCapturer;

  explicit TestHandleImpl(GetRouteTestData td)
      : rh(makeRouteHandle<RouteHandleIf, RecordingRoute>(
            td,
            UpdateRouteTestData(),
            DeleteRouteTestData(),
            this)),
        isTko(false),
        isPaused(false) {}

  explicit TestHandleImpl(UpdateRouteTestData td)
      : rh(makeRouteHandle<RouteHandleIf, RecordingRoute>(
            GetRouteTestData(),
            td,
            DeleteRouteTestData(),
            this)),
        isTko(false),
        isPaused(false) {}

  explicit TestHandleImpl(DeleteRouteTestData td)
      : rh(makeRouteHandle<RouteHandleIf, RecordingRoute>(
            GetRouteTestData(),
            UpdateRouteTestData(),
            td,
            this)),
        isTko(false),
        isPaused(false) {}

  TestHandleImpl(
      GetRouteTestData g_td,
      UpdateRouteTestData u_td,
      DeleteRouteTestData d_td)
      : rh(makeRouteHandle<RouteHandleIf, RecordingRoute>(
            g_td,
            u_td,
            d_td,
            this)),
        isTko(false),
        isPaused(false) {}

  void setTko() {
    isTko = true;
  }

  void unsetTko() {
    isTko = false;
  }

  void pause() {
    isPaused = true;
  }

  void unpause() {
    folly::fibers::addTask([this]() {
      for (auto& promise : promises_) {
        promise.setValue();
      }
      promises_.clear();
    });
  }

  void wait() {
    assert(isPaused);
    folly::fibers::await([this](folly::fibers::Promise<void> promise) {
      promises_.push_back(std::move(promise));
    });
    isPaused = false;
  }

  void setResultGenerator(
      std::function<carbon::Result(std::string reqKey)> getResult) {
    resultGenerator_ = std::move(getResult);
  }

  void resetResultGenerator() {
    resultGenerator_ = folly::none;
  }

  void enableLogCapture(std::function<std::string()> fun) {
    logCapturer = std::move(fun);
  }
};

template <class Request, typename = std::void_t<>>
struct HasBucketId : public std::false_type {};

template <class Request>
struct HasBucketId<
    Request,
    std::void_t<decltype(std::declval<Request>().bucketId_ref())>>
    : public std::true_type {};

/* Records all the keys we saw */
template <class RouteHandleIf>
struct RecordingRoute {
  static std::string routeName() {
    return "test";
  }

  GetRouteTestData dataGet_;
  UpdateRouteTestData dataUpdate_;
  DeleteRouteTestData dataDelete_;
  TestHandleImpl<RouteHandleIf>* h_;

  RecordingRoute(
      GetRouteTestData g_td,
      UpdateRouteTestData u_td,
      DeleteRouteTestData d_td,
      TestHandleImpl<RouteHandleIf>* h)
      : dataGet_(g_td), dataUpdate_(u_td), dataDelete_(d_td), h_(h) {}

  template <class Request>
  ReplyT<Request> route(
      const Request& req,
      carbon::OtherThanT<Request, McLeaseSetRequest> = 0) {
    return routeInternal(req);
  }

  McLeaseSetReply route(const McLeaseSetRequest& req) {
    h_->sawLeaseTokensSet.push_back(*req.leaseToken_ref());
    return routeInternal(req);
  }

  template <class Request>
  std::enable_if_t<HasBucketId<Request>::value, void> recordBucketId(
      const Request&) const {
    if (mcrouter::fiber_local<MemcacheRouterInfo>::getBucketId().has_value()) {
      h_->sawBucketIds.push_back(
          mcrouter::fiber_local<MemcacheRouterInfo>::getBucketId().value());
    }
  }

  template <class Request>
  std::enable_if_t<!HasBucketId<Request>::value, void> recordBucketId(
      const Request&) const {}

  template <class Request>
  void recordDistributionTargetRegion(const Request&) const {
    if (mcrouter::fiber_local<MemcacheRouterInfo>::getDistributionTargetRegion()
            .has_value()) {
      h_->distributionRegionInFiber.push_back(
          mcrouter::fiber_local<
              MemcacheRouterInfo>::getDistributionTargetRegion()
              .value());
    }
  }

  template <class Request>
  bool traverse(const Request& req, const RouteHandleTraverser<RouteHandleIf>&)
      const {
    recordBucketId(req);
    return false;
  }

  template <typename T, typename = void>
  struct has_app_error : std::false_type {};
  template <typename T>
  struct has_app_error<
      T,
      folly::void_t<decltype(std::declval<T>().appSpecificErrorCode_ref())>>
      : std::true_type {};

  template <
      typename Reply,
      typename std::enable_if_t<!has_app_error<Reply>::value>* = nullptr>
  void setAppspecificErrorCode(Reply& /* unused */) {}

  template <
      typename Reply,
      typename std::enable_if_t<has_app_error<Reply>::value>* = nullptr>
  void setAppspecificErrorCode(Reply& reply) {
    reply.appSpecificErrorCode_ref() = dataGet_.appSpecificErrorCode_;
  }

  template <class Request>
  ReplyT<Request> routeInternal(const Request& req) {
    ReplyT<Request> reply;

    if (h_->isTko) {
      return createReply<Request>(TkoReply);
    }

    if (h_->isPaused) {
      h_->wait();
    }

    if (h_->logCapturer) {
      h_->sawLogs = (*h_->logCapturer)();
    }

    h_->saw_keys.push_back(req.key_ref()->fullKey().str());
    h_->sawOperations.push_back(Request::name);
    h_->sawExptimes.push_back(getExptimeIfExist(req));
    h_->sawFlags.push_back(getFlagsIfExist(req));
    h_->sawQueryTags.push_back(getQueryTagsIfExists(req));
    recordBucketId(req);
    recordDistributionTargetRegion(req);
    if (carbon::GetLike<Request>::value) {
      reply.result_ref() = h_->resultGenerator_.hasValue()
          ? (*h_->resultGenerator_)(req.key_ref()->fullKey().str())
          : dataGet_.result_;
      detail::setReplyValue(reply, dataGet_.value_);
      detail::testSetFlags(reply, dataGet_.flags_);
      setAppspecificErrorCode(reply);
      return reply;
    }
    if (carbon::UpdateLike<Request>::value) {
      if (carbon::valuePtrUnsafe(req) != nullptr) {
        auto val = carbon::valuePtrUnsafe(req)->clone();
        folly::StringPiece sp_value = coalesceAndGetRange(val);
        h_->sawValues.push_back(sp_value.str());
      }
      reply.result_ref() = h_->resultGenerator_.hasValue()
          ? (*h_->resultGenerator_)(req.key_ref()->fullKey().str())
          : dataUpdate_.result_;
      detail::testSetFlags(reply, dataUpdate_.flags_);
      return reply;
    }
    if (carbon::DeleteLike<Request>::value) {
      reply.result_ref() = h_->resultGenerator_.hasValue()
          ? (*h_->resultGenerator_)(req.key_ref()->fullKey().str())
          : dataDelete_.result_;
      return reply;
    }
    return createReply(DefaultReply, req);
  }
};

template <class RouteHandleIf>
inline std::vector<std::shared_ptr<RouteHandleIf>> get_route_handles(
    const std::vector<std::shared_ptr<TestHandleImpl<RouteHandleIf>>>& hs) {
  std::vector<std::shared_ptr<RouteHandleIf>> r;
  for (auto& h : hs) {
    r.push_back(h->rh);
  }

  return r;
}

template <class RouterInfo = void, class = void>
struct GetLocalType {
  using type = typename folly::fibers::LocalType<void>;
};

template <class RouterInfo>
struct GetLocalType<
    RouterInfo,
    std::void_t<typename RouterInfo::RouteHandleIf>> {
  using type = typename mcrouter::fiber_local<RouterInfo>::ContextTypeTag;
};

template <class RouterInfo>
class TestFiberManager {
 public:
  TestFiberManager(size_t recordFiberStackEvery = kRecordFiberStackEveryDefault)
      : fm_(typename GetLocalType<RouterInfo>::type(),
            std::make_unique<folly::fibers::SimpleLoopController>(),
            getFiberOptions(recordFiberStackEvery)) {}

  void run(std::function<void()>&& fun) {
    runAll({std::move(fun)});
  }

  void runAll(std::vector<std::function<void()>>&& fs) {
    auto& fm = fm_;
    auto& loopController = dynamic_cast<folly::fibers::SimpleLoopController&>(
        fm_.loopController());
    fm.addTask([&fs, &loopController]() {
      folly::fibers::collectAll(fs.begin(), fs.end());
      loopController.stop();
    });

    loopController.loop([]() {});

    // Fiber stack high watermark stat is only available for build without ASAN
    // since Folly always returns 0 when FOLLY_SANITIZE_ADDRESS is set.
    auto stackHighWatermark = fm.stackHighWatermark();
    if (stackHighWatermark > 0) {
      VLOG(2) << "fiber stack high watermark: " << stackHighWatermark;
    }
  }

  folly::fibers::FiberManager& getFiberManager() {
    return fm_;
  }

 private:
  folly::fibers::FiberManager fm_;
  static constexpr size_t kRecordFiberStackEveryDefault = 0;

  static folly::fibers::FiberManager::Options getFiberOptions(
      size_t recordFiberStackEvery = 0) {
    folly::fibers::FiberManager::Options ret;
    ret.recordStackEvery = recordFiberStackEvery;
    return ret;
  }
};

inline std::string toString(const folly::IOBuf& buf) {
  auto b = buf.clone();
  return coalesceAndGetRange(b).str();
}

template <class Rh>
std::string replyFor(Rh& rh, const std::string& key) {
  auto reply = rh.route(McGetRequest(key));
  return carbon::valueRangeSlow(reply).str();
}

} // namespace memcache
} // namespace facebook
