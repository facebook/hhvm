/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

#include <folly/Conv.h>
#include <folly/Range.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/HashUtil.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

template <class HashFunc>
class HashSelectorBase {
 public:
  HashSelectorBase(std::string salt, HashFunc hashFunc)
      : salt_(std::move(salt)), hashFunc_(std::move(hashFunc)) {}

  std::string type() const {
    return folly::to<std::string>(
        "hash|", HashFunc::type(), (salt_.empty() ? "" : "|salt=" + salt_));
  }

 protected:
  template <class Key>
  size_t selectInternal(Key&& key, size_t size) const {
    size_t n = 0;
    if (salt_.empty()) {
      n = hashFunc_(key);
    } else {
      n = hashWithSalt(key, salt_, [this](const folly::StringPiece sp) {
        return hashFunc_(sp);
      });
    }
    if (FOLLY_UNLIKELY(n >= size)) {
      throw std::runtime_error("index out of range");
    }
    return n;
  }

  bool hasSalt() const {
    return (!salt_.empty());
  }

 private:
  const std::string salt_;
  const HashFunc hashFunc_;
};

template <class HashFunc>
class HashSelector : public HashSelectorBase<HashFunc> {
 public:
  HashSelector(std::string salt, HashFunc hashFunc)
      : HashSelectorBase<HashFunc>(std::move(salt), std::move(hashFunc)) {}

  template <class Request>
  size_t select(const Request& req, size_t size) const {
    if (!this->hasSalt() &&
        req.key_ref()->reuseLastHash(size, HashFunc::typeId())) {
      return req.key_ref()->getLastHash();
    }
    // Hash functions can be stack-intensive, so jump back to the main context
    auto hash = folly::fibers::runInMainContext([this, &req, size]() {
      // this-> here is necessary for gcc-4.7 - it can't find selectInternal()
      // without it
      return this->selectInternal(req.key_ref()->routingKey(), size);
    });
    if (!this->hasSalt()) {
      req.key_ref()->setLastHash(hash, size, HashFunc::typeId());
    }
    return hash;
  }
};

template <class HashFunc, class RouterInfo>
class BucketHashSelector : public HashSelectorBase<HashFunc> {
 public:
  BucketHashSelector(std::string salt, HashFunc hashFunc, bool clientFanout)
      : HashSelectorBase<HashFunc>(std::move(salt), std::move(hashFunc)),
        clientFanout_(clientFanout) {}

  template <class Request>
  size_t select(const Request& req, size_t size) const {
    auto bucketId = mcrouter::fiber_local<RouterInfo>::getBucketId();
    checkRuntime(
        bucketId.has_value(),
        "The context doesn't contain bucket id. You must use McBucketRoute in front of bucketized PoolRoute");
    // Hash functions can be stack-intensive, so jump back to the main context
    auto key = folly::to<std::string>(*bucketId);
    if (this->clientFanout_) {
      if (auto sourceIpAddr = req.getSourceIpAddr()) {
        key += sourceIpAddr->str();
      }
    }
    return folly::fibers::runInMainContext(
        [this, size, key]() { return this->selectInternal(key, size); });
  }

 private:
  bool clientFanout_{false};
};

} // namespace memcache
} // namespace facebook
