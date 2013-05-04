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
#include <stdint.h>
#include <strings.h>

#include "folly/Format.h"
#include "folly/Conv.h"
#include "util/trace.h"
#include "util/stack_trace.h"
#include "util/util.h"

#include "runtime/vm/bytecode.h"
#include "runtime/vm/runtime.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/runtime_option.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-deps.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/base/stats.h"

#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/opt.h"
#include "runtime/vm/translator/hopt/linearscan.h"
#include "runtime/vm/translator/hopt/codegen.h"
#include "runtime/vm/translator/hopt/hhbctranslator.h"
#include "runtime/vm/translator/hopt/print.h"

// Include last to localize effects to this file
#include "util/assert_throw.h"

namespace HPHP {
namespace VM {
namespace Transl {

using namespace reg;
using namespace Util;
using namespace Trace;
using std::max;

static const Trace::Module TRACEMOD = Trace::hhir;
#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

#define TVOFF(nm) offsetof(TypedValue, nm)
#define AROFF(nm) offsetof(ActRec, nm)

/*
 * tx64LocPhysicalOffset --
 *
 *   The translator uses the stack pointer slightly differently from
 *   VM::Stack. Consequently, the translated code accesses slightly
 *   different offsets from rVmSp than the C++ runtime.
 */
static inline int
tx64LocPhysicalOffset(const Location& l, const Func *f = nullptr) {
  return Translator::locPhysicalOffset(l, f);
}


#define HHIR_UNIMPLEMENTED_OP(op)                       \
  do {                                                  \
    throw JIT::FailedIRGen(__FILE__, __LINE__, op);     \
  } while (0)


#define HHIR_UNIMPLEMENTED(op)                          \
  do {                                                  \
    throw JIT::FailedIRGen(__FILE__, __LINE__, #op);    \
  } while (0)

#define HHIR_UNIMPLEMENTED_WHEN(expr, op)               \
  do {                                                  \
    if (expr) {                                         \
      throw JIT::FailedIRGen(__FILE__, __LINE__, #op);  \
    }                                                   \
  } while (0)

#define HHIR_EMIT(op, ...)                      \
  do {                                          \
    m_hhbcTrans->emit ## op(__VA_ARGS__);       \
    return;                                     \
  } while (0)

bool isInferredType(const NormalizedInstruction& i) {
  return (i.getOutputUsage(i.outStack) ==
          NormalizedInstruction::OutputInferred);
}

JIT::Type getInferredOrPredictedType(const NormalizedInstruction& i) {
  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);
  if (u == NormalizedInstruction::OutputInferred ||
      (u == NormalizedInstruction::OutputUsed && i.outputPredicted)) {
    return JIT::Type::fromRuntimeType(i.outStack->rtt);
  }
  return JIT::Type::None;
}

void
TranslatorX64::irCheckType(X64Assembler& a,
                           const Location& l,
                           const RuntimeType& rtt,
                           SrcRec& fail) {
  assert(m_useHHIR);
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue()) return;

  if (l.space == Location::Stack) {
    // tx64LocPhysicalOffset returns:
    // negative offsets for locals accessed via rVmFp
    // positive offsets for stack values, relative to rVmSp
    uint32_t stackOffset = tx64LocPhysicalOffset(l);
    m_hhbcTrans->guardTypeStack(stackOffset, JIT::Type::fromRuntimeType(rtt));
  } else if (l.space == Location::Local){
    // Convert negative offset to a positive offset for convenience
    m_hhbcTrans->guardTypeLocal(l.offset, JIT::Type::fromRuntimeType(rtt));
  } else if (l.space == Location::Iter) {
    assert(false); // should not happen
  } else {
    HHIR_UNIMPLEMENTED(Invalid_space);
  }

  return;
}

void
TranslatorX64::irEmitLoadDeps() {
  assert(m_useHHIR);
  m_hhbcTrans->emitLoadDeps();
}


void
TranslatorX64::irTranslateMod(const Tracelet& t,
                              const NormalizedInstruction& i) {
  HHIR_EMIT(Mod);
}

void
TranslatorX64::irTranslateBinaryArithOp(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  const Opcode op = i.op();
  switch (op) {
#define CASE(OpBc)                                          \
    case Op ## OpBc:   HHIR_EMIT(OpBc);
    CASE(Add)
    CASE(Sub)
    CASE(BitAnd)
    CASE(BitOr)
    CASE(BitXor)
    CASE(Mul)
#undef CASE
    default: {
      not_reached();
    };
  }
  NOT_REACHED();
}

void
TranslatorX64::irTranslateSameOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpSame || op == OpNSame);
  if (op == OpSame) {
    HHIR_EMIT(Same);
  } else {
    HHIR_EMIT(NSame);
  }
}

void
TranslatorX64::irTranslateEqOp(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpEq || op == OpNeq);
  if (op == OpEq) {
    HHIR_EMIT(Eq);
  } else {
    HHIR_EMIT(Neq);
  }
}

void
TranslatorX64::irTranslateLtGtOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpLt || op == OpLte || op == OpGt || op == OpGte);
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);

  HHIR_UNIMPLEMENTED_WHEN((!i.isNative()), LtGtOp);
  switch (op) {
    case OpLt  : HHIR_EMIT(Lt);
    case OpLte : HHIR_EMIT(Lte);
    case OpGt  : HHIR_EMIT(Gt);
    case OpGte : HHIR_EMIT(Gte);
    default    : HHIR_UNIMPLEMENTED(LtGtOp);
  }
}

void
TranslatorX64::irTranslateUnaryBooleanOp(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpCastBool || op == OpEmptyL);
  if (op == OpCastBool) {
    HHIR_EMIT(CastBool);
  } else {
    HHIR_EMIT(EmptyL, i.inputs[0]->location.offset);
  }
}

void
TranslatorX64::irTranslateBranchOp(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpJmpZ || op == OpJmpNZ);
  if (op == OpJmpZ) {
    HHIR_EMIT(JmpZ,  i.offset() + i.imm[0].u_BA);
  } else {
    HHIR_EMIT(JmpNZ, i.offset() + i.imm[0].u_BA);
  }
}

void
TranslatorX64::irTranslateCGetL(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const DEBUG_ONLY Opcode op = i.op();
  assert(op == OpFPassL || OpCGetL);
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(inputs[0]->isLocal());

  HHIR_EMIT(CGetL, inputs[0]->location.offset);
}

void
TranslatorX64::irTranslateCGetL2(const Tracelet& t,
                                 const NormalizedInstruction& ni) {
  const int locIdx   = 1;

  HHIR_EMIT(CGetL2, ni.inputs[locIdx]->location.offset);
}

void
TranslatorX64::irTranslateVGetL(const Tracelet& t,
                                const NormalizedInstruction& i) {
  HHIR_EMIT(VGetL, i.inputs[0]->location.offset);
}

