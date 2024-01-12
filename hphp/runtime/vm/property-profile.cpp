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

#include <atomic>
#include <cinttypes>
#include <tbb/concurrent_hash_map.h>

#include "hphp/runtime/base/string-functors.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

namespace HPHP::PropertyProfile {

//////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(prof_prop);

// (class, prop)
using ClassPropPair = std::pair<const StringData*, const StringData*>;

struct ClassPropPairHashCompare {
  bool equal(const ClassPropPair& p1, const ClassPropPair& p2) const {
    assertx(p1.first && p1.second && p2.first && p2.second);
    return p1.first->tsame(p2.first) && p1.second->same(p2.second);
  }
  size_t hash(const std::pair<const StringData*, const StringData*>& p) const {
    assertx(p.first && p.second);
    return folly::hash::hash_combine(p.first->hash(), p.second->hash());
  }
};

using PropertyCounts = tbb::concurrent_hash_map<
  ClassPropPair,
  uint32_t,
  ClassPropPairHashCompare
>;
PropertyCounts s_counts;

// It's not safe to iterate across a concurrent_hash_map, so store the
// keys separately (protected by a lock). When we need to serialize
// the map, we can safely iterate across the key list and do normal
// gets on the map. The lock shouldn't be an issue because we only
// insert into it the first time a key is seen.
std::mutex s_count_keys_lock;
std::vector<ClassPropPair> s_count_keys;

}

//////////////////////////////////////////////////////////////////////

void incCount(const StringData* cls, const StringData* prop) {
  auto fullName = ClassPropPair(cls, prop);

  PropertyCounts::accessor acc;
  if (!s_counts.insert(acc, PropertyCounts::value_type(fullName, 1))) {
    acc->second++;
  } else {
    std::lock_guard<std::mutex> _{s_count_keys_lock};
    s_count_keys.emplace_back(fullName);
  }
}

uint32_t getCount(const StringData* cls, const StringData* prop) {
  auto fullName = ClassPropPair(cls, prop);

  PropertyCounts::accessor acc;
  if (s_counts.find(acc, fullName)) return acc->second;
  return 0;
}

void serialize(jit::ProfDataSerializer& ser) {
  // Grab our own copy of the keys so we can iterate at will
  auto const keys = [&] {
    std::lock_guard<std::mutex> _{s_count_keys_lock};
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
    auto const fullName = ClassPropPair(clsName, propName);
    uint32_t count;
    read_raw(ser, count);
    FTRACE(1, "  {}::{} = {}\n", clsName->data(), propName->data(), count);
    PropertyCounts::accessor acc;
    if (!s_counts.insert(acc, PropertyCounts::value_type(fullName, count))) {
      always_assert_flog(
        false,
        "found duplicate entry {}::{} while deserializing PropertyProfile",
        fullName.first->data(), fullName.second->data()
      );
    }
  }
}

//////////////////////////////////////////////////////////////////////

}
