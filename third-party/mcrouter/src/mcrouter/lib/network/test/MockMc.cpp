/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MockMc.h"

#include <folly/Conv.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/IOBufUtil.h"

namespace facebook {
namespace memcache {

void MockMc::CacheItem::updateLeaseToken() {
  static uint64_t leaseCounter = 100;
  leaseToken = leaseCounter++;
}

void MockMc::CacheItem::updateCasToken() {
  static uint64_t casCounter = 100;
  casToken = casCounter++;
}

MockMc::Item::Item(std::unique_ptr<folly::IOBuf> v) : value(std::move(v)) {}

MockMc::Item::Item(const folly::IOBuf& v, int32_t t, uint64_t f)
    : value(v.clone()),
      creationTime(time(nullptr)),
      exptime(t != 0 && t <= 60 * 60 * 24 * 30 ? t + time(nullptr) : t),
      flags(f) {}

const MockMc::Item* MockMc::get(folly::StringPiece key) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return nullptr;
  }
  return &it->second.item;
}

const MockMc::Item* MockMc::gat(int32_t newExptime, folly::StringPiece key) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return nullptr;
  }
  it->second.item.exptime = newExptime != 0 && newExptime <= 60 * 60 * 24 * 30
      ? newExptime + time(nullptr)
      : newExptime;
  return &it->second.item;
}

void MockMc::set(folly::StringPiece key, Item item) {
  citems_.erase(key.str());
  citems_.insert(std::make_pair(key.str(), CacheItem(std::move(item))));
}

bool MockMc::add(folly::StringPiece key, Item item) {
  if (get(key)) {
    return false;
  }
  set(key, std::move(item));
  return true;
}

bool MockMc::replace(folly::StringPiece key, Item item) {
  if (!get(key)) {
    return false;
  }
  set(key, std::move(item));
  return true;
}

bool MockMc::append(folly::StringPiece key, Item suffix) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return false;
  }

  auto& item = it->second.item;
  item.value->appendChain(std::move(suffix.value));
  return true;
}

bool MockMc::prepend(folly::StringPiece key, Item prefix) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return false;
  }

  auto& item = it->second.item;
  prefix.value->appendChain(std::move(item.value));
  item.value = std::move(prefix.value);
  return true;
}

std::pair<bool, int64_t> MockMc::arith(folly::StringPiece key, int64_t delta) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return std::make_pair(false, 0);
  }

  auto oldval = folly::to<uint64_t>(coalesceAndGetRange(it->second.item.value));
  auto newval = folly::to<std::string>(oldval + delta);
  it->second.updateCasToken();
  it->second.item.value = folly::IOBuf::copyBuffer(newval);
  return std::make_pair(true, oldval + delta);
}

bool MockMc::del(folly::StringPiece key) {
  auto it = findUnexpired(key);
  if (it != citems_.end()) {
    bool deleted = false;
    /* Delete moves items from CACHE to TLRU, and always bumps lease tokens */
    if (it->second.state == CacheItem::CACHE) {
      deleted = true;
      it->second.state = CacheItem::TLRU;
    }
    it->second.updateLeaseToken();
    it->second.updateCasToken();
    return deleted;
  }
  return false;
}

bool MockMc::touch(folly::StringPiece key, int32_t newExptime) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return false;
  }
  it->second.item.exptime = newExptime != 0 && newExptime <= 60 * 60 * 24 * 30
      ? newExptime + time(nullptr)
      : newExptime;
  return true;
}

/**
 * @return:  (item, 0)            On a hit.
 *           (stale_item, token)  On a miss.
 *           (stale_item, 1)      On a hot miss (another lease outstanding).
 */
std::pair<const MockMc::Item*, uint64_t> MockMc::leaseGet(
    folly::StringPiece key) {
  auto it = findUnexpired(key);
  if (it == citems_.end()) {
    /* Lease get on a non-existing item: create a new empty item and
       put it in TLRU with valid token */
    it = citems_
             .insert(std::make_pair(
                 key.str(), CacheItem(Item(folly::IOBuf::copyBuffer("")))))
             .first;
    it->second.state = CacheItem::TLRU;
    it->second.updateLeaseToken();
    it->second.updateCasToken();
  }

  auto& citem = it->second;
  switch (citem.state) {
    case CacheItem::CACHE:
      /* Regular hit */
      return std::make_pair(&citem.item, 0);
    case CacheItem::TLRU:
      /* First lease-get for a TLRU item, return with a valid token */
      citem.state = CacheItem::TLRU_HOT;
      return std::make_pair(&citem.item, citem.leaseToken);
    case CacheItem::TLRU_HOT:
      /* TLRU item with other lease-gets pending, return a hot miss token
         (special value 1). Note: in real memcached this state would
         revert to TLRU after a timeout. */
      return std::make_pair(&citem.item, 1);
  }

  CHECK(false);
}

MockMc::LeaseSetResult
MockMc::leaseSet(folly::StringPiece key, Item item, uint64_t token) {
  auto it = findUnexpired(key);
  if (it == citems_.end()) {
    /* Item doesn't exist in cache or TLRU */
    return LeaseSetResult::NOT_STORED;
  }

  auto& citem = it->second;
  if (citem.state == CacheItem::CACHE || citem.leaseToken == token) {
    /* Either the item is a hit, or the token is valid. Regular set */
    set(key, std::move(item));
    return LeaseSetResult::STORED;
  } else {
    /* The token is not valid (expired or wrong), but the value is in TLRU.
       Update the value but don't promote to cache. */
    citem.item = std::move(item);
    return LeaseSetResult::STALE_STORED;
  }
}

std::pair<const MockMc::Item*, uint64_t> MockMc::gets(folly::StringPiece key) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return std::make_pair<Item*, uint64_t>(nullptr, 0);
  }
  return std::make_pair(&it->second.item, it->second.casToken);
}

std::pair<const MockMc::Item*, uint64_t> MockMc::gats(
    int32_t newExptime,
    folly::StringPiece key) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return std::make_pair<Item*, uint64_t>(nullptr, 0);
  }
  it->second.item.exptime = newExptime != 0 && newExptime <= 60 * 60 * 24 * 30
      ? newExptime + time(nullptr)
      : newExptime;
  return std::make_pair(&it->second.item, it->second.casToken);
}

MockMc::CasResult
MockMc::cas(folly::StringPiece key, Item item, uint64_t token) {
  auto it = findUnexpired(key);
  if (it == citems_.end() || it->second.state != CacheItem::CACHE) {
    return CasResult::NOT_FOUND;
  }
  if (it->second.casToken != token) {
    return CasResult::EXISTS;
  }
  set(key, std::move(item));
  return CasResult::STORED;
}

std::unordered_map<std::string, MockMc::CacheItem>::iterator
MockMc::findUnexpired(folly::StringPiece key) {
  auto it = citems_.find(key.str());
  if (it == citems_.end()) {
    return it;
  }

  if (it->second.item.exptime > 0 && it->second.item.exptime <= time(nullptr)) {
    citems_.erase(it);
    return citems_.end();
  }

  return it;
}

void MockMc::flushAll() {
  citems_.clear();
}
} // namespace memcache
} // namespace facebook
