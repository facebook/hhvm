/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_PASS_TRACER_H_
#define incl_HPHP_PASS_TRACER_H_

#include <string>

#include <folly/Format.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/print.h"

namespace HPHP { namespace jit {

struct IRUnit;
struct Vunit;

//////////////////////////////////////////////////////////////////////

/*
 * Utility class for tracing HHIR or Vasm before and after a pass that has its
 * own trace module.  The PassTracer bumps the trace level by 1, so that all
 * pass-internal traces starting at level 1 will actually show up at level 2:
 * this is so we have regularity that level 1 always just prints before and
 * after with no pass-internal traces.
 */
template<class Unit>
struct PassTracerImpl {
  explicit PassTracerImpl(const Unit* unit,
                          Trace::Module mod,
                          const char* name)
    : m_unit(*unit)
    , m_mod(mod)
    , m_name(name)
    , m_bumper{m_mod, 1}
  {
    traceUnit("before");
  }

  ~PassTracerImpl() {
    traceUnit("after");
  }

private:
  PassTracerImpl(const PassTracerImpl&) = delete;
  PassTracerImpl& operator=(const PassTracerImpl&) = delete;

private:
  void traceUnit(const char* when) const {
    FTRACE_MOD(m_mod, 0, "{}{}\n{}",
      banner(folly::sformat("{} {}", when, m_name).c_str()),
      show(m_unit),
      banner("")
    );
  }

private:
  DEBUG_ONLY const Unit& m_unit;
  Trace::Module const m_mod;
  DEBUG_ONLY const char* const m_name;
  Trace::Bump m_bumper;
};

//////////////////////////////////////////////////////////////////////

using PassTracer = PassTracerImpl<IRUnit>;
using VpassTracer = PassTracerImpl<Vunit>;

//////////////////////////////////////////////////////////////////////

}}


#endif
