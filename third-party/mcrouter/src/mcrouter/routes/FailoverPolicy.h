/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/Ch3HashFunc.h"
#include "mcrouter/lib/HashSelector.h"
#include "mcrouter/lib/RendezvousHashFunc.h"
#include "mcrouter/lib/RendezvousHashHelper.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"

#include "mcrouter/routes/RendezvousRouteHelpers.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

struct Stats {
  uint32_t num_collisions;
  uint32_t num_cached_failure_domain_hits;
  uint32_t num_failed_domain_collisions;
};

struct FailoverPolicyContext {
  size_t numTries_{0};
};

class CarbonErrorResults {
 public:
  CarbonErrorResults(const folly::dynamic& json) {
    checkLogic(json.isArray(), "List of carbon errors is not an array");
    for (const auto& elem : json) {
      checkLogic(elem.isString(), "Failover error {} is not a string", elem);
      bool foundMatch = false;
      for (size_t i = 0; i < static_cast<size_t>(carbon::Result::NUM_RESULTS);
           ++i) {
        carbon::Result errorType = static_cast<carbon::Result>(i);
        folly::StringPiece errorName(carbon::resultToString(errorType));
        errorName.removePrefix("mc_res_");
        if (isErrorResult(errorType) && elem.getString() == errorName) {
          errorResults_[i] = true;
          foundMatch = true;
          break;
        }
      }
      checkLogic(
          foundMatch,
          "Failover error {} is not a valid error result",
          elem.getString());
    }
  }

  bool contains(const carbon::Result result) const {
    return errorResults_[static_cast<size_t>(result)];
  }

 private:
  std::array<bool, static_cast<size_t>(carbon::Result::NUM_RESULTS)>
      errorResults_{};
};

template <typename RouteHandleIf>
class FailoverInOrderPolicy {
 public:
  static constexpr bool optimizeNoFailoverRouteCase = true;
  using RouteHandlePtr = std::shared_ptr<RouteHandleIf>;

  FailoverInOrderPolicy(
      const std::vector<RouteHandlePtr>& children,
      const folly::dynamic& json)
      : children_(children) {
    if (!json.isObject()) {
      return;
    }
    auto jMaxTries = json.get_ptr("max_tries");
    if (jMaxTries) {
      maxTries_ = static_cast<uint32_t>(
          parseInt(*jMaxTries, "max_tries", 1, UINT32_MAX));
      if (maxTries_ > children_.size()) {
        LOG_FAILURE(
            "mcrouter",
            failure::Category::kInvalidConfig,
            "MaxTries ({}) exceeds number of children ({}), "
            "setting it to number of children",
            maxTries_,
            children_.size());
        maxTries_ = children_.size();
      }
    }
    auto jExcludeErrors = json.get_ptr("exclude_errors");
    if (jExcludeErrors) {
      excludeErrors_ = std::make_unique<CarbonErrorResults>(*jExcludeErrors);
    }
  }

  class ChildProxy {
   public:
    ChildProxy(RouteHandlePtr child) : child_(child) {}

    template <class Request>
    ReplyT<Request> route(const Request& req) {
      return child_->route(req);
    }

   private:
    const RouteHandlePtr child_;
  };

  template <class Request>
  class Iter : public boost::iterator_facade<
                   Iter<Request>,
                   ChildProxy,
                   std::forward_iterator_tag,
                   ChildProxy> {
   public:
    Iter(const std::vector<RouteHandlePtr>& children, size_t id)
        : children_(children), id_(id) {
      assert(children_.size() > 1);
    }

    void setFailedDomain(uint32_t) {}

    size_t getTrueIndex() const {
      return id_;
    }

    void setPassive() {}

    Stats getStats() const {
      return {0, 0, 0};
    }

   private:
    void increment() {
      ++id_;
    }

    bool equal(const Iter<Request>& other) const {
      return id_ == other.id_;
    }

    ChildProxy dereference() const {
      return ChildProxy(children_[id_]);
    }

    friend class boost::iterator_core_access;

    const std::vector<RouteHandlePtr>& children_;
    size_t id_;
  };
  template <class Request>
  using Iterator = Iter<Request>;
  template <class Request>
  using ConstIterator = Iter<Request const>;

