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

#ifndef __HPHP_TAINT_DATA_H__
#define __HPHP_TAINT_DATA_H__

#include <runtime/base/taint/taint_trace_node.h>

/*
 * Taint bits have the semantic of being propagated by OR; untainted then
 * implies a semantic of propagation by AND. Note that some taint bits are
 * bit-packed flags and do not function as taints.
 */
#define TAINT_BIT_NONE          (0x00)
#define TAINT_BIT_HTML          (0x01)
#define TAINT_BIT_MUTATED       (0x02)
#define TAINT_BIT_SQL           (0x04)
#define TAINT_BIT_SHELL         (0x08)
#define TAINT_BIT_ALL_NO_TRACE  (0x0f)
#define TAINT_BIT_TRACE_HTML    (0x10)
#define TAINT_BIT_ALL           (0x1f)
#define TAINT_BIT_TRACE_SELF    (0x20)

#define TAINT_BITS_RESERVED    (0xe0000000)
#define TAINT_GET_TAINT(bits)  ((bits) & (~TAINT_BITS_RESERVED))
#define TAINT_GET_FLAGS(bits)  ((bits) & (TAINT_BITS_RESERVED))

#ifdef TAINTED

namespace HPHP {

typedef int taint_t;

class TaintData {
public:
  TaintData() : m_taint_bits(TAINT_BIT_NONE), m_taint_trace() { }

  taint_t getTaint() const { return TAINT_GET_TAINT(m_taint_bits); }
  taint_t getRawTaint() const { return m_taint_bits; }
  void setTaint(taint_t bits) { m_taint_bits |= bits; }
  void unsetTaint(taint_t bits) {
    m_taint_bits &= (~bits);
    if (bits & TAINT_BIT_HTML) { dropTaintTrace(); }
  }

  const TaintTraceNodePtr& getTaintTrace() const { return m_taint_trace; }
  void setTaintTrace(TaintTraceNodePtr trace);
  void setTaintTrace(TaintTraceDataPtr data);
  void attachTaintTrace(TaintTraceNodePtr trace);
  void attachTaintTrace(TaintTraceDataPtr data);
  void dropTaintTrace();

  void dump() const;

private:
  taint_t m_taint_bits;
  TaintTraceNodePtr m_taint_trace;
};

}

#endif // TAINTED

#endif // __HPHP_TAINT_DATA_H__
