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
#include "hphp/runtime/vm/instance-bits.h"

#include <algorithm>
#include <atomic>
#include <vector>

#include <folly/MapUtil.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/lock.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP::InstanceBits {

//////////////////////////////////////////////////////////////////////

using folly::SharedMutex;

namespace {

using InstanceBitsMap = hphp_hash_map<const StringData*,
                                      unsigned,
                                      string_data_hash,
                                      string_data_tsame>;
InstanceBitsMap s_instanceBitsMap;

Mutex s_initLock(RankInstanceBitsInit);

constexpr size_t kNumInstanceBits = sizeof(BitSet) * CHAR_BIT;

// Tracked just for an assertion in lookup().
std::atomic<pthread_t> s_initThread;

}

//////////////////////////////////////////////////////////////////////

SharedMutex g_clsInitLock;
// True if initialization has been finished
std::atomic<bool> g_initFlag{false};
// True if profiling should no longer be done
std::atomic<bool> g_profileDoneFlag{false};

//////////////////////////////////////////////////////////////////////

void profile(const Class* c) {
  assertx(c);
  if (!Cfg::Repo::Authoritative ||
      g_profileDoneFlag.load(std::memory_order_acquire)) {
    return;
  }

  // Don't profile final classes since they can be checked more efficiently via
  // direct pointer comparison through ExtendsClass than via InstanceOfBitmask.
  if (c->attrs() & AttrNoOverride) return;

  // The lowest bit needs to be zero, as the nonzero value is used as a flag to
  // prevent further changes to the counter.
  uint64_t inc = 2;
  if (c->attrs() & AttrInterface) {
    inc = 256;                          // Favor interfaces
  }

  c->incInstanceCheckCount(inc);
}

template<typename F>
void initImpl(F&& func) {
  if (g_initFlag.load(std::memory_order_acquire)) return;

  Lock l(s_initLock);
  if (g_initFlag.load(std::memory_order_acquire)) return;

  // Stop profiling before reading the counters.
  g_profileDoneFlag.store(true, std::memory_order_release);

  if (!Cfg::Repo::Authoritative) {
    g_initFlag.store(true, std::memory_order_release);
    return;
  }
  if (debug) s_initThread.store(pthread_self(), std::memory_order_release);

  func();

  // Update m_instanceBits on every Class that currently exists. This must be
  // done while holding a lock that blocks insertion of new Classes into their
  // class lists, but in practice most Classes will already be created by now
  // and this process is very fast.
  std::unique_lock clsLocker(g_clsInitLock);
  NamedType::foreach_class([&](Class* cls) {
    cls->setInstanceBitsAndParents();
  });
  NamedType::foreach_class([&](Class* cls) {
    cls->setInstanceBitsIndex(lookup(cls->name()));
  });

  if (debug) {
    // There isn't a canonical invalid pthread_t, but this is only used for the
    // assert in lookup() and it's ok to have false negatives.
    s_initThread.store(pthread_t{}, std::memory_order_release);
  }
  g_initFlag.store(true, std::memory_order_release);
}

void init() {
  initImpl(
    [] {
      using Count = std::pair<const Class*, uint64_t>;
      std::vector<Count> counts;
      uint64_t total = 0;
      NamedType::foreach_class([&](Class* cls) {
        auto count = cls->getInstanceCheckCount();
        if (count == 0) return;         // most likely
        counts.push_back({cls,  count});
        total += count;
      });
      std::sort(counts.begin(),
                counts.end(),
                [&](const Count& a, const Count& b) {
                  return a.second > b.second;
                });

      // Next, initialize s_instanceBitsMap with the top 127 most checked
      // classes. Bit 0 is reserved as an 'initialized' flag
      unsigned i = 1;
      uint64_t accum = 0;
      for (auto& item : counts) {
        if (i >= kNumInstanceBits) break;
        auto const cls = item.first;
        assertx(cls->attrs() & AttrPersistent);
        s_instanceBitsMap[cls->name()] = i;
        accum += item.second;
        ++i;
      }
      // Print out stats about what we ended up using
      if (Trace::moduleEnabledRelease(Trace::instancebits, 1)) {
        Trace::traceRelease("InstanceBits: %u classes, %" PRIu64
                            " (%.2f%%) of warmup checks\n",
                             i - 1, accum, 100.0 * accum / total);
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
            Trace::traceRelease(
                "%3u %5.2f%% %7" PRIu64" -- %6.2f%% %7" PRIu64 " %s\n",
                i++, 100.0 * pair.second / total, pair.second,
                100.0 * accum / total, accum,
                pair.first->name()->data()
            );
          }
        }
      }
    }
  );
}

bool initted() {
  return g_initFlag.load(std::memory_order_acquire);
}

unsigned lookup(const StringData* name) {
  assertx(g_initFlag.load(std::memory_order_acquire) ||
         pthread_equal(s_initThread.load(std::memory_order_acquire),
                       pthread_self()));

  if (auto const ptr = folly::get_ptr(s_instanceBitsMap, name)) {
    assertx(*ptr >= 1 && *ptr < kNumInstanceBits);
    return *ptr;
  }
  return 0;
}

bool getMask(const StringData* name, int& offset, uint8_t& mask) {
  assertx(g_initFlag.load(std::memory_order_acquire));

  unsigned bit = lookup(name);
  if (!bit) return false;

  const size_t bitWidth = sizeof(mask) * CHAR_BIT;
  offset = Class::instanceBitsOff() + bit / bitWidth * sizeof(mask);
  mask = 1u << (bit % bitWidth);
  return true;
}

void serialize(jit::ProfDataSerializer& ser) {
  assertx(g_initFlag.load(std::memory_order_acquire));

  write_raw(ser, s_instanceBitsMap.size());
  for (auto const& elm : s_instanceBitsMap) {
    write_string(ser, elm.first);
    write_raw(ser, elm.second);
  }
}

void deserialize(jit::ProfDataDeserializer& ser) {
  size_t elems;
  read_raw(ser, elems);
  if (Trace::moduleEnabledRelease(Trace::instancebits, 1)) {
    Trace::traceRelease("InstanceBits: %zu classes from deserialization\n",
                        elems);
  }
  if (!elems) return;

  auto DEBUG_ONLY done = false;
  initImpl(
    [&] {
      while (elems--) {
        auto const clsName = read_string(ser);
        auto const bit = jit::read_raw<InstanceBitsMap::mapped_type>(ser);
        s_instanceBitsMap.emplace(clsName, bit);
      }
      done = true;
    }
  );
  assertx(done);
}

//////////////////////////////////////////////////////////////////////

}
