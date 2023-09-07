/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <folly/Random.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/Observable.h"
#include "mcrouter/lib/network/MessageHelpers.h"

// forward declarations
namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

// forward declarations
class CarbonRouterInstanceBase;
class RuntimeVarsData;

using ObservableRuntimeVars =
    Observable<std::shared_ptr<const RuntimeVarsData>>;

class ShadowSettings {
 public:
  /**
   * @return  nullptr if config is invalid, new ShadowSettings struct otherwise
   */
  static std::shared_ptr<ShadowSettings> create(
      const folly::dynamic& json,
      CarbonRouterInstanceBase& router,
      size_t totalBuckets = 0);

  ~ShadowSettings();

  const std::string& keyFractionRangeRv() const {
    return keyFractionRangeRv_;
  }

  size_t startIndex() const {
    return startIndex_;
  }

  size_t endIndex() const {
    return endIndex_;
  }

  bool validateRepliesFlag() const {
    return validateReplies_;
  }

  // [start, end] where 0 <= start <= end <= numeric_limits<uint32_t>::max()
  std::pair<uint32_t, uint32_t> keyRange() const {
    auto fraction = keyRange_.load();
    return {fraction >> 32, fraction & ((1UL << 32) - 1)};
  }

  struct BucketRange {
    uint64_t start;
    uint64_t end;
  };
  BucketRange bucketRange() const {
    return bucketRange_.load();
  }

  void setTotalBuckets(size_t totalBuckets) {
    totalBuckets_ = totalBuckets;
  }

  /**
   * @throws std::logic_error if !(0 <= start <= end <= 1)
   */
  void setKeyRange(double start, double end);

  /**
   * Specify a list of keys to be shadowed.
   * Cannot be mixed with index range/key fraction range-based shadowing.
   */
  void setKeysToShadow(const std::vector<std::string>& keys);

  const std::vector<std::tuple<uint32_t, std::string>>& keysToShadow() const {
    return keysToShadow_;
  }

  void setRequestsFraction(double fraction) {
    requestsFraction_ = fraction;
  }

  /**
   * Tells whether or not a given request should be shadowed.
   *
   * NOTE: This does *not* take into account "index_ranges", as that information
   * is used to decide whether or not we should build ShadowRoute in the first
   * place.
   *
   * @param req   The original request.
   * @param rng   Random number generator.
   */
  template <class Request, class RNG>
  bool shouldShadow(
      const Request& req,
      const std::optional<uint64_t>& bucketId,
      RNG&& rng) const {
    return shouldShadowKey(req, bucketId) &&
        shouldShadowRandom(std::forward<RNG>(rng));
  }

  template <class Request>
  std::enable_if_t<HasKeyTrait<Request>::value, bool> shouldShadowKey(
      const Request& req,
      const std::optional<uint64_t>& bucketId) const {
    // If configured to use an explicit list of keys to be shadowed, check for
    // req.key_ref() in that list. Otherwise, decide to shadow based on
    // keyRange().
    if (!keysToShadow_.empty()) {
      const auto hashAndKeyToFind = std::make_tuple(
          req.key_ref()->routingKeyHash(), req.key_ref()->routingKey());
      return std::binary_search(
          keysToShadow_.begin(),
          keysToShadow_.end(),
          hashAndKeyToFind,
          std::less<std::tuple<uint32_t, folly::StringPiece>>());
    }

    if (bucketId) {
      auto bucketRange = bucketRange_.load();
      return bucketRange.start <= *bucketId && *bucketId <= bucketRange.end;
    }
    auto range = keyRange();
    return range.first <= req.key_ref()->routingKeyHash() &&
        req.key_ref()->routingKeyHash() <= range.second;
  }
  template <class Request>
  std::enable_if_t<!HasKeyTrait<Request>::value, bool> shouldShadowKey(
      const Request& /* req */,
      const std::optional<uint64_t>& /* bucketId */) const {
    return true;
  }

 private:
  ObservableRuntimeVars::CallbackHandle handle_;
  void registerOnUpdateCallback(CarbonRouterInstanceBase& router);

  std::string keyFractionRangeRv_;
  size_t startIndex_{0};
  size_t endIndex_{0};
  double requestsFraction_{1.0};

  // Ideally, this would just be an unordered set<Key<string>>, but we need to
  // allow for comparing to Key<IOBuf>. We can work with a vector<Key<string>>
  // sorted by routingKeyHash.
  std::vector<std::tuple<uint32_t, std::string>> keysToShadow_;

  std::atomic<uint64_t> keyRange_{0};

  std::atomic<BucketRange> bucketRange_{{.start = 1, .end = 0}};
  size_t totalBuckets_{0};

  bool validateReplies_{false};

  template <class RNG>
  bool shouldShadowRandom(RNG&& rng) const {
    const double kEpsilon = 0.00001;
    const double kAlwaysSend = 1.0 - kEpsilon;

    if (requestsFraction_ >= kAlwaysSend) {
      return true;
    }
    return folly::Random::randDouble01(rng) < requestsFraction_;
  }

  ShadowSettings() = default;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