void
TranslatorX64::irTranslateAssignToLocalOp(const Tracelet& t,
                                          const NormalizedInstruction& ni) {
  DEBUG_ONLY const int rhsIdx  = 0;
  const int locIdx  = 1;
  const Opcode op = ni.op();
  assert(op == OpSetL || op == OpBindL);
  assert(ni.inputs.size() == 2);
  assert((op == OpBindL) ==
         (ni.inputs[rhsIdx]->outerType() == KindOfRef));

  assert(!ni.outStack || ni.inputs[locIdx]->location != ni.outStack->location);
  assert(ni.outLocal);
  assert(ni.inputs[locIdx]->location == ni.outLocal->location);
  assert(ni.inputs[rhsIdx]->isStack());

  if (op == OpSetL) {
    assert(ni.inputs[locIdx]->isLocal());
    HHIR_EMIT(SetL, ni.inputs[locIdx]->location.offset);
  } else {
    assert(op == OpBindL);
    HHIR_EMIT(BindL, ni.inputs[locIdx]->location.offset);
  }
}

void
TranslatorX64::irTranslatePopC(const Tracelet& t,
                             const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(!i.outStack && !i.outLocal);

  if (i.inputs[0]->rtt.isVagueValue()) {
    HHIR_EMIT(PopR);
  } else {
    HHIR_EMIT(PopC);
  }
}

void
TranslatorX64::irTranslatePopV(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs[0]->rtt.isVagueValue() || i.inputs[0]->isRef());
  HHIR_EMIT(PopV);
}

void
TranslatorX64::irTranslatePopR(const Tracelet& t,
                               const NormalizedInstruction& i) {
  irTranslatePopC(t, i);
}

void
TranslatorX64::irTranslateUnboxR(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  if (i.noOp) {
    // statically proved to be unboxed -- just pass that info to the IR
    TRACE(1, "HHIR: irTranslateUnboxR: output inferred to be Cell\n");
    m_hhbcTrans->assertTypeStack(0, JIT::Type::Cell);
  } else {
    HHIR_EMIT(UnboxR);
  }
}

void
TranslatorX64::irTranslateNull(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(Null);
}

void
TranslatorX64::irTranslateNullUninit(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_EMIT(NullUninit);
}

void
TranslatorX64::irTranslateTrue(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(True);
}

void
TranslatorX64::irTranslateFalse(const Tracelet& t,
                                const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(False);
}

void
TranslatorX64::irTranslateInt(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);
  assert(!i.outLocal);

  HHIR_EMIT(Int, i.imm[0].u_I64A);
}

void
TranslatorX64::irTranslateDouble(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_EMIT(Double, i.imm[0].u_DA);
}

void
TranslatorX64::irTranslateString(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);
  assert(!i.outLocal);

  HHIR_EMIT(String, (i.imm[0].u_SA));
}

void
TranslatorX64::irTranslateArray(const Tracelet& t,
                                const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(Array, i.imm[0].u_AA);
}

void
TranslatorX64::irTranslateNewArray(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  HHIR_EMIT(NewArray, i.imm[0].u_IVA);
}

void
TranslatorX64::irTranslateNop(const Tracelet& t,
                              const NormalizedInstruction& i) {
  HHIR_EMIT(Nop);
}

void
TranslatorX64::irTranslateAddElemC(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  HHIR_EMIT(AddElemC);
}

void
TranslatorX64::irTranslateAddNewElemC(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);
  assert(i.inputs[0]->isStack());
  assert(i.inputs[1]->isStack());

  HHIR_EMIT(AddNewElemC);
}

void
TranslatorX64::irTranslateCns(const Tracelet& t,
                              const NormalizedInstruction& i) {
  HHIR_EMIT(Cns, i.imm[0].u_SA);
}

void
TranslatorX64::irTranslateDefCns(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_EMIT(DefCns, (i.imm[0].u_SA));
}

void
TranslatorX64::irTranslateClsCnsD(const Tracelet& t,
                                const NormalizedInstruction& i) {
  HHIR_EMIT(ClsCnsD, (i.imm[0].u_SA), (i.imm[1].u_SA));
}

void
TranslatorX64::irTranslateConcat(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_EMIT(Concat);
}

void
TranslatorX64::irTranslateAdd(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);

  if (planInstrAdd_Array(i)) {
    HHIR_EMIT(ArrayAdd);
    return;
  }
  HHIR_EMIT(Add);
}

void
TranslatorX64::irTranslateXor(const Tracelet& t,
                              const NormalizedInstruction& i) {
  HHIR_EMIT(Xor);
}

void
TranslatorX64::irTranslateNot(const Tracelet& t,
                              const NormalizedInstruction& i) {
  HHIR_EMIT(Not);
}

void
TranslatorX64::irTranslateBitNot(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);

  HHIR_EMIT(BitNot);
}

void
TranslatorX64::irTranslateCastInt(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack && !i.outLocal);

  HHIR_EMIT(CastInt);
  /* nop */
}

void
TranslatorX64::irTranslateCastArray(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(CastArray);
}

void
TranslatorX64::irTranslateCastObject(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(CastObject);
}

void
TranslatorX64::irTranslateCastDouble(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  HHIR_EMIT(CastDouble);
}

void
TranslatorX64::irTranslateCastString(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  HHIR_EMIT(CastString);
}

void
TranslatorX64::irTranslatePrint(const Tracelet& t,
                                const NormalizedInstruction& i) {
  HHIR_EMIT(Print);
}

void
TranslatorX64::irTranslateJmp(const Tracelet& t,
                              const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, i.noSurprise);
}

void
TranslatorX64::irTranslateSwitch(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_EMIT(Switch, i.immVec, i.imm[1].u_I64A, i.imm[2].u_IVA);
}

void
TranslatorX64::irTranslateSSwitch(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  HHIR_EMIT(SSwitch, i.immVec);
}

/*
 * translateRetC --
 *
 *   Return to caller with the current activation record replaced with the
 *   top-of-stack return value.
 */
void
TranslatorX64::irTranslateRetC(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_EMIT(RetC, i.inlineReturn);
}

void
TranslatorX64::irTranslateRetV(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_EMIT(RetV, i.inlineReturn);
}

void
TranslatorX64::irTranslateNativeImpl(const Tracelet& t,
                                     const NormalizedInstruction& ni) {
  HHIR_EMIT(NativeImpl);
}

// emitClsLocalIndex --
// emitStringToClass --
// emitStringToKnownClass --
// emitObjToClass --
// emitClsAndPals --
//   Helpers for AGetC/AGetL.

const int kEmitClsLocalIdx = 0;

void TranslatorX64::irTranslateAGetC(const Tracelet& t,
                                     const NormalizedInstruction& ni) {
  const StringData* clsName =
    ni.inputs[kEmitClsLocalIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(AGetC, clsName);
}

void TranslatorX64::irTranslateAGetL(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  assert(i.inputs[kEmitClsLocalIdx]->isLocal());
  const DynLocation* dynLoc = i.inputs[kEmitClsLocalIdx];
  const StringData* clsName = dynLoc->rtt.valueStringOrNull();
  HHIR_EMIT(AGetL, dynLoc->location.offset, clsName);
}

void TranslatorX64::irTranslateSelf(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(Self);
}

void TranslatorX64::irTranslateParent(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  HHIR_EMIT(Parent);
}

void TranslatorX64::irTranslateDup(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  HHIR_EMIT(Dup);
}

void TranslatorX64::irTranslateCreateCont(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCont, i.imm[0].u_IVA, i.imm[1].u_SA);
}

