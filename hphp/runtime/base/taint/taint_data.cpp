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

#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_trace.h>

namespace HPHP {

void TaintData::setTaintTrace(TaintTraceNodePtr trace) {
  assert(!TAINT_ISSET_ORIG(m_taint_bits));

  // Check if the new trace is just the old trace under a dummy node.
  bool is_retrace = m_taint_trace.get() &&
                    trace.get() && !trace->getNext().get() &&
                    m_taint_trace.get() == trace->getChild().get();

  if (trace.get() && !is_retrace) {
    m_taint_trace = trace;
  }
}

void TaintData::setTaintTrace(TaintTraceDataPtr data) {
  assert(!m_taint_trace.get());
  if (data.get()) {
    m_taint_trace = NEW(TaintTraceNode)(NULL, data);
  }
}

void TaintData::attachTaintTrace(TaintTraceNodePtr trace) {
  assert(!TAINT_ISSET_ORIG(m_taint_bits));

  // Check if the trace we're attaching is ourselves or our child/sibling.
  bool is_retrace = m_taint_trace.get() &&
                   (trace.get() == m_taint_trace.get() ||
                    trace.get() == m_taint_trace->getChild().get() ||
                    trace.get() == m_taint_trace->getNext().get());

  if (trace.get() && !is_retrace) {
    m_taint_trace = NEW(TaintTraceNode)(m_taint_trace, trace);
  }
}

void TaintData::attachTaintTrace(TaintTraceDataPtr data) {
  assert(!TAINT_ISSET_ORIG(m_taint_bits));
  if (data.get()) {
    m_taint_trace = NEW(TaintTraceNode)(m_taint_trace, data);
  }
}

void TaintData::dropTaintTrace() {
  assert(!TAINT_ISSET_ORIG(m_taint_bits));
  m_taint_trace.reset();
}

void TaintData::dump() const {
  printf("Taint: %x\n", m_taint_bits);
}

}

#endif // TAINTED
