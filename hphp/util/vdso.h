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

#ifndef incl_HPHP_UTIL_VDSO_H
#define incl_HPHP_UTIL_VDSO_H

#include "hphp/util/base.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/util.h"

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

class Vdso {
public:
  Vdso();
  ~Vdso();

  static int64_t ClockGetTimeNS(int clk_id);
  static int ClockGetTime(int clk_id, timespec *ts);

  ALWAYS_INLINE int clockGetTime(int clk_id, timespec *ts);
  ALWAYS_INLINE int64_t clockGetTimeNS(int clk_id);

private:
  void *m_handle;
  int (*m_clock_gettime)(clockid_t, timespec *ts);
  int64_t (*m_clock_gettime_ns)(clockid_t);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_UTIL_HARDWARE_COUNTER_H__
