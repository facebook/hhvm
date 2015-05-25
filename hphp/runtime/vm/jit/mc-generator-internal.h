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

// Helper structs for jcc vs. jcc8.
struct Jcc8 {
  static void branch(X64Assembler& a, ConditionCode cc, TCA dest) {
    a.   jcc8(cc, dest);
  }
  static void patch(X64Assembler& a, TCA site, TCA newDest) {
    a.patchJcc8(site, newDest);
  }
};

struct Jcc32 {
  static void branch(X64Assembler& a, ConditionCode cc, TCA dest) {
    a.   jcc(cc, dest);
  }
  static void patch(X64Assembler& a, TCA site, TCA newDest) {
    a.patchJcc(site, newDest);
  }
};

// A CondBlock is an RAII structure for emitting conditional code. It
// compares the source register at fieldOffset with fieldValue, and
// conditionally branches over the enclosing block of assembly on the
// passed-in condition-code.
//
//  E.g.:
//    {
//      RefCountedOnly ifRefCounted(a, rdi, 0);
//      emitIncRef(rdi);
//    }
//
// will only execute emitIncRef if we find at runtime that rdi points at
// a ref-counted cell.
//
// It's ok to do reconcilable register operations in the body.
template<unsigned FieldOffset, int32_t FieldValue, ConditionCode Jcc,
         typename FieldType>
struct CondBlock {
  X64Assembler& m_a;
  int m_off;
  TCA m_jcc8;

  CondBlock(X64Assembler& a, PhysReg reg, int offset = 0)
      : m_a(a)
      , m_off(offset) {
    int typeDisp = m_off + FieldOffset;
    static_assert(sizeof(FieldType) == 1 || sizeof(FieldType) == 4,
                  "CondBlock of unimplemented field size");
    if (sizeof(FieldType) == 4) {
      a. cmpl(FieldValue, reg[typeDisp]);
    } else if (sizeof(FieldType) == 1) {
      a. cmpb(FieldValue, reg[typeDisp]);
    }
    m_jcc8 = a.frontier();
    a.   jcc8(Jcc, m_jcc8);
    // ...
  }

  ~CondBlock() {
    m_a.patchJcc8(m_jcc8, m_a.frontier());
  }
};

// IfRefCounted --
//   Emits if (IS_REFCOUNTED_TYPE()) { ... }
typedef CondBlock <TVOFF(m_type),
                   KindOfRefCountThreshold,
                   CC_LE,
                   DataType> IfRefCounted;

typedef CondBlock <TVOFF(m_type),
                   KindOfRef,
                   CC_NZ,
                   DataType> IfVariant;

typedef CondBlock <TVOFF(m_type),
                   KindOfUninit,
                   CC_Z,
                   DataType> UnlessUninit;

/*
 * locToRegDisp --
 *
 * Helper code for stack frames. The struct is a "template" in the non-C++
 * sense: we don't build source-level stack frames in C++ for the most part,
 * but its offsets tell us where to find fields in assembly.
 *
 * If we were physically pushing stack frames, we would push them in reverse
 * order to what you see here.
 */
inline void
locToRegDisp(int32_t localIndex, PhysReg* outbase, int* outdisp) {
  *outdisp = cellsToBytes(-(localIndex + 1));
  *outbase = x64::rVmFp;
}

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

template<typename OpndType>
static inline void verifyTVOff(const OpndType& op) { /* nop */ }
static inline void verifyTVOff(MemoryRef mr) {
  DEBUG_ONLY auto disp = mr.r.disp;
  // Make sure that we're operating on the m_type field of a
  // TypedValue*.
  assertx((disp & (sizeof(TypedValue) - 1)) == TVOFF(m_type));
}

template<typename SrcType, typename OpndType>
void emitTestTVType(X64Assembler& a, SrcType src, OpndType tvOp) {
  a.  testb(src, toByte(tvOp));
}

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

template<typename DestType, typename OpndType>
void emitStoreTVType(X64Assembler& a, OpndType tvOp, DestType dest) {
  a.  storeb(toByte(tvOp), dest);
}

inline void emitStoreTVType(Vout& v, Vreg src, Vptr dest) {
  v << storeb{src, dest};
}

inline void
emitStoreTVType(Vout& v, DataType src, Vptr dest) {
  v << storebi{src, dest};
}

// emitDeref --
// emitStoreTypedValue --
// emitStoreUninitNull --
//
//   Helpers for common cell operations.
//
//   Dereference the var in the cell whose address lives in src into
//   dest.
static inline void
emitDeref(X64Assembler &a, PhysReg src, PhysReg dest) {
  // src is a RefData, dest will be m_data field of inner gizmoom.
  a.    loadq (src[TVOFF(m_data)], dest);
}

// NB: leaves count field unmodified. Does not store to m_data if type
// is a null type.
static inline void
emitStoreTypedValue(X64Assembler& a, DataType type, PhysReg val,
                    int disp, PhysReg dest, bool writeType = true) {
  if (writeType) {
    emitStoreTVType(a, type, dest[disp + TVOFF(m_type)]);
  }
  if (!IS_NULL_TYPE(type)) {
    assertx(val != InvalidReg);
    a.  storeq(val, dest[disp + TVOFF(m_data)]);
  }
}

static inline void
emitStoreUninitNull(X64Assembler& a,
                    int disp,
                    PhysReg dest) {
  // OK to leave garbage in m_data, m_aux.
  emitStoreTVType(a, KindOfUninit, dest[disp + TVOFF(m_type)]);
}

static inline void
emitCopyTo(X64Assembler& a,
           Reg64 src,
           int srcOff,
           Reg64 dest,
           int destOff,
           PhysReg scratch) {
  assertx(src != scratch);
  // This is roughly how gcc compiles this.  Blow off m_aux.
  a.    loadq  (src[srcOff + TVOFF(m_data)], scratch);
  a.    storeq (scratch, dest[destOff + TVOFF(m_data)]);
  emitLoadTVType(a, src[srcOff + TVOFF(m_type)], r32(scratch));
  emitStoreTVType(a, r32(scratch), dest[destOff + TVOFF(m_type)]);
}

///////////////////////////////////////////////////////////////////////////////
}}

#endif
