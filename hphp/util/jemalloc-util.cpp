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

#include "hphp/util/jemalloc-util.h"

namespace HPHP {

int jemalloc_pprof_enable() {
  return mallctlWrite<bool, true>("prof.active", true);
}

int jemalloc_pprof_disable() {
  return mallctlWrite<bool, true>("prof.active", false);
}

int jemalloc_pprof_dump(const std::string& prefix, bool force) {
  if (!force) {
    bool enabled = false;
    bool active = false;
    // Check if profiling is active before trying to dump.
    int err = mallctlRead<bool, true>("opt.prof", &enabled) ||
      (enabled && mallctlRead<bool, true>("prof.active", &active));
    if (err || !active) {
      return 0; // nothing to do
    }
  }

  if (prefix != "") {
    const char *s = prefix.c_str();
    return mallctlWrite<const char*, true>("prof.dump", s);
  } else {
    return mallctlCall<true>("prof.dump");
  }
}

bool purge_all(std::string* errStr) {
#ifdef USE_JEMALLOC
  assert(mallctlnametomib && mallctlbymib);
  unsigned allArenas = 0;
#ifndef MALLCTL_ARENAS_ALL
  if (mallctlRead<unsigned, true>("arenas.narenas", &allArenas)) {
    if (errStr) {
      *errStr = "arenas.narena";
    }
    return false;
  }
#else
  allArenas = MALLCTL_ARENAS_ALL;
#endif

  size_t mib[3];
  size_t miblen = 3;
  if (mallctlnametomib("arena.0.purge", mib, &miblen)) {
    if (errStr) {
      *errStr = "mallctlnametomib(arena.0.purge)";
    }
    return false;
  }

  mib[1] = allArenas;
  if (mallctlbymib(mib, miblen, nullptr, nullptr, nullptr, 0)) {
    if (errStr) {
      *errStr = "mallctlbymib(arena.all.purge)";
    }
    return false;
  }
  return true;
#else
  return false;
#endif
}

#ifdef USE_JEMALLOC

// Frequently used mallctl mibs.

namespace {
// If a value is passed in, refresh the data from which the mallctl*() functions
// report values, and increment the epoch. Return the current epoch. This is
// useful for detecting whether another thread caused a refresh.
size_t g_epoch_mib[1];                  // "epoch"
size_t g_pactive_mib[4];                // "stats.arenas.<i>.pactive"

}

void init_mallctl_mibs() {
  size_t miblen = 1;
  mallctlnametomib("epoch", g_epoch_mib, &miblen);
  miblen = 4;
  mallctlnametomib("stats.arenas.0.pactive", g_pactive_mib, &miblen);
}

void mallctl_epoch() {
  uint64_t epoch = 1;
  mallctlbymib(g_epoch_mib, 1, nullptr, nullptr, &epoch, sizeof(epoch));
}

size_t mallctl_pactive(unsigned arenaId) {
  size_t mib[4] =
    {g_pactive_mib[0], g_pactive_mib[1], arenaId, g_pactive_mib[3]};
  size_t pactive = 0;
  size_t sz = sizeof(pactive);
  if (mallctlbymib(mib, 4, &pactive, &sz, nullptr, 0)) return 0;
  return pactive;
}

#endif
}
