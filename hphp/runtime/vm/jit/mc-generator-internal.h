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
#ifndef incl_HPHP_MC_GENERATOR_INTERNAL_H_
#define incl_HPHP_MC_GENERATOR_INTERNAL_H_

#include <boost/filesystem.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

// Common code emission patterns.

static_assert(sizeof(DataType) == 1,
              "Your DataType has an unsupported size.");

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1) {
  v << testbi{s0, s1, sf};
}

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1) {
  v << testbim{s0, s1, sf};
}

inline void emitLoadTVType(X64Assembler& a, MemoryRef src, Reg8 d) {
  a.  loadb(src, d);
}

inline void emitLoadTVType(X64Assembler& a, MemoryRef src, Reg32 d) {
  a.  loadzbl(src, d);
}

inline void emitLoadTVType(Vout& v, Vptr mem, Vreg8 d) {
  v << loadb{mem, d};
}

inline void emitCmpTVType(X64Assembler& a, DataType dt, Reg8 typeReg) {
  a.  cmpb(dt, typeReg);
}

inline void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vptr s1) {
  v << cmpbim{s0, s1, sf};
}

inline void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vreg s1) {
  v << cmpbi{s0, s1, sf};
}

///////////////////////////////////////////////////////////////////////////////
}}

#endif