void TranslatorX64::irTranslateContEnter(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  int after = nextSrcKey(t, i).offset();

  // ContEnter can't exist in an inlined function right now.  (If it
  // ever can, this curFunc() needs to change.)
  assert(!m_hhbcTrans->isInlining());
  const Func* srcFunc = curFunc();
  int32_t callOffsetInUnit = after - srcFunc->base();

  HHIR_EMIT(ContEnter, callOffsetInUnit);
}

void TranslatorX64::irTranslateContExit(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(ContExit);
}

void TranslatorX64::irTranslateUnpackCont(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  HHIR_EMIT(UnpackCont);
}

void TranslatorX64::irTranslatePackCont(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(PackCont, i.imm[0].u_IVA);
}

void TranslatorX64::irTranslateContReceive(const Tracelet& t,
                                           const NormalizedInstruction& i) {
  HHIR_EMIT(ContReceive);
}

void TranslatorX64::irTranslateContRetC(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(ContRetC);
}

void TranslatorX64::irTranslateContNext(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(ContNext);
}

void TranslatorX64::irTranslateContSend(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(ContSend);
}

void TranslatorX64::irTranslateContRaise(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  HHIR_EMIT(ContRaise);
}

void TranslatorX64::irTranslateContValid(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  HHIR_EMIT(ContValid);
}

void TranslatorX64::irTranslateContCurrent(const Tracelet& t,
                                           const NormalizedInstruction& i) {
  HHIR_EMIT(ContCurrent);
}

void TranslatorX64::irTranslateContStopped(const Tracelet& t,
                                           const NormalizedInstruction& i) {
  HHIR_EMIT(ContStopped);
}

void TranslatorX64::irTranslateContHandle(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  HHIR_EMIT(ContHandle);
}

void TranslatorX64::irTranslateStrlen(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  HHIR_EMIT(Strlen);
}

void TranslatorX64::irTranslateIncStat(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  HHIR_EMIT(IncStat, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void TranslatorX64::irTranslateClassExists(const Tracelet& t,
                                           const NormalizedInstruction& i) {
  const StringData* clsName = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(ClassExists, clsName);
}

void TranslatorX64::irTranslateInterfaceExists(const Tracelet& t,
                                               const NormalizedInstruction& i) {
  const StringData* ifaceName = i.inputs[1]->rtt.valueStringOrNull();

  HHIR_EMIT(InterfaceExists, ifaceName);
}

void TranslatorX64::irTranslateTraitExists(const Tracelet& t,
                                           const NormalizedInstruction& i) {
  const StringData* traitName = i.inputs[1]->rtt.valueStringOrNull();

  HHIR_EMIT(TraitExists, traitName);
}

void TranslatorX64::irTranslateVGetS(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetS, propName);
}

void
TranslatorX64::irTranslateVGetG(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetG, name);
}

void TranslatorX64::irTranslateBindS(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  const int kPropIdx = 2;
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(BindS, propName);
}

void TranslatorX64::irTranslateEmptyS(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(EmptyS, propName);
}

void TranslatorX64::irTranslateEmptyG(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(EmptyG, gblName);
}

void
TranslatorX64::irTranslateIssetS(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(IssetS, propName);
}

void
TranslatorX64::irTranslateIssetG(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(IssetG, gblName);
}

void
TranslatorX64::irTranslateUnsetG(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(UnsetG, gblName);
}

void TranslatorX64::irTranslateCGetS(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetS, propName,
            getInferredOrPredictedType(i), isInferredType(i));
}

void TranslatorX64::irTranslateSetS(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  const int kPropIdx = 2;
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(SetS, propName);
}

void
TranslatorX64::irTranslateCGetG(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetG, name, getInferredOrPredictedType(i), isInferredType(i));
}

void TranslatorX64::irTranslateSetG(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  const StringData* name = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(SetG, name);
}

void TranslatorX64::irTranslateBindG(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  const StringData* name = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(BindG, name);
}

void
TranslatorX64::irTranslateLateBoundCls(const Tracelet&,
                                       const NormalizedInstruction&i) {
  HHIR_EMIT(LateBoundCls);
}


void TranslatorX64::irTranslateFPassL(const Tracelet& t,
                                      const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    irTranslateVGetL(t, ni);
  } else {
    irTranslateCGetL(t, ni);
  }
}

void TranslatorX64::irTranslateFPassS(const Tracelet& t,
                                      const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    irTranslateVGetS(t, ni);
  } else {
    irTranslateCGetS(t, ni);
  }
}

void TranslatorX64::irTranslateFPassG(const Tracelet& t,
                                      const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    irTranslateVGetG(t, ni);
  } else {
    irTranslateCGetG(t, ni);
  }
}

void
TranslatorX64::irTranslateCheckTypeOp(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);
  assert(ni.outStack);

  const Opcode op = ni.op();
  const int    off = ni.inputs[0]->location.offset;
  switch (op) {
    case OpIssetL:    HHIR_EMIT(IssetL, off);
    case OpIsNullL:   HHIR_EMIT(IsNullL, off);
    case OpIsNullC:   HHIR_EMIT(IsNullC);
    case OpIsStringL: HHIR_EMIT(IsStringL, off);
    case OpIsStringC: HHIR_EMIT(IsStringC);
    case OpIsArrayL:  HHIR_EMIT(IsArrayL, off);
    case OpIsArrayC:  HHIR_EMIT(IsArrayC);
    case OpIsIntL:    HHIR_EMIT(IsIntL, off);
    case OpIsIntC:    HHIR_EMIT(IsIntC);
    case OpIsBoolL:   HHIR_EMIT(IsBoolL, off);
    case OpIsBoolC:   HHIR_EMIT(IsBoolC);
    case OpIsDoubleL: HHIR_EMIT(IsDoubleL, off);
    case OpIsDoubleC: HHIR_EMIT(IsDoubleC);
    case OpIsObjectL: HHIR_EMIT(IsObjectL, off);
    case OpIsObjectC: HHIR_EMIT(IsObjectC);
    // Note: for IsObject*, we need to emit some kind of
    // call to ObjectData::isResource or something.
  }
  NOT_REACHED();
}

void
TranslatorX64::irTranslateAKExists(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  HHIR_EMIT(AKExists);
}

