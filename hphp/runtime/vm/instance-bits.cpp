/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/instance-bits.h"

#include <vector>
#include <algorithm>
#include <tbb/concurrent_hash_map.h>

#include "folly/ScopeGuard.h"

#include "hphp/util/base.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace InstanceBits {

//////////////////////////////////////////////////////////////////////

namespace {

typedef tbb::concurrent_hash_map<
  const StringData*, uint64_t, pointer_hash<StringData>> InstanceCounts;
typedef hphp_hash_map<const StringData*, unsigned,
                      pointer_hash<StringData>> InstanceBitsMap;

InstanceCounts s_instanceCounts;
ReadWriteMutex s_instanceCountsLock(RankInstanceCounts);
InstanceBitsMap s_instanceBitsMap;

const size_t kNumInstanceBits = sizeof(BitSet) * CHAR_BIT;

// Tracked just for an assertion in lookup().
std::atomic<bool> s_currentlyInitializing{false};

}

//////////////////////////////////////////////////////////////////////

ReadWriteMutex lock(RankInstanceBits);
std::atomic<bool> initFlag{false};

//////////////////////////////////////////////////////////////////////

void profile(const StringData* name) {
  assert(name->isStatic());
  unsigned inc = 1;
  Class* c = Unit::lookupClass(name);
  if (c && (c->attrs() & AttrInterface)) {
    // Favor interfaces
    inc = 250;
  }

  // The extra layer of locking is here so that InstanceBits::init can safely
  // iterate over s_instanceCounts while building its map of names to bits.
  ReadLock l(s_instanceCountsLock);
  InstanceCounts::accessor acc;
  if (!s_instanceCounts.insert(acc, InstanceCounts::value_type(name, inc))) {
    acc->second += inc;
  }
}

void init() {
  assert(JIT::Translator::WriteLease().amOwner());
  if (initFlag.load(std::memory_order_acquire)) return;

  s_currentlyInitializing = true;
  SCOPE_EXIT { s_currentlyInitializing = false; };

  // First, grab a write lock on s_instanceCounts and grab the current set of
  // counts as quickly as possible to minimize blocking other threads still
  // trying to profile instance checks.
  typedef std::pair<const StringData*, unsigned> Count;
  std::vector<Count> counts;
  uint64_t total = 0;
  {
    // If you think of the read-write lock as a shared-exclusive lock instead,
    // the fact that we're grabbing a write lock to iterate over the table
    // makes more sense: it's safe to concurrently modify a
    // tbb::concurrent_hash_map, but iteration is not guaranteed to be safe
    // with concurrent insertions.
    WriteLock l(s_instanceCountsLock);
    for (auto& pair : s_instanceCounts) {
      counts.push_back(pair);
      total += pair.second;
    }
  }
  std::sort(counts.begin(), counts.end(), [&](const Count& a, const Count& b) {
    return a.second > b.second;
  });

  // Next, initialize s_instanceBitsMap with the top 127 most checked
  // classes. Bit 0 is reserved as an 'initialized' flag
  unsigned i = 1;
  uint64_t accum = 0;
  for (auto& item : counts) {
    if (i >= kNumInstanceBits) break;
    if (Class* cls = Unit::lookupUniqueClass(item.first)) {
      if (!(cls->attrs() & AttrUnique)) {
        continue;
      }
    }
    s_instanceBitsMap[item.first] = i;
    accum += item.second;
    ++i;
  }

  // Print out stats about what we ended up using
  if (Trace::moduleEnabledRelease(Trace::instancebits, 1)) {
    Trace::traceRelease("%s: %u classes, %" PRIu64 " (%.2f%%) of warmup"
                        " checks\n",
                        __FUNCTION__, i-1, accum, 100.0 * accum / total);
    if (Trace::moduleEnabledRelease(Trace::instancebits, 2)) {
      accum = 0;
      i = 1;
      for (auto& pair : counts) {
        if (i >= 256) {
          Trace::traceRelease("skipping the remainder of the %zu classes\n",
                              counts.size());
          break;
        }
        accum += pair.second;
        Trace::traceRelease("%3u %5.2f%% %7u -- %6.2f%% %7" PRIu64 " %s\n",
                            i++, 100.0 * pair.second / total, pair.second,
                            100.0 * accum / total, accum,
                            pair.first->data());
      }
    }
  }

  // Finally, update m_instanceBits on every Class that currently exists. This
  // must be done while holding a lock that blocks insertion of new Classes
  // into their class lists, but in practice most Classes will already be
  // created by now and this process takes at most 10ms.
  WriteLock l(InstanceBits::lock);
  for (AllClasses ac; !ac.empty(); ) {
    Class* c = ac.popFront();
    c->setInstanceBitsAndParents();
  }

  initFlag.store(true, std::memory_order_release);
}

unsigned lookup(const StringData* name) {
  assert(s_currentlyInitializing || initFlag.load(std::memory_order_acquire));

  unsigned bit;
  if (!mapGet(s_instanceBitsMap, name, &bit)) return 0;
  assert(bit >= 1 && bit < kNumInstanceBits);
  return bit;
}

bool getMask(const StringData* name, int& offset, uint8_t& mask) {
  assert(initFlag.load(std::memory_order_acquire));

  unsigned bit = lookup(name);
  if (!bit) return false;

  const size_t bitWidth = sizeof(mask) * CHAR_BIT;
  offset = Class::instanceBitsOff() + bit / bitWidth * sizeof(mask);
  mask = 1u << (bit % bitWidth);
  return true;
}

//////////////////////////////////////////////////////////////////////

}}