  template <class Request>
  ConstIterator<Request> cbegin(Request&, const FailoverPolicyContext&) const {
    return ConstIterator<Request>(children_, 0);
  }

  template <class Request>
  ConstIterator<Request> cend(Request&, const FailoverPolicyContext&) const {
    return ConstIterator<Request>(children_, children_.size());
  }

  uint32_t maxErrorTries() const {
    return maxTries_;
  }

  bool excludeError(const carbon::Result result) const {
    return excludeErrors_ && excludeErrors_->contains(result);
  }

  template <class Request>
  FailoverPolicyContext context(const Request&) const {
    return FailoverPolicyContext();
  }

  template <class Request>
  Iterator<Request> begin(Request&, const FailoverPolicyContext&) {
    return Iterator<Request>(children_, 0);
  }

  template <class Request>
  Iterator<Request> end(Request&, const FailoverPolicyContext&) const {
    return Iterator<Request>(children_, children_.size());
  }

  // Returns the stat to increment when failover occurs.
  stat_name_t getFailoverStat() const {
    return failover_inorder_policy_stat;
  }

  // Returns the stat when all failover destinations are exhausted.
  stat_name_t getFailoverFailedStat() const {
    return failover_inorder_policy_failed_stat;
  }

  bool getFailureDomainsEnabled() const {
    return false;
  }

 private:
  const std::vector<RouteHandlePtr>& children_;
  uint32_t maxTries_{std::numeric_limits<uint32_t>::max()};
  std::unique_ptr<CarbonErrorResults> excludeErrors_;
};

template <typename RouteHandleIf, typename RouterInfo>
class FailoverDeterministicOrderPolicy {
 public:
  static constexpr bool optimizeNoFailoverRouteCase = true;
  using RouteHandlePtr = std::shared_ptr<RouteHandleIf>;

  FailoverDeterministicOrderPolicy(
      const std::vector<std::shared_ptr<RouteHandleIf>>& children,
      const folly::dynamic& json)
      : children_(children) {
    checkLogic(
        json.isObject(),
        "Failover: DeterministicOrderPolicy config is not an object");

    auto jMaxTries = json.get_ptr("max_tries");
    checkLogic(
        jMaxTries != nullptr,
        "Failover: DeterministicOrderPolicy must specify 'max_tries' field");
    maxTries_ =
        static_cast<uint32_t>(parseInt(*jMaxTries, "max_tries", 1, UINT32_MAX));
    if (maxTries_ > children_.size()) {
      LOG_FAILURE(
          "mcrouter",
          failure::Category::kInvalidConfig,
          "MaxTries ({}) exceeds number of children ({}), "
          "setting it to number of children",
          maxTries_,
          children_.size());
      maxTries_ = children_.size();
    }

    auto jMaxErrorTries = json.get_ptr("max_error_tries");
    checkLogic(
        jMaxErrorTries != nullptr,
        "Failover: DeterministicOrderPolicy must specify"
        " 'max_error_tries' field");
    maxErrorTries_ = static_cast<uint32_t>(
        parseInt(*jMaxErrorTries, "max_error_tries", 1, UINT32_MAX));
    if (maxErrorTries_ > maxTries_) {
      LOG_FAILURE(
          "mcrouter",
          failure::Category::kInvalidConfig,
          "MaxErrorTries ({}) exceeds MaxTries ({}), "
          "setting it to number of children",
          maxErrorTries_,
          maxTries_);
      maxErrorTries_ = maxTries_;
    }

    auto jExcludeErrors = json.get_ptr("exclude_errors");
    if (jExcludeErrors) {
      excludeErrors_ = std::make_unique<CarbonErrorResults>(*jExcludeErrors);
    }

    if (auto jEnableFailureDomains = json.get_ptr("enable_failure_domains")) {
      enableFailureDomains_ = jEnableFailureDomains->getBool();
    }

    if (auto jIgnore_normal_reply_index =
            json.get_ptr("ignore_normal_reply_index")) {
      ignore_normal_reply_index_ = jIgnore_normal_reply_index->getBool();
    }

    funcType_ = Ch3HashFunc::type();
    if (auto jHash = json.get_ptr("hash")) {
      if (auto jhashFunc = jHash->get_ptr("hash_func")) {
        checkLogic(
            jhashFunc->isString(),
            "Failover: DeterministicOrderPolicy: hash_func is not a string");
        funcType_ = jhashFunc->getString();
      }
      config_ = *jHash;
    } else {
      config_ = json;
    }
    checkLogic(
        (funcType_ == Ch3HashFunc::type() ||
         funcType_ == WeightedCh3HashFunc::type()),
        "Unknown hash function {}",
        funcType_);
  }

