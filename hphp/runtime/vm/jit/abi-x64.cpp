/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/abi-x64.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

const Abi trace_abi {
  kGPUnreserved,
  kGPReserved,
  kXMMUnreserved,
  kXMMReserved,
  kCalleeSaved,
  kSF,
  true,
};

const Abi cross_trace_abi {
  trace_abi.gp() & kScratchCrossTraceRegs,
  trace_abi.gp() - kScratchCrossTraceRegs,
  trace_abi.simd() & kScratchCrossTraceRegs,
  trace_abi.simd() - kScratchCrossTraceRegs,
  trace_abi.calleeSaved & kScratchCrossTraceRegs,
  trace_abi.sf,
  false
};

auto const helper_gp = x64::rAsm | reg::r11;
auto const helper_simd = reg::xmm5 | reg::xmm6 | reg::xmm7;

const Abi helper_abi {
  helper_gp,
  trace_abi.gp() - helper_gp,
  helper_simd,
  trace_abi.simd() - helper_simd,
  trace_abi.calleeSaved,
  trace_abi.sf,
  false
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

const Abi& abi(CodeKind kind) {
  switch (kind) {
    case CodeKind::Trace:
      return trace_abi;
    case CodeKind::CrossTrace:
      return cross_trace_abi;
    case CodeKind::Helper:
      return helper_abi;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}}}
