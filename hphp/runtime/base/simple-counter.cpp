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

#include "hphp/runtime/base/simple-counter.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/lock.h"
#include <stdio.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool SimpleCounter::Enabled = false;
int SimpleCounter::SampleStackCount = 0;
int SimpleCounter::SampleStackDepth = 5;

IMPLEMENT_STATIC_REQUEST_LOCAL(SimpleCounter, s_counter);

void SimpleCounter::requestInit() {
  m_counters.clear();
  m_stacks.clear();
}

bool SimpleCounter::Comparer::operator()(const string &s1, const string &s2) {
  return m_map[s1] > m_map[s2];
}

static Mutex s_outputMutex;

void SimpleCounter::requestShutdown() {
  Lock lock(s_outputMutex);
  for (CounterMap::const_iterator it = m_counters.begin();
       it != m_counters.end(); ++it) {
    fprintf(stderr, "====================\n");
    fprintf(stderr, "%s : %d\n", it->first.c_str(), it->second);
    if (SampleStackCount > 0) {
      CounterMap cm;
      vector<string> &stackVec = m_stacks[it->first];
      for (size_t i = 0; i < stackVec.size(); i++) {
        cm[stackVec[i]]++;
      }
      vector<string> unique;
      for (CounterMap::const_iterator jt = cm.begin(); jt != cm.end(); ++jt) {
        unique.push_back(jt->first);
      }
      sort(unique.begin(), unique.end(), Comparer(cm));
      for (size_t i = 0; i < unique.size(); i++) {
        fprintf(stderr, "Stack #%d: %d/%d\n",
                (int)(i + 1), cm[unique[i]], (int)stackVec.size());
        StackTrace st(unique[i]);
        fprintf(stderr, "%s", st.toString().c_str());
      }
    }
  }
}

void SimpleCounter::Count(const string &name) {
  if (Enabled) {
    int count = ++s_counter->m_counters[name];
    if (SampleStackCount > 0) {
      assert(StackTrace::Enabled);
      vector<string> &stackVec = s_counter->m_stacks[name];
      if ((int)stackVec.size() < SampleStackCount ||
          f_rand(0, count - 1) < SampleStackCount) {
        StackTrace st;
        if ((int)stackVec.size() < SampleStackCount) {
          // skip StackTrace methods and the Count() call.
          stackVec.push_back(st.hexEncode(3, 3 + SampleStackDepth));
        } else {
          // skip StackTrace methods and the Count() call.
          stackVec[f_rand(0, SampleStackCount - 1)] =
            st.hexEncode(3, 3 + SampleStackDepth);
        }
      }
    }
  }
}

// For places where it is not supposed to include simple_counter.h.
void counting(const char *s) {
  COUNTING(s);
}

///////////////////////////////////////////////////////////////////////////////
}