  class ChildProxy {
   public:
    ChildProxy(RouteHandlePtr child) : child_(child) {}

    template <class Request>
    ReplyT<Request> route(const Request& req) {
      return child_->route(req);
    }

   private:
    RouteHandlePtr child_;
  };

  template <class Request, class Policy, class Config, class StringLoc>
  class Iter : public boost::iterator_facade<
                   Iter<Request, Policy, Config, StringLoc>,
                   ChildProxy,
                   std::forward_iterator_tag,
                   ChildProxy> {
   public:
    Iter(
        Policy& failoverPolicy,
        const Config& config,
        const StringLoc& funcType,
        uint32_t salt,
        uint32_t id,
        Request& req)
        : policy_(failoverPolicy),
          funcType_(funcType),
          salt_(salt),
          config_(config),
          id_(id),
          req_(req) {
      // Do default initialization here. Failover related initialization/sizing
      // is done later, when failover actually occurs. So, usedIndexes_ is
      // not initialized here.
      index_ = 0;
    }

    // By setting this, most of the increment() logic can be skipped for
    // passive iterators and save some valuable CPU cycles
    void setPassive() {
      active_ = false;
    }

    size_t getTrueIndex() const {
      return index_;
    }

    Stats getStats() const {
      return {
          collisions_,
          num_cached_failured_domain_hits_,
          num_failed_domain_collisions_};
    }

    uint32_t getFailureDomain(size_t index) {
      uint32_t failureDomain = policy_.getMemoizedFailureDomain(index);
      if (failureDomain != 0) {
        num_cached_failured_domain_hits_++;
        return failureDomain;
      }
      // This may traverse the failover destinations if a failover route is
      // under a failover route.
      RouteHandleTraverser<typename RouterInfo::RouteHandleIf> t(
          /* start */ nullptr,
          /* end */ nullptr,
          [&failureDomain](const AccessPoint& ap, const PoolContext&) mutable {
            failureDomain = ap.getFailureDomain();
            return true;
          });
      policy_.children_[index]->traverse(req_, t);

      // Do memoization only in case of non-const iterators
      if constexpr (!std::is_const<Policy>{}) {
        if (failureDomain > 0) {
          policy_.memoizeFailureDomain(index, failureDomain);
        }
      }
      return failureDomain;
    }

    void setFailedDomain(uint32_t failedDomain) {
      // failedDomain 0 means there is no failure Domain associated with the
      // destination
      if (policy_.enableFailureDomains_ && failedDomain > 0) {
        failedDomains_.insert(failedDomain);
        if (index_ > 0) {
          policy_.memoizeFailureDomain(index_, failedDomain);
        }
      }
    }

   private:
    void increment() {
      // No need to determine "next" index for passive iterators
      if (!active_) {
        ++id_;
        return;
      }
      uint32_t numAttempts = 0;
      const auto nChildren = policy_.children_.size();
      // arbitrarily set high number of max attempts so that finding the next
      // index does not get into infinite loop
      constexpr uint32_t maxAttempts = 100;
      // arbitrarily set percent of max attempts as failure domain threshold
      // so that we do not consider all the MSB as failed (Remember that we
      // do not have direct signal that a TKO is due to MSB failure, so if
      // the number failure domains are small and all the failure domains are
      // in the set, then we would be skipping all of them. This threshold is
      // to get out of skipping all the destinations due to this pathological
      // case
      constexpr uint32_t failureDomainThreshold = (3 * maxAttempts) / 4;
      if (index_ == 0) {
        // initialize usedIndexes_ here before it is used
        usedIndexes_.resize(nChildren);
        usedIndexes_.set(0);

        // Use normal_reply_index only if ignore flag is false
        if (!policy_.ignore_normal_reply_index_) {
          size_t normal_reply_index =
              mcrouter::fiber_local<RouterInfo>::getSelectedIndex();
          // Skip the destination selected by normal route by adding the
          // index of the normal route destination to usedIndexes.
          if ((normal_reply_index + 1) < usedIndexes_.size()) {
            usedIndexes_.set(normal_reply_index + 1);
          } else {
            LOG_FAILURE(
                "mcrouter",
                failure::Category::kInvalidConfig,
                "Normal Route and Failover route pool sizes seem to be different."
                " ignore_normal_reply_index config flag should be used.");
          }
        }
      }
      bool failedDomain = false;
      do {
        salt_++;
        if (funcType_ == Ch3HashFunc::type()) {
          Ch3HashFunc ch3Func(nChildren);
          index_ = HashSelector<Ch3HashFunc>(std::to_string(salt_), ch3Func)
                       .select(req_, nChildren);
        } else {
          WeightedCh3HashFunc wCh3Func{config_, nChildren};
          index_ =
              HashSelector<WeightedCh3HashFunc>(std::to_string(salt_), wCh3Func)
                  .select(req_, nChildren);
        }

        // Use failure domains only in case of non-const iterators
        if constexpr (!std::is_const<Policy>{}) {
          uint32_t nextFd = getFailureDomain(index_);
          failedDomain =
              (policy_.enableFailureDomains_ &&
               (failedDomains_.find(nextFd) != failedDomains_.end()));
        }
        // force ignore failure-domain if we have tried more than
        // failureDomainThreshold -- FAIL safe mechanism
        if (numAttempts > failureDomainThreshold) {
          failedDomain = false;
        }

        if (failedDomain) {
          num_failed_domain_collisions_++;
        } else {
          collisions_++;
        }

      } while ((usedIndexes_.test(index_) || failedDomain) &&
               (numAttempts++ < maxAttempts));
      collisions_--;
      usedIndexes_.set(index_);
      ++id_;
    }

    bool equal(const Iter<Request, Policy, Config, StringLoc>& other) const {
      return id_ == other.id_;
    }

    ChildProxy dereference() const {
      return ChildProxy(policy_.children_[index_]);
    }

    friend class boost::iterator_core_access;
    Policy& policy_;
    const StringLoc& funcType_;
    uint32_t salt_{0};
    const Config& config_;
    uint32_t id_{0};
    const Request& req_;
    uint32_t collisions_{0};
    uint32_t num_cached_failured_domain_hits_{0};
    uint32_t num_failed_domain_collisions_{0};
    // usedIndexes_ is used to keep track of indexes that have already been
    // used and is useful in avoiding picking the same destinations again and
    // again
    boost::dynamic_bitset<> usedIndexes_;
    size_t index_;
    folly::F14FastSet<uint32_t> failedDomains_;
    bool active_{true};
  };
  template <class Request>
  using Iterator = Iter<
      Request,
      FailoverDeterministicOrderPolicy<RouteHandleIf, RouterInfo>,
      folly::dynamic,
      std::string>;
  template <class Request>
  using ConstIterator = Iter<
      Request const,
      FailoverDeterministicOrderPolicy<RouteHandleIf, RouterInfo> const,
      folly::dynamic const,
      std::string const>;

