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

#include "hphp/util/lock.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

LockProfiler::PFUNC_PROFILE LockProfiler::s_pfunc_profile = nullptr;
bool LockProfiler::s_profile = false;
int LockProfiler::s_profile_sampling = 1000;

LockProfiler::LockProfiler(bool profile) : m_profiling(false) {
  if (s_profile && s_pfunc_profile && profile &&
      s_profile_sampling && (rand() % s_profile_sampling) == 0) {
    m_profiling = true;
    Timer::GetMonotonicTime(m_lockTime);
  }
}

LockProfiler::~LockProfiler() {
  if (m_profiling) {
    timespec unlockTime;
    Timer::GetMonotonicTime(unlockTime);
    time_t dsec = unlockTime.tv_sec - m_lockTime.tv_sec;
    long dnsec = unlockTime.tv_nsec - m_lockTime.tv_nsec;
    int64_t dusec = dsec * 1000000 + dnsec / 1000;

    StackTrace st;
    s_pfunc_profile(st.hexEncode(3, 9), dusec);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
