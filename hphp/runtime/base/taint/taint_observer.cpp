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

#ifdef TAINTED

#include <runtime/base/complex_types.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/taint/taint_trace.h>

namespace HPHP {

IMPLEMENT_THREAD_LOCAL(TaintObserver*, TaintObserver::instance);

void TaintObserver::RegisterAccessed(const TaintData& td) {
  if (!IsActive()) { return; }

  // Prevent recursive calls into the TaintObserver.
  TaintObserver *tc = *instance;
  TAINT_OBSERVER_CAP_STACK();

  // If the string is HTML-untainted, set the HTML_CLEAN flag.
  if (!(td.getTaint() & TAINT_BIT_HTML)) {
    tc->m_current_taint.setTaint(TAINT_FLAG_HTML_CLEAN);
  }

  // Absorb the taint and any trace data.
  tc->m_current_taint.setTaint(td.getTaint());
  if (TaintTracer::IsTraceEnabled(TAINT_BIT_TRACE_HTML)) {
    tc->m_current_taint.attachTaintTrace(td.getTaintTrace());
  }
}

void TaintObserver::RegisterMutated(TaintData& td, const char *s) {
  if (!IsActive()) { return; }

  // Prevent recursive calls into the TaintObserver.
  TaintObserver *tc = *instance;
  TAINT_OBSERVER_CAP_STACK();

  taint_t t = tc->m_current_taint.getTaint();
  taint_t set_mask = tc->m_set_mask;
  taint_t clear_mask = tc->m_clear_mask;

  // Trace the passed string if we're asked to inside some extension function.
  if ((set_mask & TAINT_BIT_TRACE_HTML) && s) {
    td.attachTaintTrace(NEW(TaintTraceData)(TaintTracer::Trace(s, true, true)));
  }

  bool do_trace = (t & TAINT_BIT_HTML) &&
                  (t & TAINT_BIT_TRACE_HTML) &&
                  (tc->m_current_taint.getRawTaint() & TAINT_FLAG_HTML_CLEAN);

  // Perform HTML trace as desired.
  if (TaintTracer::IsTraceEnabled(TAINT_BIT_TRACE_HTML) && do_trace) {
    t &= ~TAINT_BIT_TRACE_HTML;
    TaintTraceDataPtr ttd = TaintTracer::CreateTrace();
    tc->m_current_taint.attachTaintTrace(ttd);
  }

  // Propagate the taint and any trace data.
  td.setTaint(set_mask | (~clear_mask & t));
  if (TaintTracer::IsTraceEnabled(TAINT_BIT_TRACE_HTML)) {
    td.setTaintTrace(tc->m_current_taint.getTaintTrace());
  }
}

}

#endif // TAINTED