  template <class Request>
  Iterator<Request> begin(Request& req, const FailoverPolicyContext&) {
    return Iterator<Request>(*this, config_, funcType_, salt_, 0, req);
  }

  template <class Request>
  Iterator<Request> end(Request& req, const FailoverPolicyContext&) {
    return Iterator<Request>(*this, config_, funcType_, salt_, maxTries_, req);
  }

  template <class Request>
  ConstIterator<Request> cbegin(Request& req, const FailoverPolicyContext&)
      const {
    return ConstIterator<Request>(*this, config_, funcType_, salt_, 0, req);
  }

  template <class Request>
  ConstIterator<Request> cend(Request& req, const FailoverPolicyContext&)
      const {
    return ConstIterator<Request>(
        *this, config_, funcType_, salt_, maxTries_, req);
  }

  uint32_t maxErrorTries() const {
    return maxErrorTries_;
  }

  bool excludeError(const carbon::Result result) const {
    return excludeErrors_ && excludeErrors_->contains(result);
  }

  template <class Request>
  FailoverPolicyContext context(const Request&) const {
    return FailoverPolicyContext();
  }

  // Returns the stat to increment when failover occurs.
  stat_name_t getFailoverStat() const {
    return failover_deterministic_order_policy_stat;
  }

  // Returns the stat when all failover destinations are exhausted.
  stat_name_t getFailoverFailedStat() const {
    return failover_deterministic_order_policy_failed_stat;
  }