void
TranslatorX64::irTranslateSetOpL(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  JIT::Opcode opc;
  switch (i.imm[1].u_OA) {
    case SetOpPlusEqual:   opc = JIT::OpAdd;    break;
    case SetOpMinusEqual:  opc = JIT::OpSub;    break;
    case SetOpMulEqual:    opc = JIT::OpMul;    break;
    case SetOpDivEqual:    HHIR_UNIMPLEMENTED(SetOpL_Div);
    case SetOpConcatEqual: opc = JIT::Concat;   break;
    case SetOpModEqual:    HHIR_UNIMPLEMENTED(SetOpL_Mod);
    case SetOpAndEqual:    opc = JIT::OpAnd; break;
    case SetOpOrEqual:     opc = JIT::OpOr;  break;
    case SetOpXorEqual:    opc = JIT::OpXor; break;
    case SetOpSlEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shl);
    case SetOpSrEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shr);
    default: not_reached();
  }
  const int localIdx = 1;
  HHIR_EMIT(SetOpL, opc, i.inputs[localIdx]->location.offset);
}

void
TranslatorX64::irTranslateIncDecL(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(i.outLocal);
  assert(inputs[0]->isLocal());
  const IncDecOp oplet = IncDecOp(i.imm[1].u_OA);
  assert(oplet == PreInc || oplet == PostInc || oplet == PreDec ||
         oplet == PostDec);
  bool post = (oplet == PostInc || oplet == PostDec);
  bool pre  = !post;
  bool inc  = (oplet == PostInc || oplet == PreInc);

  HHIR_UNIMPLEMENTED_WHEN((i.inputs[0]->valueType() != KindOfBoolean) &&
                          (i.inputs[0]->valueType() != KindOfInt64) &&
                          (i.inputs[0]->valueType() != KindOfDouble),
                          IncDecL_unsupported);
  HHIR_EMIT(IncDecL, pre, inc, inputs[0]->location.offset);
}

void
TranslatorX64::irTranslateUnsetL(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetL, i.inputs[0]->location.offset);
}

void
TranslatorX64::irTranslateReqLit(const Tracelet& t,
                                 const NormalizedInstruction& i,
                                 InclOpFlags flags) {
  HHIR_UNIMPLEMENTED(ReqLit);
}

void
TranslatorX64::irTranslateReqDoc(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(ReqDoc, name);
}

void TranslatorX64::irTranslateDefCls(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  HHIR_EMIT(DefCls, cid, i.source.offset());
}

void TranslatorX64::irTranslateDefFunc(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  int fid = i.imm[0].u_IVA;
  HHIR_EMIT(DefFunc, fid);
}

void
TranslatorX64::irTranslateFPushFunc(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFunc, (i.imm[0].u_IVA));
}

void
TranslatorX64::irTranslateFPushClsMethodD(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  HHIR_EMIT(FPushClsMethodD,
             (i.imm[0].u_IVA),
             (i.imm[1].u_SA),
             (i.imm[2].u_SA));
}

void
TranslatorX64::irTranslateFPushClsMethodF(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  // For now, only support cases where both the class and the method are known.
  const int classIdx  = 0;
  const int methodIdx = 1;
  DynLocation* classLoc  = i.inputs[classIdx];
  DynLocation* methodLoc = i.inputs[methodIdx];

  HHIR_UNIMPLEMENTED_WHEN(!(methodLoc->isString() &&
                            methodLoc->rtt.valueString() != nullptr &&
                            classLoc->valueType() == KindOfClass &&
                            classLoc->rtt.valueClass() != nullptr),
                          FPushClsMethodF_unknown);

  auto cls = classLoc->rtt.valueClass();
  HHIR_EMIT(FPushClsMethodF,
            i.imm[0].u_IVA, // # of arguments
            cls,
            methodLoc->rtt.valueString());
}

void
TranslatorX64::irTranslateFPushObjMethodD(const Tracelet &t,
                                          const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  HHIR_UNIMPLEMENTED_WHEN((i.inputs[0]->valueType() != KindOfObject),
                          FPushObjMethod_nonObj);
  assert(i.inputs[0]->valueType() == KindOfObject);
  const Class* baseClass = i.inputs[0]->rtt.valueClass();

  HHIR_EMIT(FPushObjMethodD,
            i.imm[0].u_IVA,
            i.imm[1].u_IVA,
            baseClass);
}

void TranslatorX64::irTranslateFPushCtor(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCtor, (i.imm[0].u_IVA));
}

