
/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/bespoke/key-order.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/bespoke/struct-dict.h"

#include <sstream>

namespace HPHP::bespoke {

TRACE_SET_MOD(bespoke);

namespace {

const StaticString s_extraKey("...");

struct KeyCollectionHash {
  size_t operator()(const KeyOrderData& ko) const {
    size_t seed = 0;
    for (auto const key : ko) {
      seed = folly::hash::hash_combine(seed, key->hash());
    }
    return seed;
  }
};

using KeyOrderSet =
  std::unordered_set<KeyOrderData, KeyCollectionHash>;

KeyOrderSet s_profilingKeyOrders;
KeyOrderSet s_persistentKeyOrders;
folly::SharedMutex s_keyOrderLock;
}

KeyOrder KeyOrder::insert(const StringData* k) const {
  if (!k->isStatic() || !m_keys) return KeyOrder::MakeInvalid();
  if (isTooLong() || contains(k)) return *this;
  KeyOrderData newOrder{*m_keys};
  auto const full = m_keys->size() == RO::EvalBespokeMaxTrackedKeys;
  newOrder.push_back(full ? s_extraKey.get() : k);
  return Make(newOrder);
}

KeyOrder KeyOrder::remove(const StringData* k) const {
  if (!valid()) return *this;
  KeyOrderData newOrder{*m_keys};
  newOrder.erase(std::remove_if(newOrder.begin(), newOrder.end(),
                                [&](LowStringPtr key) { return k->same(key); }),
                 newOrder.end());
  return Make(newOrder);
}

KeyOrder KeyOrder::pop() const {
  if (empty() || !valid()) return *this;
  KeyOrderData newOrder{*m_keys};
  newOrder.pop_back();
  return Make(newOrder);
}

KeyOrder KeyOrder::Make(const KeyOrderData& ko) {
  auto trimmedKeyOrder = trimKeyOrder(ko);
  {
    folly::SharedMutex::ReadHolder rlock{s_keyOrderLock};
    auto it = s_profilingKeyOrders.find(trimmedKeyOrder);
    if (it != s_profilingKeyOrders.end()) return KeyOrder{&*it};
  }

  std::unique_lock wlock{s_keyOrderLock};
  auto const ret = s_profilingKeyOrders.insert(std::move(trimmedKeyOrder));
  return KeyOrder{&*ret.first};
}

KeyOrder::KeyOrder(const KeyOrderData* keys)
  : m_keys(keys)
{}

KeyOrderData KeyOrder::trimKeyOrder(const KeyOrderData& ko) {
  KeyOrderData res{ko};
  if (res.size() > RO::EvalBespokeMaxTrackedKeys) {
    res.resize(RO::EvalBespokeMaxTrackedKeys);
    res.push_back(s_extraKey.get());
  }
  return res;
}

std::string KeyOrder::toString() const {
  if (!m_keys) return "<invalid>";
  std::stringstream ss;
  ss << '[';
  for (auto i = 0; i < m_keys->size(); ++i) {
    if (i > 0) ss << ',';
    ss << '"' << folly::cEscape<std::string>((*m_keys)[i]->data()) << '"';
  }
  ss << ']';
  return ss.str();
}

bool KeyOrder::operator==(const KeyOrder& other) const {
  return this->m_keys == other.m_keys;
}

KeyOrder KeyOrder::ForArray(const ArrayData* ad) {
  KeyOrderData ko;
  auto hasStaticStrKeysOnly = true;
  IterateKV(ad, [&](auto k, auto /*v*/) -> bool {
    if (tvIsString(k) && val(k).pstr->isStatic()) {
      ko.push_back(val(k).pstr);
      return false;
    } else {
      hasStaticStrKeysOnly = false;
      return true;
    }
  });
  return hasStaticStrKeysOnly ? Make(ko) : KeyOrder();
}

KeyOrder KeyOrder::MakeInvalid() {
  return KeyOrder{};
}

bool KeyOrder::isTooLong() const {
  assertx(m_keys);
  return m_keys->size() > RO::EvalBespokeMaxTrackedKeys;
}

size_t KeyOrder::size() const {
  assertx(valid());
  return m_keys->size();
}

bool KeyOrder::empty() const {
  return m_keys && m_keys->empty();
}

bool KeyOrder::valid() const {
  return m_keys && !isTooLong();
}

bool KeyOrder::contains(const StringData* val) const {
  assertx(valid());
  return std::find(m_keys->begin(), m_keys->end(), val) != m_keys->end();
}

KeyOrder::const_iterator KeyOrder::begin() const {
  assertx(valid());
  return m_keys->begin();
}

KeyOrder::const_iterator KeyOrder::end() const {
  assertx(valid());
  return m_keys->end();
}

void KeyOrder::ReleaseProfilingKeyOrders() {
  s_profilingKeyOrders.clear();
}

KeyOrder collectKeyOrder(const KeyOrderMap& keyOrderMap) {
  hphp_fast_set<const StringData*> keys;
  uint64_t weightedSizeSum = 0;
  uint64_t weight = 0;
  for (auto const& pair : keyOrderMap) {
    if (!pair.first.valid()) return pair.first;
    weightedSizeSum += pair.first.size() * pair.second;
    weight += pair.second;
    keys.insert(pair.first.begin(), pair.first.end());
  }

  if (weight == 0 || keys.size() > bespoke::StructLayout::maxNumKeys()) {
    return KeyOrder::MakeInvalid();
  }

  auto const weightedAvg = (double)weightedSizeSum / weight;
  auto const scale =
    (keys.size() + RO::EvalBespokeStructDictMinKeys) /
    (weightedAvg + RO::EvalBespokeStructDictMinKeys);
  // If the final merged key size is significantly larger than the average
  // key order size, creating a StructDict will waste memory.
  if (scale > RO::EvalBespokeStructDictMaxSizeRatio) {
    return KeyOrder::MakeInvalid();
  }

  KeyOrderData sorted;
  sorted.reserve(keys.size());
  for (auto const key : keys) {
    sorted.push_back(key);
  }
  std::sort(sorted.begin(), sorted.end(),
            [](auto a, auto b) { return a->compare(b) < 0; });
  return KeyOrder::Make(sorted);
}

void mergeKeyOrderMap(KeyOrderMap& dst, const KeyOrderMap& src) {
  for (auto const& [key, count] : src) {
    auto iter = dst.find(key);
    if (iter == dst.end()) {
      dst.emplace(key, count);
    } else {
      iter->second = iter->second + count;
    }
  }
}

}
