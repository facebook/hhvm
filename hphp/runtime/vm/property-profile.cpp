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

#include "hphp/runtime/vm/property-profile.h"

#include <deque>

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/util/alloc.h"
#include "hphp/util/trace.h"

namespace HPHP::PropertyProfile {

//////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(prof_prop)

// (class, prop)
using ClassPropPair = std::pair<const StringData*, const StringData*>;

struct ClassPropPairHash {
  size_t operator() (const ClassPropPair& p) {
    return hash_int64_pair(p.first->hash(), p.second->hash());
  }
};

using PropertyCountAddrMap = folly::ConcurrentHashMap<
  ClassPropPair,
  uint32_t*,                            // pointer to the value
  ClassPropPairHash,
  std::equal_to<ClassPropPair>,
  VMAllocator<uint8_t>
>;

std::mutex s_counter_creation_lock;
PropertyCountAddrMap* s_counter_addr_map{};
std::deque<uint32_t, VMAllocator<uint32_t>> s_counters;
std::vector<ClassPropPair, VMAllocator<ClassPropPair>> s_count_keys;

}

//////////////////////////////////////////////////////////////////////

uint32_t* getCounterAddr(const StringData* cls, const StringData* prop) {
  auto fullName = ClassPropPair(cls, prop);

  if (UNLIKELY(s_counter_addr_map == nullptr)) {
    // Grab lock when initializing
    std::lock_guard<std::mutex> _{s_counter_creation_lock};
    if (s_counter_addr_map == nullptr) {
      s_counter_addr_map = new PropertyCountAddrMap(/* size hint */ 65536);
    }
  }

  auto iter = s_counter_addr_map->find(fullName);
  if (iter == s_counter_addr_map->end()) {
    // We need to insert it, but just in case someone else just inserted it,
    // let's grab the lock and check again.
    std::lock_guard<std::mutex> _{s_counter_creation_lock};
    iter = s_counter_addr_map->find(fullName);
    if (iter == s_counter_addr_map->end()) {
      s_count_keys.emplace_back(fullName);
      s_counters.push_back(0);
      iter = s_counter_addr_map->insert(fullName, &(s_counters.back())).first;
    }
  }
  return iter->second;
}

uint32_t getCount(const StringData* cls, const StringData* prop) {
  if (s_counter_addr_map == nullptr) return 0;
  if (s_counter_addr_map->empty()) return 0;

  auto fullName = ClassPropPair(cls, prop);
  auto iter = s_counter_addr_map->find(fullName);
  if (iter != s_counter_addr_map->end()) return *(iter->second);
  return 0;
}

void serialize(jit::ProfDataSerializer& ser) {
  // Grab our own copy of the keys so we can iterate at will
  auto const keys = [&] {
    std::lock_guard<std::mutex> _{s_counter_creation_lock};
    return s_count_keys;
  }();

  // Keys should be unique. Verify it.
  hphp_fast_set<ClassPropPair> seen;

  write_raw(ser, keys.size());
  FTRACE(1, "PropertyProfile::serialize ({} entries):\n", keys.size());
  for (auto const& key : keys) {
    always_assert_flog(
      seen.emplace(key).second,
      "Attempting to serialize duplicate PropertyProfile entry: {}::{}",
      key.first->data(), key.second->data()
    );
    auto const count = getCount(key.first, key.second);
    FTRACE(1, "  {}::{} = {}\n",
           key.first->data(), key.second->data(), count);
    write_string(ser, key.first);
    write_string(ser, key.second);
    write_raw(ser, count);
  }
}

void deserialize(jit::ProfDataDeserializer& ser) {
  size_t elems;
  read_raw(ser, elems);
  FTRACE(1, "PropertyProfile::deserialize ({} entries):\n", elems);
  while (elems--) {
    auto const clsName = read_string(ser);
    auto const propName = read_string(ser);
    uint32_t count;
    read_raw(ser, count);
    FTRACE(1, "  {}::{} = {}\n", clsName->data(), propName->data(), count);
    uint32_t* counter = getCounterAddr(clsName, propName);
    *counter = count;
  }
}

void clear() {
  if (s_counter_addr_map == nullptr) return;
  std::lock_guard<std::mutex> _(s_counter_creation_lock);
  s_counter_addr_map->clear();
  s_counters.clear();
  s_count_keys.clear();
}

//////////////////////////////////////////////////////////////////////

}
