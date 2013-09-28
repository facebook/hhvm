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

#include "hphp/runtime/vm/treadmill.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include <list>

#include "hphp/util/trace.h"
#include "hphp/util/base.h"
#include "hphp/util/rank.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP {  namespace Treadmill {

TRACE_SET_MOD(treadmill);

namespace {
static pthread_mutex_t s_genLock = PTHREAD_MUTEX_INITIALIZER;
static GenCount s_gen = 1;
static const GenCount kIdleGenCount = 0; // not processing any requests.
static GenCount* s_inflightRequests;
static int s_maxThreadID;

struct GenCountGuard {
  GenCountGuard() {
    checkRank(RankTreadmill);
    pthread_mutex_lock(&s_genLock);
    pushRank(RankTreadmill);
    if (!s_inflightRequests) {
      s_maxThreadID = 2;
      s_inflightRequests = (GenCount*)calloc(sizeof(GenCount), s_maxThreadID);
      static_assert(kIdleGenCount == 0, "kIdleGenCount should be zero");
    }
  }
  ~GenCountGuard() {
    popRank(RankTreadmill);
    pthread_mutex_unlock(&s_genLock);
  }
};
}

static GenCount* idToCount(int threadID) {
  if (threadID >= s_maxThreadID) {
    int newSize = threadID + 1;
    s_inflightRequests = (GenCount*)realloc(s_inflightRequests,
                                           sizeof(GenCount) * newSize);
    for (int i = s_maxThreadID; i < newSize; i++) {
      s_inflightRequests[i] = kIdleGenCount;
    }
    s_maxThreadID = newSize;
  }
  return s_inflightRequests + threadID;
}

typedef std::list<WorkItem*> PendingTriggers;
static PendingTriggers s_tq;

// Inherently racy. We get a lower bound on the generation; presumably
// clients are aware of this, and are creating the trigger for an object
// that was reachable strictly in the past.
WorkItem::WorkItem() : m_gen(s_gen) {
}

void WorkItem::enqueue(WorkItem* gt) {
  GenCountGuard g;
  gt->m_gen = s_gen++;
  s_tq.push_back(gt);
}

void startRequest(int threadId) {
  GenCountGuard g;
  assert(*idToCount(threadId) == kIdleGenCount);
  TRACE(1, "tid %d start @gen %d\n", threadId, int(s_gen));
  *idToCount(threadId) = s_gen;
}

void finishRequest(int threadId) {
  TRACE(1, "tid %d finish\n", threadId);
  std::vector<WorkItem*> toFire;
  {
    GenCountGuard g;
    assert(*idToCount(threadId) != kIdleGenCount);
    *idToCount(threadId) = kIdleGenCount;

    // After finishing a request, check to see if we've allowed any triggers
    // to fire.
    PendingTriggers::iterator it = s_tq.begin();
    PendingTriggers::iterator end = s_tq.end();
    if (it != end) {
      GenCount gen = (*it)->m_gen;
      GenCount limit = s_gen + 1;
      for (int i = 0; i < s_maxThreadID; ++i) {
        if (s_inflightRequests[i] != kIdleGenCount &&
            s_inflightRequests[i] < limit) {
          limit = s_inflightRequests[i];
          if (limit <= gen) break;
        }
      }
      do {
        TRACE(2, "considering delendum %d\n", int((*it)->m_gen));
        if ((*it)->m_gen >= limit) {
          TRACE(2, "not unreachable! %d\n", int((*it)->m_gen));
          break;
        }
        toFire.push_back(*it);
        it = s_tq.erase(it);
      } while (it != end);
    }
  }
  for (unsigned i = 0; i < toFire.size(); ++i) {
    (*toFire[i])();
    delete toFire[i];
  }
}

FreeMemoryTrigger::FreeMemoryTrigger(void* ptr) : m_ptr(ptr) {
  TRACE(3, "FreeMemoryTrigger @ %p, m_f %p\n", this, m_ptr);
}

void FreeMemoryTrigger::operator()() {
  TRACE(3, "FreeMemoryTrigger: Firing @ %p , m_f %p\n", this, m_ptr);
  free(m_ptr);
}

void deferredFree(void* p) {
  WorkItem::enqueue(new FreeMemoryTrigger(p));
}

}}