  uint32_t getMemoizedFailureDomain(uint32_t index) const {
    auto it = failureDomainMap_.find(index);
    if (it != failureDomainMap_.end()) {
      return it->second;
    }
    return 0;
  }

  void memoizeFailureDomain(uint32_t index, uint32_t failureDomain) {
    auto existingFD = getMemoizedFailureDomain(index);
    if (existingFD != 0) {
      assert(existingFD == failureDomain);
    } else {
      failureDomainMap_[index] = failureDomain;
    }
  }

  bool getFailureDomainsEnabled() const {
    return enableFailureDomains_;
  }

 private:
  const std::vector<RouteHandlePtr>& children_;
  uint32_t maxTries_;
  uint32_t maxErrorTries_;
  std::unique_ptr<CarbonErrorResults> excludeErrors_;
  folly::dynamic config_;
  std::string funcType_;
  uint32_t salt_{1};
  bool enableFailureDomains_{false};
  bool ignore_normal_reply_index_{false};
  // map of index in children_ to it's failure Domain
  folly::F14FastMap<uint32_t, uint32_t> failureDomainMap_;
};

template <typename RouteHandleIf, typename RouterInfo, typename HashFunc>
class FailoverRendezvousPolicy {
 public:
  static constexpr bool optimizeNoFailoverRouteCase = true;
  using RouteHandlePtr = std::shared_ptr<RouteHandleIf>;

  FailoverRendezvousPolicy(
      const std::vector<std::shared_ptr<RouteHandleIf>>& children,
      const folly::dynamic& json)
      : children_(children),
        hashFunc_(
            getTags(json, children.size() - 1, "FailoverRendezvousPolicy"),
            json) {
    checkLogic(
        json.isObject(),
        "Failover: FailoverRendezvousPolicy config is not an object");
  }

  class ChildProxy {
   public:
    ChildProxy(RouteHandlePtr child) : child_(child) {}

    template <class Request>
    ReplyT<Request> route(const Request& req) {
      return child_->route(req);
    }

   private:
    RouteHandlePtr child_;
  };

