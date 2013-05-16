/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/base/memory/leak_detectable.h"
#include "hphp/runtime/base/program_functions.h"
#include "hphp/runtime/base/server/server_stats.h"
#include <stdlib.h>
#include "tbb/concurrent_hash_map.h"
#include "hphp/runtime/base/runtime_option.h"
#ifdef GOOGLE_HEAP_PROFILER
#include "hphp/util/atomic.h"
#include "google/malloc_hook.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int LeakDetectable::TotalCount = 0;
int LeakDetectable::TotalAllocated = 0;
Mutex LeakDetectable::LeakMutex;
LeakDetectable::StackTraceMap LeakDetectable::AllocStackTraces;

LeakDetectable::LeakDetectable() : m_reachable(false) {
  Lock lock(LeakMutex);
  TotalAllocated++;
  TotalCount++;

  AllocStackTraces.operator[](this);
}

LeakDetectable::~LeakDetectable() {
  Lock lock(LeakMutex);
  TotalCount--;

  StackTraceMap::iterator iter = AllocStackTraces.find(this);
  assert(iter != AllocStackTraces.end());
  AllocStackTraces.erase(iter);
}

void LeakDetectable::markReachable() {
  assert(!m_reachable);
  m_reachable = true;
}

void LeakDetectable::BeginLeakChecking() {
  Lock lock(LeakMutex);
  for (StackTraceMap::iterator iter = AllocStackTraces.begin();
       iter != AllocStackTraces.end(); ++iter) {
    iter->first->m_reachable = false;
  }
}

int LeakDetectable::EndLeakChecking(std::string &dumps, int sampling) {
  vector<string> objDumps;
  vector<StackTrace> objStackTraces;
  int count = 0;

  {
    Lock lock(LeakMutex);
    for (StackTraceMap::iterator iter = AllocStackTraces.begin();
         iter != AllocStackTraces.end(); ++iter) {
      if (!iter->first->m_reachable) {
        ++count;
        if (rand() % sampling == 0) {
          string out;
          iter->first->dump(out);
          objDumps.push_back(out);
          objStackTraces.push_back(iter->second);
        }
      }
    }
  }

  for (unsigned int i = 0; i < objDumps.size(); i++) {
    dumps += "---------------------------------------------------------\n";
    dumps += "Leaked Item ";
    dumps += boost::lexical_cast<string>(i+1);
    dumps += ":\n";
    dumps += objDumps[i];
    dumps += objStackTraces[i].toString();
  }

  return count;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef GOOGLE_HEAP_PROFILER
class AllocData {
public:
  AllocData() : m_peak_malloced(0), m_current_malloced(0) {}

  int getPeakUsage() const { return m_peak_malloced;}
  int getLeaked() const { return m_current_malloced;}

  void on_malloc(const void *ptr, size_t size) {
    m_objects[ptr] = size;
    m_current_malloced += size;
    if (m_current_malloced > m_peak_malloced) {
      m_peak_malloced = m_current_malloced;
    }
  }

  void on_free(const void *ptr) {
    std::map<const void*, size_t>::iterator iter = m_objects.find(ptr);
    if (iter != m_objects.end()) {
      m_current_malloced -= iter->second;
      m_objects.erase(iter);
    }
  }

private:
  int m_peak_malloced;
  int m_current_malloced;
  std::map<const void*, size_t> m_objects;
};
static IMPLEMENT_THREAD_LOCAL(AllocData, s_allocs);

static void hphp_malloc_hook_size_only(const void *ptr, size_t size) {
  MallocHook::SetNewHook(nullptr);
  s_allocs->on_malloc(ptr, size);
  MallocHook::SetNewHook(hphp_malloc_hook_size_only);
}

static void hphp_free_hook_size_only(const void *ptr) {
  MallocHook::SetDeleteHook(nullptr);
  s_allocs->on_free(ptr);
  MallocHook::SetDeleteHook(hphp_free_hook_size_only);
}
#endif

void LeakDetectable::EnableMallocStats(bool enable) {
#ifdef GOOGLE_HEAP_PROFILER
  if (enable) {
    MallocHook::SetDeleteHook(hphp_free_hook_size_only);
    MallocHook::SetNewHook(hphp_malloc_hook_size_only);
  } else {
    MallocHook::SetNewHook(nullptr);
    MallocHook::SetDeleteHook(nullptr);
  }
#endif
}

void LeakDetectable::LogMallocStats() {
#ifdef GOOGLE_HEAP_PROFILER
  ServerStats::Log("mem.malloc.peak", s_allocs->getPeakUsage());
  ServerStats::Log("mem.malloc.leaked", s_allocs->getLeaked());
#endif
}

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StackTrace);
struct AllocRecord {
  time_t time;
  StackTracePtr st;
  size_t size;
};

static Mutex sampling_mutex;
typedef tbb::concurrent_hash_map<const void*, AllocRecord> SamplingMap;
static SamplingMap sampling_traces;

