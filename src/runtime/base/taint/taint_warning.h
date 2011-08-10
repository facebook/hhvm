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

#ifndef __HPHP_TAINT_WARNING_H__
#define __HPHP_TAINT_WARNING_H__

#ifdef TAINTED

#include <runtime/base/types.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/taint/taint_data.h>

namespace HPHP {

class TaintWarningRequestData : public RequestEventHandler {
public:
  virtual void requestInit() {
    m_counts.set(TAINT_BIT_HTML,    0);
    m_counts.set(TAINT_BIT_MUTATED, 0);
    m_counts.set(TAINT_BIT_SQL,     0);
    m_counts.set(TAINT_BIT_SHELL,   0);
    m_counts.set(TAINT_BIT_ALL,     0);
  }
  virtual void requestShutdown() { m_counts.clear(); }

  int incCount(taint_t bit) { return m_counts.lvalAt(bit)++; }
  void zeroCount(taint_t bit) { m_counts.lvalAt(bit) = 0; }
  CArrRef getCounts() { return m_counts; }

private:
  Array m_counts;
};

class TaintWarning {
public:
  static void WarnIfTainted(CStrRef s, const taint_t bit);

  static int IncCount(taint_t bit) { return s_requestdata->incCount(bit); }
  static void ZeroCount(taint_t bit) { s_requestdata->zeroCount(bit); }
  static CArrRef GetCounts() { return s_requestdata->getCounts(); }

private:
  DECLARE_STATIC_REQUEST_LOCAL(TaintWarningRequestData, s_requestdata);
};

}

#endif // TAINTED

#endif // __HPHP_TAINT_WARNING_H__

