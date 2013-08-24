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
#ifndef incl_HPHP_TRANSLATOR_X64_INTERNAL_H_
#define incl_HPHP_TRANSLATOR_X64_INTERNAL_H_

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
namespace Transl {

TRACE_SET_MOD(tx64);
static const DataType BitwiseKindOfString = KindOfString;

// Generate an if-then block into a.  thenBlock is executed if cc is true.
template <class Then>
void ifThen(Transl::X64Assembler& a, ConditionCode cc, Then thenBlock) {
  Label done;
  a.jcc8(ccNegate(cc), done);
  thenBlock();
  asm_label(a, done);
}

// RAII aids to machine code.

// UnlikelyIfBlock:
//
//  Branch to distant code (that we presumably don't expect to
//  take). This helps keep hot paths compact.
//
//  A common pattern using this involves patching the jump in astubs
//  to jump past the normal control flow in a (as in the following
//  example).  Do this using DiamondReturn so the register allocator
//  state will be properly maintained.  (Spills/fills to keep the
//  states in sync will be emitted on the unlikely path.)
//
// Example:
//
//  {
//    PhysReg inputParam = i.getReg(i.inputs[0]->location);
//    a.   test_reg_reg(inputParam, inputParam);
//    DiamondReturn retFromStubs;
//    {
//      UnlikelyIfBlock ifNotRax(CC_Z, a, astubs, &retFromStubs);
//      EMIT_CALL(a, TCA(launch_nuclear_missiles));
//    }
//    // The inputParam was non-zero, here is the likely branch:
//    m_regMap.allocOutputRegs(i);
//    emitMovRegReg(inputParam, m_regMap.getReg(i.outLocal->location));
//    // ~DiamondReturn patches the jump, and reconciles the branch
//    // with the main line.  (In this case it will fill the outLocal
//    // register since the main line thinks it is dirty.)
//  }
//  // The two cases are joined here.  We can do logic that was
//  // independent of whether the branch was taken, if necessary.
//  emitMovRegReg(i.outLocal, m_regMap.getReg(i.outStack->location));
//
// Note: it is ok to nest UnlikelyIfBlocks, as long as their
// corresponding DiamondReturns are correctly destroyed in reverse
// order.  But also note that this can lead to more jumps on the
// unlikely branch (see ~DiamondReturn).
struct UnlikelyIfBlock {
  X64Assembler& m_likely;
  X64Assembler& m_unlikely;
  TCA m_likelyPostBranch;

  explicit UnlikelyIfBlock(ConditionCode cc,
                           X64Assembler& likely,
                           X64Assembler& unlikely)
    : m_likely(likely)
    , m_unlikely(unlikely)
  {
    m_likely.jcc(cc, m_unlikely.frontier());
    m_likelyPostBranch = m_likely.frontier();
  }

  ~UnlikelyIfBlock() {
    m_unlikely.jmp(m_likelyPostBranch);
  }
};

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

// JccBlock --
//   A raw condition-code block; assumes whatever comparison or ALU op
//   that sets the Jcc has already executed.
template <ConditionCode Jcc, typename J=Jcc8>
struct JccBlock {
  mutable X64Assembler* m_a;
  TCA m_jcc;

  explicit JccBlock(X64Assembler& a)
    : m_a(&a) {
    m_jcc = a.frontier();
    J::branch(a, Jcc, m_a->frontier());
  }

