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

#include "hphp/util/vdso.h"

#define _GNU_SOURCE 1
#include <dlfcn.h>


namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

static Vdso s_vdso;

Vdso::Vdso() : m_handle(nullptr), m_clock_gettime_ns(nullptr) {
  m_handle = dlopen("linux-vdso.so.1", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
  if (!m_handle) {
    return;
  }

  m_clock_gettime_ns =
    (int64_t (*)(clockid_t))dlsym(m_handle, "__vdso_clock_gettime_ns");
  m_clock_gettime =
    (int (*)(clockid_t, timespec *))dlsym(m_handle, "__vdso_clock_gettime");
}

Vdso::~Vdso() {
  if (m_handle) {
    dlclose(m_handle);
  }
}

int64_t Vdso::ClockGetTimeNS(int clk_id) {
  return s_vdso.clockGetTimeNS(clk_id);
}

int Vdso::ClockGetTime(int clk_id, timespec *ts) {
  return s_vdso.clockGetTime(clk_id, ts);
}

ALWAYS_INLINE int64_t Vdso::clockGetTimeNS(int clk_id) {
  if (m_clock_gettime_ns)
    return m_clock_gettime_ns(clk_id);
  return -1;
}

ALWAYS_INLINE int Vdso::clockGetTime(int clk_id, timespec *ts) {
  if (m_clock_gettime) {
    memset(ts, 0, sizeof(*ts));
    return m_clock_gettime(clk_id, ts);
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
}}