#ifdef GOOGLE_HEAP_PROFILER
static int sampling_counter = 0;
static void hphp_malloc_hook(const void *ptr, size_t size) {
  if (LeakDetectable::MallocSampling &&
      atomic_inc(sampling_counter) % LeakDetectable::MallocSampling == 0) {
    Lock lock(sampling_mutex);
    MallocHook::SetNewHook(nullptr);
    SamplingMap::accessor acc;
    sampling_traces.insert(acc, ptr);
    AllocRecord &alloc = acc->second;
    alloc.time = time(nullptr);
    alloc.st = StackTracePtr(new StackTrace());
    alloc.size = size;
    MallocHook::SetNewHook(hphp_malloc_hook);
  }
}

static void hphp_free_hook(const void *ptr) {
  sampling_traces.erase(ptr);
}
#endif

int LeakDetectable::MallocSampling = 0;
int LeakDetectable::StackTraceGroupLevel = 6;

void LeakDetectable::BeginMallocSampling() {
  if (MallocSampling == 0) return;
#ifdef GOOGLE_HEAP_PROFILER
  sampling_counter = 0;
  MallocHook::SetDeleteHook(hphp_free_hook);
  MallocHook::SetNewHook(hphp_malloc_hook);
#endif
}

class LeakStats {
public:
  LeakStats() : count(0) {}
  string hash;
  int count;
  size_t totalSize;
  vector<string> examples;
};
static bool SortByCount(const LeakStats *s1, const LeakStats *s2) {
  return s1->count < s2->count;
}

void LeakDetectable::EndMallocSampling(std::string &dumps, int cutoff) {
  if (MallocSampling == 0) return;

  // grouping
  std::map<std::string, LeakStats> leaked_stacks;
  {
    // turn off hooks
    Lock lock(sampling_mutex);
#ifdef GOOGLE_HEAP_PROFILER
    MallocHook::SetNewHook(nullptr);
    MallocHook::SetDeleteHook(nullptr);
#endif

    time_t cutoff_time = time(nullptr) - cutoff;
    for (SamplingMap::iterator iter =
           sampling_traces.begin(); iter != sampling_traces.end(); ++iter) {
      if (iter->second.time > cutoff_time) continue; // too new

      StackTrace &st = *iter->second.st;
      string hash = st.hexEncode(4, StackTraceGroupLevel + 4);
      LeakStats &stats = leaked_stacks[hash];
      if (stats.count++ == 0) {
        stats.hash = hash;
        stats.totalSize = 0;
      }
      stats.totalSize += iter->second.size;
      if (stats.examples.size() < 5) {
        stats.examples.push_back(st.hexEncode());
      }
    }
    sampling_traces.clear();
  }

  // sorting
  vector<LeakStats*> stats;
  stats.reserve(leaked_stacks.size());
  for (std::map<std::string, LeakStats>::iterator iter =
         leaked_stacks.begin(); iter != leaked_stacks.end(); ++iter) {
    stats.push_back(&iter->second);
  }
  sort(stats.begin(), stats.end(), SortByCount);

  // reporting
  int totalLeaked = 0;
  int totalGroup = 0;
  for (unsigned int i = 0; i < stats.size(); i++) {
    LeakStats *stat = stats[i];
    string translated("");
    if (RuntimeOption::TranslateLeakStackTrace) {
      translated = translate_stack(stat->hash.c_str());
    }
    if (!SuppressStackTrace(translated)) {
      totalLeaked += stat->count;
      totalGroup++;

      dumps += "---------------------------------------------------------\n";
      dumps += lexical_cast<string>(i + 1) + ". Leaked ";
      dumps += lexical_cast<string>(stat->count) + " objects ";
      dumps += lexical_cast<string>(stat->totalSize) + " bytes, ";
      dumps += stat->hash + ":\n\n";
      for (unsigned int j = 0; j < stat->examples.size(); j++) {
        dumps += "  (";
        dumps += lexical_cast<string>(j + 1) + ") ";
        dumps += stat->examples[j] + "\n";
      }
      dumps += "\n";
      dumps += translated;
    }
  }
  dumps += "---------------------------------------------------------\n";
  dumps += "Total Leaked " + lexical_cast<string>(totalLeaked);
  dumps += " in " + lexical_cast<string>(totalGroup) + " groups.\n";
}

bool LeakDetectable::SuppressStackTrace(const std::string &st) {
  static const char *known_okay_stacks[] = {
    "gdCacheGet",
    "ThreadSharedStore::",
    "ThreadSharedVariant::ThreadSharedVariant",
    "NS_NewPermanentAtom",
    "Transliterator::createBasicInstance",
  };

  for (unsigned int i = 0;
       i < sizeof(known_okay_stacks)/sizeof(known_okay_stacks[0]); i++) {
    if (st.find(known_okay_stacks[i]) != string::npos) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
