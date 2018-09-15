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

#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/vm/disas.h"

#include <folly/Hash.h>

#include <tbb/concurrent_unordered_set.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(stats);

UserAttributeMap::Map UserAttributeMap::s_empty_map;

struct UserAttributeMap::MapCompare {
  bool operator()(const copy_ptr<Map>& a, const copy_ptr<Map>& b) const {
    if (a->size() != b->size()) return false;
    auto ait = a->begin();
    auto bit = b->begin();
    while (ait != a->end()) {
      if (ait->first != bit->first) return false;
      if (!cellSame(ait->second, bit->second)) return false;
      ++ait;
      ++bit;
    }
    return true;
  }

  size_t operator()(const copy_ptr<Map>& m) const {
    auto ret = m->size();
    for (auto& kv : *m) {
      ret = folly::hash::hash_combine(ret, kv.first.get());
      ret = folly::hash::hash_combine(ret,
                                      static_cast<int64_t>(kv.second.m_type));
      if (!isNullType(kv.second.m_type)) {
        ret = folly::hash::hash_combine(ret, kv.second.m_data.num);
      }
    }
    return ret;
  }
};

void UserAttributeMap::lookup(Map&& m) {
  if (m.empty()) {
    m_map.reset();
    return;
  }

  static std::atomic<size_t> s_count;
  static tbb::concurrent_unordered_set<copy_ptr<Map>,
                                       UserAttributeMap::MapCompare,
                                       UserAttributeMap::MapCompare> s_maps;
  s_count.fetch_add(1, std::memory_order_relaxed);
  m_map.emplace(std::move(m));

  auto ret = s_maps.insert(m_map);
  if (ret.second) {
    FTRACE(3, "Adding User Attributes {}; {} out of {}\n",
           user_attrs(this),
           s_maps.size(),
           s_count.load(std::memory_order_relaxed));
  } else {
    m_map = *ret.first;
  }
}

//////////////////////////////////////////////////////////////////////

}