void TranslatorX64::irTranslateFPushCtorD(const Tracelet& t,
                                          const NormalizedInstruction& i) {

  HHIR_EMIT(FPushCtorD, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void TranslatorX64::irTranslateCreateCl(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCl, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

// static void fatalNullThis() { raise_error(Strings::FATAL_NULL_THIS); }

void
TranslatorX64::irTranslateThis(const Tracelet &t,
                               const NormalizedInstruction &i) {
  HHIR_EMIT(This);
}

void
TranslatorX64::irTranslateBareThis(const Tracelet &t,
                                  const NormalizedInstruction &i) {
  HHIR_EMIT(BareThis, (i.imm[0].u_OA));
}

void
TranslatorX64::irTranslateCheckThis(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(CheckThis);
}

void
TranslatorX64::irTranslateInitThisLoc(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(InitThisLoc, i.outLocal->location.offset);
}

void
TranslatorX64::irTranslateFPushFuncD(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFuncD, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void
TranslatorX64::irTranslateFPassCOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const Opcode op = i.op();
  if (i.preppedByRef && (op == OpFPassCW || op == OpFPassCE)) {
    // These cases might have to raise a warning or an error
    HHIR_UNIMPLEMENTED(FPassCW_FPassCE_byref);
  } else {
    HHIR_EMIT(FPassCOp);
  }
}

void
TranslatorX64::irTranslateFPassV(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  if (i.preppedByRef || i.noOp) {
    TRACE(1, "HHIR: irTranslateFPassV: noOp\n");
    return;
  }
  HHIR_EMIT(FPassV);
}

void
TranslatorX64::irTranslateFPushCufOp(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  Class* cls = nullptr;
  StringData* invName = nullptr;
  bool forward = false;
  const Func* func = findCuf(i, cls, invName, forward);
  HHIR_EMIT(FPushCufOp, i.op(), cls, invName, func, i.imm[0].u_IVA);
}

void
TranslatorX64::irTranslateFPassR(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  /*
   * Like FPassC, FPassR is able to cheat on boxing if the current
   * parameter is pass by reference but we have a cell: the box would refer
   * to exactly one datum (the value currently on the stack).
   *
   * However, if the callee wants a cell and we have a variant we must
   * unbox; otherwise we might accidentally make callee changes to its
   * parameter globally visible.
   */
  if (!i.preppedByRef) {
    HHIR_EMIT(FPassR);
  } else {
    HHIR_UNIMPLEMENTED(FPassR);
  }
}

void
TranslatorX64::irTranslateFCallBuiltin(const Tracelet& t,
                              const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  int numNonDefault  = i.imm[1].u_IVA;
  Id funcId = i.imm[2].u_SA;

  HHIR_EMIT(FCallBuiltin, numArgs, numNonDefault, funcId);
}

static bool shouldIRInline(const Func* curFunc,
                           const Func* func,
                           const Tracelet& callee) {
  if (!RuntimeOption::EvalHHIREnableGenTimeInlining) {
    return false;
  }

  auto refuse = [&](const char* why) -> bool {
    FTRACE(1, "shouldIRInline: refusing {} <reason: {}>\n",
              func->fullName()->data(), why);
    return false;
  };
  auto accept = [&](const char* kind) -> bool {
    FTRACE(1, "shouldIRInline: inlining {} <kind: {}>\n",
              func->fullName()->data(), kind);
    return true;
  };

  if (func->numLocals() != func->numParams()) {
    return refuse("locals");
  }
  if (func->numIterators() != 0) {
    return refuse("iterators");
  }
  if (func->maxStackCells() >= kStackCheckPadding) {
    FTRACE(1, "{} >= {}\n", func->maxStackCells(), kStackCheckPadding);
    return refuse("too many stack cells");
  }

  // Disable anything with locals---specialized RetC generates stores
  // that zero out the m_type's and depend on the frame.
  if (func->numLocals() != 0) {
    return refuse("has locals (would use frame)");
  }

  // Little pattern recognition helpers:
  const NormalizedInstruction* cursor;
  Opcode current;
  auto resetCursor = [&] {
    cursor = callee.m_instrStream.first;
    current = cursor->op();
  };
  auto next = [&]() -> Opcode {
    auto op = cursor->op();
    cursor = cursor->next;
    current = cursor->op();
    return op;
  };
  auto nextIf = [&](Opcode op) -> bool {
    if (current != op) return false;
    next();
    return true;
  };
  auto atRet = [&] { return current == OpRetC || current == OpRetV; };

  // Simple operations that just put a Cell on the stack without any
  // inputs.  For now avoid CreateCont because it depends on the
  // frame.
  auto simpleCell = [&]() -> bool {
    if (cursor->outStack && cursor->inputs.empty() &&
        current != OpCreateCont) {
      next();
      return true;
    }
    return false;
  };

  // Simple two-cell comparison operators.
  auto simpleCmp = [&]() -> bool {
    switch (current) {
    case OpAdd: case OpSub: case OpMul: case OpDiv: case OpMod:
    case OpXor: case OpNot: case OpSame: case OpNSame: case OpEq:
    case OpNeq: case OpLt: case OpLte: case OpGt: case OpGte:
    case OpBitAnd: case OpBitOr: case OpBitXor: case OpBitNot:
    case OpShl: case OpShr:
      next();
      return true;
    default:
      return false;
    }
  };

  // In the various patterns below, when we're down to a cell on the
  // stack, this is used to allow simple constant-foldable
  // manipulations of it before return.
  auto cellManipRet = [&]() -> bool {
    if (nextIf(OpNot)) return atRet();
    if (simpleCell() && simpleCmp()) return atRet();
    return atRet();
  };

  /////////////

  // Identity functions.
  resetCursor();
  if (current == OpCGetL) {
    next();
    if (atRet()) return accept("returns parameter");
  }

  // Simple property accessors.
  resetCursor();
  if (current == OpCheckThis) next();
  if (cursor->op() == OpCGetM &&
      cursor->immVec.locationCode() == LH &&
      cursor->immVecM.size() == 1 &&
      cursor->immVecM.front() == MPT &&
      !mInstrHasUnknownOffsets(*cursor, func->cls())) {
    next();
    // Can't currently support cellManipRet because it's usually going
    // to be CGetM-prediction, which will use the frame.
    if (atRet()) {
      return accept("simple property accessor");
    }
  }

  /*
   * Functions that set an object property to a simple cell value.
   * E.g. something that does $this->foo = null;
   */
  resetCursor();
  if (current == OpCheckThis) next();
  if (simpleCell()) {
    if (cursor->op() == OpSetM &&
        cursor->immVec.locationCode() == LH &&
        cursor->immVecM.size() == 1 &&
        cursor->immVecM.front() == MPT &&
        !mInstrHasUnknownOffsets(*cursor, func->cls())) {
      next();
      if (nextIf(OpPopC) && simpleCell() && atRet()) {
        return accept("simpleCell prop setter");
      }
    }
  }

  /*
   * Continuation allocation functions that take no arguments.
   */
  resetCursor();
  if (current == OpCreateCont && cursor->imm[0].u_IVA == 0) {
    next();
    if (atRet()) return accept("zero-arg continuation creator");
  }

  /*
   * Anything that just puts a value on the stack with no inputs, and
   * then returns it, after possibly doing some comparison with
   * another such thing.
   *
   * E.g. String; String; Same; RetC, or Null; RetC.
   */
  resetCursor();
  if (simpleCell() && cellManipRet()) {
    return accept("simple returner");
  }

  // BareThis; InstanceOfD; RetC
  resetCursor();
  if (nextIf(OpBareThis) && nextIf(OpInstanceOfD) && atRet()) {
    return accept("$this instanceof D");
  }

  return refuse("unknown kind of function");
}

void
TranslatorX64::irTranslateFCall(const Tracelet& t,
                                const NormalizedInstruction& i) {
  auto const numArgs = i.imm[0].u_IVA;

  always_assert(!m_hhbcTrans->isInlining() && "curUnit and curFunc calls");
  const Opcode* after = curUnit()->at(nextSrcKey(t, i).offset());
  const Func* srcFunc = curFunc();
  Offset returnBcOffset =
    srcFunc->unit()->offsetOf(after - srcFunc->base());

  /*
   * If we have a calleeTrace, we're going to see if we should inline
   * the call.
   */
  if (i.calleeTrace) {
    if (!m_hhbcTrans->isInlining() &&
        shouldIRInline(curFunc(), i.funcd, *i.calleeTrace)) {
      m_hhbcTrans->beginInlining(numArgs, i.funcd, returnBcOffset);
      static const bool shapeStats = Stats::enabledAny() &&
                                     getenv("HHVM_STATS_INLINESHAPE");
      if (shapeStats) {
        m_hhbcTrans->profileInlineFunctionShape(traceletShape(*i.calleeTrace));
      }

      for (auto* ni = i.calleeTrace->m_instrStream.first;
          ni; ni = ni->next) {
        m_curNI = ni;
        SCOPE_EXIT { m_curNI = &i; };
        irTranslateInstr(*i.calleeTrace, *ni);
      }
      return;
    }

    static const auto enabled = Stats::enabledAny() &&
                                getenv("HHVM_STATS_FAILEDINL");
    if (enabled) {
      m_hhbcTrans->profileFunctionEntry("FailedCandidate");
      m_hhbcTrans->profileFailedInlShape(traceletShape(*i.calleeTrace));
    }
  }

  HHIR_EMIT(FCall, numArgs, returnBcOffset, i.funcd);
}

void
TranslatorX64::irTranslateFCallArray(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  const Offset pcOffset = i.offset();
  SrcKey next = i.next ? i.next->source : t.m_nextSk;
  const Offset after = next.offset();

  HHIR_EMIT(FCallArray, pcOffset, after);
}

void
TranslatorX64::irTranslateNewTuple(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  HHIR_EMIT(NewTuple, numArgs);
}

void
TranslatorX64::irTranslateNewCol(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_EMIT(NewCol, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void
TranslatorX64::irTranslateColAddNewElemC(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  HHIR_EMIT(ColAddNewElemC);
}

void
TranslatorX64::irTranslateColAddElemC(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  HHIR_EMIT(ColAddElemC);
}

void
TranslatorX64::irTranslateStaticLocInit(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(StaticLocInit, i.imm[0].u_IVA, i.imm[1].u_SA);
}

// check class hierarchy and fail if no match
void
TranslatorX64::irTranslateVerifyParamType(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  int param = i.imm[0].u_IVA;
  HHIR_EMIT(VerifyParamType, param);
}

void
TranslatorX64::irTranslateInstanceOfD(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  HHIR_EMIT(InstanceOfD, (i.imm[0].u_SA));
}

void
TranslatorX64::irTranslateIterInit(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  HHIR_EMIT(IterInit,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
TranslatorX64::irTranslateIterInitK(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(IterInitK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
TranslatorX64::irTranslateIterNext(const Tracelet& t,
                                   const NormalizedInstruction& i) {

  HHIR_EMIT(IterNext,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
TranslatorX64::irTranslateIterNextK(const Tracelet& t,
                                    const NormalizedInstruction& i) {

  HHIR_EMIT(IterNextK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
TranslatorX64::irTranslateIterFree(const Tracelet& t,
                                   const NormalizedInstruction& i) {

  HHIR_EMIT(IterFree, i.imm[0].u_IVA);
}

// PSEUDOINSTR_DISPATCH is a switch() fragment that routes opcodes to their
// shared handlers, as per the PSEUDOINSTRS macro.
#define PSEUDOINSTR_DISPATCH(func)              \
  case OpBitAnd:                                \
  case OpBitOr:                                 \
  case OpBitXor:                                \
  case OpSub:                                   \
  case OpMul:                                   \
    func(BinaryArithOp, t, i)                   \
  case OpSame:                                  \
  case OpNSame:                                 \
    func(SameOp, t, i)                          \
  case OpEq:                                    \
  case OpNeq:                                   \
    func(EqOp, t, i)                            \
  case OpLt:                                    \
  case OpLte:                                   \
  case OpGt:                                    \
  case OpGte:                                   \
    func(LtGtOp, t, i)                          \
  case OpEmptyL:                                \
  case OpCastBool:                              \
    func(UnaryBooleanOp, t, i)                  \
  case OpJmpZ:                                  \
  case OpJmpNZ:                                 \
    func(BranchOp, t, i)                        \
  case OpSetL:                                  \
  case OpBindL:                                 \
    func(AssignToLocalOp, t, i)                 \
  case OpFPassC:                                \
  case OpFPassCW:                               \
  case OpFPassCE:                               \
    func(FPassCOp, t, i)                        \
  case OpFPushCuf:                              \
  case OpFPushCufF:                             \
  case OpFPushCufSafe:                          \
    func(FPushCufOp, t, i)                      \
  case OpIssetL:                                \
  case OpIsNullL:                               \
  case OpIsStringL:                             \
  case OpIsArrayL:                              \
  case OpIsIntL:                                \
  case OpIsObjectL:                             \
  case OpIsBoolL:                               \
  case OpIsDoubleL:                             \
  case OpIsNullC:                               \
  case OpIsStringC:                             \
  case OpIsArrayC:                              \
  case OpIsIntC:                                \
  case OpIsObjectC:                             \
  case OpIsBoolC:                               \
  case OpIsDoubleC:                             \
    func(CheckTypeOp, t, i)

// All vector instructions are handled by one HhbcTranslator method.
#define MII(instr, ...)                                                 \
  void TranslatorX64::irTranslate##instr##M(const Tracelet& t,          \
                                            const NormalizedInstruction& ni) { \
    m_hhbcTrans->emitMInstr(ni);                                        \
  }
MINSTRS
MII(FPass)
#undef MII

void
TranslatorX64::irTranslateInstrDefault(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  const char *opNames[] = {
#define O(name, imm, push, pop, flags) \
"Unimplemented" #name,
  OPCODES
#undef O
  };
  const Opcode op = i.op();

  // Add to this switch the bytecodes that the IR handles but the base
  // translator does not analyze and translate
  switch (op) {
    case OpLateBoundCls:
      irTranslateLateBoundCls(t, i);
      break;
    case OpEmptyS:
      irTranslateEmptyS(t, i);
      break;
    case OpEmptyG:
      irTranslateEmptyG(t, i);
      break;
    case OpVGetS:
      irTranslateVGetS(t, i);
      break;
    case OpIssetS:
      irTranslateIssetS(t, i);
      break;
    case OpIssetG:
      irTranslateIssetG(t, i);
      break;
    case OpUnsetN:
      m_hhbcTrans->emitUnsetN();
      break;
    case OpUnsetG:
      irTranslateUnsetG(t, i);
      break;
    case OpBindG:
      irTranslateBindG(t, i);
      break;
    case OpIterFree:
      irTranslateIterFree(t, i);
      break;
    case OpBPassC:
    case OpBPassV:
      // OpBPass* instructions are no-ops
      break;
    case OpFPassV:
      irTranslateFPassV(t, i);
      break;
    case OpBindS:
      irTranslateBindS(t, i);
      break;
    default:
      // GO: if you hit this, check opNames[op] and add support for it
      HHIR_UNIMPLEMENTED_OP(opNames[op]);
      assert(false);
  }
}

void
TranslatorX64::irTranslateInstrWork(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  const Opcode op = i.op();

  switch (op) {
#define CASE(iNm)                               \
  case Op ## iNm:                               \
                  irTranslate ## iNm(t, i);     \
    break;
#define TRANSLATE(a, b, c) irTranslate ## a(b, c); break;
    INSTRS
      PSEUDOINSTR_DISPATCH(TRANSLATE)
#undef TRANSLATE
#undef CASE
  default:
      irTranslateInstrDefault(t, i);
  }
}

void
TranslatorX64::irPassPredictedAndInferredTypes(const NormalizedInstruction& i) {
  assert(m_useHHIR);

  if (!i.outStack || i.breaksTracelet) return;

  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);

  if ((u == NormalizedInstruction::OutputUsed && i.outputPredicted) ||
      (u == NormalizedInstruction::OutputInferred)) {
    JIT::Type jitType = JIT::Type::fromRuntimeType(i.outStack->rtt);
    if (u == NormalizedInstruction::OutputInferred) {
      TRACE(1, "HHIR: irPassPredictedAndInferredTypes: output inferred as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans->assertTypeStack(0, jitType);
    } else {
      TRACE(1, "HHIR: irPassPredictedAndInferredTypes: output predicted as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans->checkTypeTopOfStack(jitType, i.next->offset());
    }
  }
}

/**
 * Returns the number of cells that instruction i pops from the stack.
 */
static int getNumPopped(const NormalizedInstruction& i) {
  return -getStackDelta(i)
    // getStackDelta includes the output left on the stack, so discount it
    + (i.outStack ? 1 : 0)
    // getStackDelta includes ActRec cells pushed on the stack, so discount them
    + (pushesActRec(i.op()) ? kNumActRecCells : 0);
}

/**
 * Returns the number of Act-Rec cells that instruction i pushes onto the stack.
 */
static int getNumARCellsPushed(const NormalizedInstruction& i) {
  return pushesActRec(i.op()) ? kNumActRecCells : 0;
}

void TranslatorX64::irInterpretInstr(const NormalizedInstruction& i) {
  JIT::Type outStkType = JIT::Type::fromDynLocation(i.outStack);
  int poppedCells      = getNumPopped(i);
  int arPushedCells    = getNumARCellsPushed(i);

  FTRACE(5, "HHIR: BC Instr {}  Popped = {}  ARCellsPushed = {}\n",
         i.toString(), poppedCells, arPushedCells);

  if (i.changesPC) {
    m_hhbcTrans->emitInterpOneCF(poppedCells);
  } else {
    m_hhbcTrans->emitInterpOne(outStkType, poppedCells, arPushedCells);
    if (i.outLocal) {
      // HHIR tracks local values and types, so we should inform it about
      // the new local type.  This is done via an overriding type assertion.
      assert(i.outLocal->isLocal());
      int32_t locId = i.outLocal->location.offset;
      JIT::Type newType = JIT::Type::fromRuntimeType(i.outLocal->rtt);
      m_hhbcTrans->overrideTypeLocal(locId, newType);
    }
  }
}

void TranslatorX64::irTranslateInstr(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  assert(m_useHHIR);
  assert(!i.outStack || i.outStack->isStack());
  assert(!i.outLocal || i.outLocal->isLocal());
  FTRACE(1, "translating: {}\n", opcodeToName(i.op()));

  m_hhbcTrans->setBcOff(i.source.offset(),
                        i.breaksTracelet && !m_hhbcTrans->isInlining());

  if (!i.grouped) {
    emitVariantGuards(t, i);
    const NormalizedInstruction* n = &i;
    while (n->next && n->next->grouped) {
      n = n->next;
      emitVariantGuards(t, *n);
    }
  }

  if (i.guardedThis) {
    // Task #2067635: This should really generate an AssertThis
    m_hhbcTrans->setThisAvailable();
  }

  if (moduleEnabled(HPHP::Trace::stats, 2)) {
    m_hhbcTrans->emitIncStat(Stats::opcodeToIRPreStatCounter(i.op()), 1);
  }
  if (RuntimeOption::EnableInstructionCounts ||
      moduleEnabled(HPHP::Trace::stats, 3)) {
    // If the instruction takes a slow exit, the exit trace will
    // decrement the post counter for that opcode.
    m_hhbcTrans->emitIncStat(Stats::opcodeToIRPostStatCounter(i.op()),
                             1, true);
  }

  if (i.interp) {
    irInterpretInstr(i);
  } else {
    irTranslateInstrWork(t, i);
  }

  irPassPredictedAndInferredTypes(i);
}

void TranslatorX64::irAssertType(const Location& l,
                                 const RuntimeType& rtt) {
  assert(m_useHHIR);
  if (rtt.isVagueValue()) return;

  switch (l.space) {
    case Location::Stack: {
      // tx64LocPhysicalOffset returns positive offsets for stack values,
      // relative to rVmSp
      uint32_t stackOffset = tx64LocPhysicalOffset(l);
      m_hhbcTrans->assertTypeStack(stackOffset,
                                   JIT::Type::fromRuntimeType(rtt));
      break;
    }
    case Location::Local:  // Stack frame's registers; offset == local register
      m_hhbcTrans->assertTypeLocal(l.offset, JIT::Type::fromRuntimeType(rtt));
      break;

    case Location::Invalid:           // Unknown location
      HHIR_UNIMPLEMENTED(Invalid);
      break;

    case Location::Iter:              // Stack frame's iterators
      HHIR_UNIMPLEMENTED(AssertType_Iter);
      break;

    case Location::Litstr:            // Literal string pseudo-location
      HHIR_UNIMPLEMENTED(AssertType_Litstr);
      break;

    case Location::Litint:            // Literal int pseudo-location
      HHIR_UNIMPLEMENTED(AssertType_Litint);
      break;

    case Location::This:
      HHIR_UNIMPLEMENTED(AssertType_This);
      break;
  }
}

void TranslatorX64::irEmitResolvedDeps(const ChangeMap& resolvedDeps) {
  for (const auto dep : resolvedDeps) {
    irAssertType(dep.first, dep.second->rtt);
  }
}

static bool supportedInterpOne(const NormalizedInstruction* i) {
  switch (i->op()) {
    // Instructions that do function return are not supported yet
    case OpRetC:
    case OpRetV:
    case OpContRetC:
    case OpNativeImpl:
      return false;
    default:
      return true;
  }
}

TranslatorX64::TranslateTraceletResult
TranslatorX64::irTranslateTracelet(Tracelet&               t,
                                   const TCA               start,
                                   const TCA               stubStart,
                                   vector<TransBCMapping>* bcMap) {
  auto transResult = Failure;
  assert(m_useHHIR);

  const SrcKey &sk = t.m_sk;
  SrcRec& srcRec = *getSrcRec(sk);
  assert(srcRec.inProgressTailJumps().size() == 0);
  try {
    // Don't translate if we have already reached the maximum # of
    // translations for this tracelet
    HHIR_UNIMPLEMENTED_WHEN(checkTranslationLimit(t.m_sk, srcRec),
                            TOO_MANY_TRANSLATIONS);

    irEmitResolvedDeps(t.m_resolvedDeps);
    emitGuardChecks(a, sk, t.m_dependencies, t.m_refDeps, srcRec);

    dumpTranslationInfo(t, a.code.frontier);

    // after guards, add a counter for the translation if requested
    if (RuntimeOption::EvalJitTransCounters) {
      m_hhbcTrans->emitIncTransCounter();
    }

    emitRB(a, RBTypeTraceletBody, t.m_sk);
    Stats::emitInc(a, Stats::Instr_TC, t.m_numOpcodes);
    recordBCInstr(OpTraceletGuard, a, start);
    m_hhbcTrans->setBcOffNextTrace(t.m_nextSk.offset());

    // Profiling on function entry.
    if (m_curTrace->m_sk.offset() == curFunc()->base()) {
      m_hhbcTrans->profileFunctionEntry("Normal");
    }

    /*
     * Profiling on the shapes of tracelets that are whole functions.
     * (These are the things we might consider trying to support
     * inlining.)
     */
    [&]{
      static const bool enabled = Stats::enabledAny() &&
                                  getenv("HHVM_STATS_FUNCSHAPE");
      if (!enabled) return;
      if (m_curTrace->m_sk.offset() != curFunc()->base()) return;
      if (auto last = m_curTrace->m_instrStream.last) {
        if (last->op() != OpRetC && last->op() != OpRetV) {
          return;
        }
      }
      m_hhbcTrans->profileSmallFunctionShape(traceletShape(*m_curTrace));
    }();

    // Translate each instruction in the tracelet
    for (auto* ni = t.m_instrStream.first; ni; ni = ni->next) {
      try {
        SKTRACE(1, ni->source, "HHIR: irTranslateInstr\n");
        Nuller<NormalizedInstruction> niNuller(&m_curNI);
        m_curNI = ni;
        irTranslateInstr(t, *ni);
      } catch (JIT::FailedIRGen& fcg) {
        // If we haven't tried interpreting ni yet, flag it to be interpreted
        // and retry
        if (RuntimeOption::EvalHHIRDisableTx64 && !ni->interp &&
            supportedInterpOne(ni)) {
          ni->interp = true;
          transResult = Retry;
          break;
        }
        if (!RuntimeOption::EvalHHIRDisableTx64 || !ni->prev) {
          // Let tx64 handle the entire tracelet.
          throw;
        }
        // We've made forward progress. Proceed with the partial tracelet,
        // with this last instruction interpreted. Since the interpretation
        // might have had unknowable side effects, kill the trace right
        // after.
        SKTRACE(1, ni->source, "HHIR: RETRY to translate, breaking tracelet.\n");
        ni = ni->prev;
        ni->breaksTracelet = true;
        t.m_nextSk = ni->next->source;
        transResult = Retry;
        break;
      }
      assert(ni->source.offset() >= curFunc()->base());
      // We sometimes leave the tail of a truncated tracelet in place to aid
      // analysis, but breaksTracelet is authoritative.
      if (ni->breaksTracelet) break;
    }

    hhirTraceEnd(t.m_nextSk.offset());
    if (transResult != Retry) {
      transResult = Success;
      hhirTraceCodeGen(bcMap);

      TRACE(1, "HHIR: SUCCEEDED to generate code for Translation %d\n\n\n",
            getCurrentTransID());
    }
  } catch (JIT::FailedCodeGen& fcg) {
    transResult = Failure;
    if (Trace::moduleEnabled(Trace::punt, 1)) m_lastHHIRPunt = fcg.func;
    TRACE(1, "HHIR: FAILED to generate code for Translation %d "
          "@ %s:%d (%s)\n", getCurrentTransID(),
          fcg.file, fcg.line, fcg.func);
    // HHIR:TODO Remove extra TRACE and adjust tools
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          fcg.file, fcg.line, fcg.func);
  } catch (JIT::FailedIRGen& x) {
    transResult = Failure;
    if (Trace::moduleEnabled(Trace::punt, 1)) m_lastHHIRPunt = x.func;
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          x.file, x.line, x.func);
  } catch (TranslationFailedExc& tfe) {
    not_reached();
  } catch (const FailedAssertion& fa) {
    fa.print();
    StackTraceNoHeap::AddExtraLogging(
      "Assertion failure",
      folly::format("{}\n\nActive Trace:\n{}\n",
                    fa.summary, m_hhbcTrans->getTrace()->toString()).str());
    abort();
  } catch (const std::exception& e) {
    transResult = Failure;
    FTRACE(1, "HHIR: FAILED with exception: {}\n", e.what());
    assert(0);
  }

  m_useHHIR = transResult != Failure;
  if (transResult != Success) {
    // The whole translation failed; give up on this BB. Since it is not
    // linked into srcDB yet, it is guaranteed not to be reachable.
    // Free IR resources for this trace, rollback the Translation cache
    // frontiers, and discard any pending fixups.
    hhirTraceFree();
    a.code.frontier = start;
    astubs.code.frontier = stubStart;
    m_pendingFixups.clear();
    // Reset additions to list of addresses which need to be patched
    srcRec.clearInProgressTailJumps();
  }
  return transResult;
}

void TranslatorX64::hhirTraceStart(Offset bcStartOffset) {
  assert(!m_irFactory);
  assert(m_useHHIR);

  Cell* fp = vmfp();
  if (curFunc()->isGenerator()) {
    fp = (Cell*)Stack::generatorStackBase((ActRec*)fp);
  }
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         " HHIR during translation ",
         color(ANSI_COLOR_END));

  m_useHHIR      = true;
  m_irFactory.reset(new JIT::IRFactory());
  m_hhbcTrans.reset(new JIT::HhbcTranslator(
    *m_irFactory, bcStartOffset, fp - vmsp(), curFunc()));
}

void TranslatorX64::hhirTraceEnd(Offset bcSuccOffset) {
  assert(m_useHHIR);
  m_hhbcTrans->end(bcSuccOffset);
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         "",
         color(ANSI_COLOR_END));
}

void TranslatorX64::hhirTraceCodeGen(vector<TransBCMapping>* bcMap) {
  using namespace JIT;
  assert(m_useHHIR);

  Trace* trace = m_hhbcTrans->getTrace();
  auto finishPass = [&](const char* msg, int level,
                        const RegAllocInfo* regs = nullptr,
                        const LifetimeInfo* lifetime = nullptr) {
    assert(checkCfg(trace, *m_irFactory));
    dumpTrace(level, trace, msg, regs, lifetime);
  };

  finishPass(" after initial translation ", kIRLevel);
  optimizeTrace(trace, m_hhbcTrans->getTraceBuilder());
  finishPass(" after optimizing ", kOptLevel);

  auto* factory = m_irFactory.get();
  if (dumpIREnabled() || RuntimeOption::EvalJitCompareHHIR) {
    LifetimeInfo lifetime(factory);
    RegAllocInfo regs = allocRegsForTrace(trace, factory, &lifetime);
    AsmInfo ai(factory);
    genCodeForTrace(trace, a, astubs, factory, bcMap, this, regs,
                    &lifetime, &ai);
    finishPass(" after reg alloc ", kRegAllocLevel, &regs, &lifetime);
    if (RuntimeOption::EvalJitCompareHHIR) {
      std::ostringstream out;
      dumpTraceImpl(trace, out, &regs, &lifetime, &ai);
      m_lastHHIRDump = out.str();
    } else {
      dumpTrace(kCodeGenLevel, trace, " after code gen ", &regs,
                &lifetime, &ai);
    }
  } else {
    RegAllocInfo regs = allocRegsForTrace(trace, factory);
    genCodeForTrace(trace, a, astubs, factory, bcMap, this, regs);
    finishPass(" after reg alloc ", kRegAllocLevel);
  }

  m_numHHIRTrans++;
  hhirTraceFree();
}

void TranslatorX64::hhirTraceFree() {
  FTRACE(1, "HHIR free: arena size: {}\n",
         m_irFactory->arena().size());
  m_hhbcTrans.reset();
  m_irFactory.reset();
}

}}}