  class Iterator : public boost::iterator_facade<
                       Iterator,
                       ChildProxy,
                       std::forward_iterator_tag,
                       ChildProxy> {
   public:
    // A begin iterator.
    template <class Request>
    Iterator(const FailoverRendezvousPolicy& failoverPolicy, Request& req)
        : policy_(failoverPolicy),
          iter_(failoverPolicy.hashFunc_.begin(req.key_ref()->routingKey())) {}

    // An end, empty iterator.
    Iterator(const FailoverRendezvousPolicy& failoverPolicy)
        : policy_(failoverPolicy),
          iter_(failoverPolicy.hashFunc_.end()),
          primaryIndex_(0) {}

    size_t getTrueIndex() const {
      CHECK(!iter_.empty());
      // + 1 because element 0 isn't a host, but rather a route pool.
      return primaryIndex_ < 0 ? 0 : *iter_ + 1;
    }

    Stats getStats() const {
      return {0, 0, 0};
    }

    void setPassive() {}

    void setFailedDomain(uint32_t) {}

   private:
    void increment() {
      if (primaryIndex_ < 0) {
        primaryIndex_ = mcrouter::fiber_local<RouterInfo>::getSelectedIndex();
        if (primaryIndex_ < 0) {
          primaryIndex_ = 0;
        }
      } else {
        // Can't increment past end.
        CHECK(!iter_.empty());
        ++iter_;
      }

      if (!iter_.empty() && static_cast<int64_t>(*iter_) == primaryIndex_) {
        ++iter_;
      }

      CHECK(iter_.empty() || static_cast<int64_t>(*iter_) != primaryIndex_);
    }

    bool equal(const Iterator& other) const {
      if (primaryIndex_ < 0) {
        return other.primaryIndex_ < 0;
      } else {
        return iter_ == other.iter_;
      }
    }

    ChildProxy dereference() const {
      return ChildProxy(policy_.children_[getTrueIndex()]);
    }

   private:
    friend class boost::iterator_core_access;
    const FailoverRendezvousPolicy& policy_;
    RendezvousIterator iter_;
    // Child 0 is not one of the hosts, but rather a RoutePool.  We need to
    // return that first, before returning actual hosts.  Also, the index of the
    // primary isn't known when the Iterator is constructed; we need to wait for
    // the first call to increment() to query it.
    //
    // This index is 0-based, i.e. if the primaryIndex_ is 2, we need to never
    // return 3 from getTrueIndex().
    int32_t primaryIndex_{-1};
  };

  template <class Request>
  Iterator begin(Request& req, const FailoverPolicyContext&) {
    return Iterator(*this, req);
  }

  template <class Request>
  Iterator end(Request&, const FailoverPolicyContext&) {
    return Iterator(*this);
  }

  template <class Request>
  Iterator cbegin(Request& req, const FailoverPolicyContext&) const {
    return Iterator(*this, req);
  }

  template <class Request>
  Iterator cend(Request&, const FailoverPolicyContext&) const {
    return Iterator(*this);
  }

  uint32_t maxErrorTries() const {
    return std::numeric_limits<uint32_t>::max();
  }

  bool excludeError(const carbon::Result) const {
    return false; // No exclusions from retry counts
  }

  template <class Request>
  FailoverPolicyContext context(const Request&) const {
    return FailoverPolicyContext();
  }

  // Returns the stat to increment when failover occurs.
  stat_name_t getFailoverStat() const {
    return failover_rendezvous_policy_stat;
  }

  // Returns the stat when all failover destinations are exhausted.
  stat_name_t getFailoverFailedStat() const {
    return failover_rendezvous_policy_failed_stat;
  }

  bool getFailureDomainsEnabled() const {
    return false;
  }

 private:
  const std::vector<RouteHandlePtr>& children_;
  folly::dynamic config_;
  HashFunc hashFunc_;
};

template <typename RouteHandleIf>
class FailoverLeastFailuresPolicy {
 public:
  static constexpr bool optimizeNoFailoverRouteCase = true;
  using RouteHandlePtr = std::shared_ptr<RouteHandleIf>;

  FailoverLeastFailuresPolicy(
      const std::vector<std::shared_ptr<RouteHandleIf>>& children,
      const folly::dynamic& policyConfig)
      : children_(children), recentErrorCount_(children_.size(), 0) {
    auto jMaxTries = policyConfig.get_ptr("max_tries");
    checkLogic(
        jMaxTries != nullptr,
        "Failover: LeastFailuresPolicy must specify 'max_tries' field");
    maxTries_ =
        static_cast<uint32_t>(parseInt(*jMaxTries, "max_tries", 1, UINT32_MAX));
    if (maxTries_ > children_.size()) {
      LOG_FAILURE(
          "mcrouter",
          failure::Category::kInvalidConfig,
          "MaxTries ({}) exceeds number of children ({}), "
          "setting it to number of children",
          maxTries_,
          children_.size());
      maxTries_ = children_.size();
    }
  }

