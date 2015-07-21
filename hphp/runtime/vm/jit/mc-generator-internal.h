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
static inline Reg8 toByte(const Reg32& x)   { return rbyte(x); }
static inline Reg8 toByte(const Reg64& x)   { return rbyte(x); }
static inline Reg8 toByte(PhysReg x)        { return rbyte(Reg64(x)); }

static inline Reg32 toReg32(const Reg64& x) { return r32(x); }
static inline Reg32 toReg32(const Reg8& x)  { return r32(x); }
static inline Reg32 toReg32(PhysReg x)      { return r32(Reg64(x)); }

// For other operand types, let whatever conversions (or compile
// errors) exist handle it.
template<typename OpndType>
static OpndType toByte(const OpndType& x) { return x; }
template<typename OpndType>
static OpndType toReg32(const OpndType& x) { return x; }

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1) {
  v << testbi{s0, s1, sf};
}

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1) {
  v << testbim{s0, s1, sf};
}

template<typename SrcType, typename OpndType>
static inline void
emitLoadTVType(X64Assembler& a, SrcType src, OpndType tvOp) {
  // Zero extend the type, just in case.
  a.  loadzbl(src, toReg32(tvOp));
}

inline void emitLoadTVType(Vout& v, Vptr mem, Vreg d) {
  v << loadzbq{mem, d};
}

template<typename SrcType, typename OpndType>
void emitCmpTVType(X64Assembler& a, SrcType src, OpndType tvOp) {
  a.  cmpb(src, toByte(tvOp));
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