  ~JccBlock() {
    if (m_a) {
      J::patch(*m_a, m_jcc, m_a->frontier());
    }
  }

private:
  JccBlock(const JccBlock&);
  JccBlock& operator=(const JccBlock&);
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
template<unsigned FieldOffset, unsigned FieldValue, ConditionCode Jcc,
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
 * Helper code for stack frames. The struct is a "template" in the
 * non-C++ sense: we don't build source-level stack frames in C++
 * for the most part, but its offsets tell us where to find fields
 * in assembly.
 *
 * If we were physically pushing stack frames, we would push them
 * in reverse order to what you see here.
 */
static inline void
locToRegDisp(const Location& l, PhysReg *outbase, int *outdisp,
             const Func* f = nullptr) {
  assert_not_implemented((l.space == Location::Stack ||
                          l.space == Location::Local ||
                          l.space == Location::Iter));
  *outdisp = cellsToBytes(locPhysicalOffset(l, f));
  *outbase = l.space == Location::Stack ? rVmSp : rVmFp;
}

// Common code emission patterns.

static_assert(sizeof(DataType) == 4 || sizeof(DataType) == 1,
              "Your DataType has an unsupported size.");
static inline Reg8 toByte(const Reg32& x)   { return rbyte(x); }
static inline Reg8 toByte(const Reg64& x)   { return rbyte(x); }
static inline Reg8 toByte(PhysReg x)        { return rbyte(x); }

static inline Reg32 toReg32(const Reg64& x) { return r32(x); }
static inline Reg32 toReg32(const Reg8& x)  { return r32(x); }
static inline Reg32 toReg32(PhysReg x)      { return r32(x); }

// For other operand types, let whatever conversions (or compile
// errors) exist handle it.
template<typename OpndType>
static OpndType toByte(const OpndType& x) { return x; }
template<typename OpndType>
static OpndType toReg32(const OpndType& x) { return x; }

template<typename OpndType>
static inline void verifyTVOff(const OpndType& op) { /* nop */ }
static inline void verifyTVOff(const MemoryRef& mr) {
  DEBUG_ONLY auto disp = mr.r.disp;
  // Make sure that we're operating on the m_type field of a
  // TypedValue*.
  assert((disp & (sizeof(TypedValue) - 1)) == TVOFF(m_type));
}

template<typename SrcType, typename OpndType>
static inline void
emitTestTVType(X64Assembler& a, SrcType src, OpndType tvOp) {
  verifyTVOff(src);
  if (sizeof(DataType) == 4) {
    a.  testl(src, toReg32(tvOp));
  } else {
    a.  testb(src, toByte(tvOp));
  }
}

template<typename SrcType, typename OpndType>
static inline void
emitLoadTVType(X64Assembler& a, SrcType src, OpndType tvOp) {
  verifyTVOff(src);
  if (sizeof(DataType) == 4) {
    a.  loadl(src, toReg32(tvOp));
  } else {
    // Zero extend the type, just in case.
    a.  loadzbl(src, toReg32(tvOp));
  }
}

template<typename SrcType, typename OpndType>
static inline void
emitCmpTVType(X64Assembler& a, SrcType src, OpndType tvOp) {
  verifyTVOff(src);
  if (sizeof(DataType) == 4) {
    a.  cmpl(src, toReg32(tvOp));
  } else {
    a.  cmpb(src, toByte(tvOp));
  }
}

template<typename DestType, typename OpndType>
static inline void
emitStoreTVType(X64Assembler& a, OpndType tvOp, DestType dest) {
  verifyTVOff(dest);
  if (sizeof(DataType) == 4) {
    a.  storel(toReg32(tvOp), dest);
  } else {
    a.  storeb(toByte(tvOp), dest);
  }
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

static inline void
emitDerefIfVariant(X64Assembler &a, PhysReg reg) {
  emitCmpTVType(a, KindOfRef, reg[TVOFF(m_type)]);
  if (RefData::tvOffset() == 0) {
    a.    cload_reg64_disp_reg64(CC_E, reg, TVOFF(m_data), reg);
  } else {
    ifThen(a, CC_E, [&] {
      a.  loadq(reg[TVOFF(m_data)], reg);
      a.  addq(RefData::tvOffset(), reg);
    });
  }
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
    assert(val != reg::noreg);
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
  assert(src != scratch);
  // This is roughly how gcc compiles this.  Blow off m_aux.
  auto s64 = r64(scratch);
  auto s32 = r32(scratch);
  a.    loadq  (src[srcOff + TVOFF(m_data)], s64);
  a.    storeq (s64, dest[destOff + TVOFF(m_data)]);
  emitLoadTVType(a, src[srcOff + TVOFF(m_type)], s32);
  emitStoreTVType(a, s32, dest[destOff + TVOFF(m_type)]);
}

// Pops the return address pushed by fcall and stores it into the
// actrec in rStashedAR.
inline void emitPopRetIntoActRec(X64Assembler& a) {
  a.    pop  (rStashedAR[AROFF(m_savedRip)]);
}

}}

#endif