  class ChildProxy {
   public:
    ChildProxy(
        FailoverLeastFailuresPolicy<RouteHandleIf>& failoverPolicy,
        size_t index)
        : failoverPolicy_(failoverPolicy), index_(index) {}

    template <class Request>
    ReplyT<Request> route(const Request& req) {
      auto& child = failoverPolicy_.children_[index_];
      auto reply = child->route(req);
      if (isErrorResult(*reply.result_ref())) {
        failoverPolicy_.recentErrorCount_[index_]++;
      } else {
        failoverPolicy_.recentErrorCount_[index_] = 0;
      }
      return reply;
    }

   private:
    FailoverLeastFailuresPolicy<RouteHandleIf>& failoverPolicy_;
    size_t index_;
  };

  template <class Request, class Policy>
  class Iter : public boost::iterator_facade<
                   Iter<Request, Policy>,
                   ChildProxy,
                   std::forward_iterator_tag,
                   ChildProxy> {
   public:
    Iter(Policy& failoverPolicy, size_t id)
        : policy_(failoverPolicy), id_(id) {}

    size_t getTrueIndex() const {
      return order_[id_];
    }

    void setPassive() {}

    void setFailedDomain(uint32_t) {}

    Stats getStats() const {
      return {0, 0, 0};
    }

   private:
    void increment() {
      if (id_ == 0) {
        order_ = std::move(policy_.getLeastFailureRouteIndices());
      }
      ++id_;
    }

    bool equal(const Iter<Request, Policy>& other) const {
      return id_ == other.id_;
    }

    ChildProxy dereference() const {
      return ChildProxy(policy_, id_ == 0 ? id_ : order_[id_]);
    }

    friend class boost::iterator_core_access;

    Policy& policy_;
    std::vector<size_t> order_;
    size_t id_;
  };

  template <class Request>
  using Iterator = Iter<Request, FailoverLeastFailuresPolicy<RouteHandleIf>>;
  template <class Request>
  using ConstIterator =
      Iter<Request const, FailoverLeastFailuresPolicy<RouteHandleIf> const>;

  template <class Request>
  ConstIterator<Request> cbegin(Request&, const FailoverPolicyContext&) const {
    return ConstIterator<Request>(*this, 0);
  }

  template <class Request>
  ConstIterator<Request> cend(Request&, const FailoverPolicyContext&) const {
    return ConstIterator<Request>(*this, maxTries_);
  }

  uint32_t maxErrorTries() const {
    return std::numeric_limits<uint32_t>::max();
  }

  bool excludeError(const carbon::Result) const {
    return false; // No exclusions from retry counts
  }

  template <class Request>
  FailoverPolicyContext context(const Request&) const {
    return FailoverPolicyContext();
  }

  template <class Request>
  Iterator<Request> begin(Request&, const FailoverPolicyContext&) {
    return Iterator<Request>(*this, 0);
  }

  template <class Request>
  Iterator<Request> end(Request&, const FailoverPolicyContext&) {
    return Iterator<Request>(*this, maxTries_);
  }

  // Returns the stat to increment when failover occurs.
  stat_name_t getFailoverStat() const {
    return failover_least_failures_policy_stat;
  }

  // Returns the stat when all failover destinations are exhausted.
  stat_name_t getFailoverFailedStat() const {
    return failover_least_failures_policy_failed_stat;
  }

  bool getFailureDomainsEnabled() const {
    return false;
  }

 private:
  std::vector<size_t> getLeastFailureRouteIndices() const {
    std::vector<size_t> indices;
    size_t sz = recentErrorCount_.size();
    indices.reserve(sz);
    for (size_t i = 0; i < sz; i++) {
      indices.push_back(i);
    }
    // 0th index always goes first.
    std::stable_sort(
        indices.begin() + 1, indices.end(), [this](size_t a, size_t b) {
          return recentErrorCount_[a] < recentErrorCount_[b];
        });
    indices.resize(maxTries_);

    return indices;
  }

  const std::vector<RouteHandlePtr>& children_;
  size_t maxTries_;

  std::vector<size_t> recentErrorCount_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
