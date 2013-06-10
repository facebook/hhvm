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
#include <stdint.h>
#include "hphp/runtime/base/strings.h"

#include "folly/Format.h"
#include "folly/Conv.h"
#include "hphp/util/trace.h"
#include "hphp/util/stack_trace.h"
#include "hphp/util/util.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/vm/jit/targetcache.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/linearscan.h"
#include "hphp/runtime/vm/jit/codegen.h"
#include "hphp/runtime/vm/jit/hhbctranslator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/check.h"

// Include last to localize effects to this file
#include "hphp/util/assert_throw.h"

namespace HPHP {
namespace Transl {

using namespace reg;
using namespace Util;
using namespace Trace;
using std::max;

TRACE_SET_MOD(hhir);
#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

#define TVOFF(nm) offsetof(TypedValue, nm)
#define AROFF(nm) offsetof(ActRec, nm)

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
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue()) return;

  switch (l.space) {
  case Location::Stack:
    {
      uint32_t stackOffset = locPhysicalOffset(l);
      m_hhbcTrans->guardTypeStack(stackOffset,
                                  JIT::Type::fromRuntimeType(rtt));
    }
    break;

  case Location::Local:
    m_hhbcTrans->guardTypeLocal(l.offset, JIT::Type::fromRuntimeType(rtt));
    break;

  case Location::Iter:
  case Location::Invalid:
  case Location::Litstr:
  case Location::Litint:
  case Location::This:
    assert(false); // should not happen
  }
}

void
Translator::translateMod(const NormalizedInstruction& i) {
  HHIR_EMIT(Mod);
}

void
Translator::translateBinaryArithOp(const NormalizedInstruction& i) {
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
Translator::translateSameOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpSame || op == OpNSame);
  if (op == OpSame) {
    HHIR_EMIT(Same);
  } else {
    HHIR_EMIT(NSame);
  }
}

void
Translator::translateEqOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpEq || op == OpNeq);
  if (op == OpEq) {
    HHIR_EMIT(Eq);
  } else {
    HHIR_EMIT(Neq);
  }
}

void
Translator::translateLtGtOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpLt || op == OpLte || op == OpGt || op == OpGte);
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);

  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  bool ok = TypeConstraint::equivDataTypes(leftType, rightType) &&
    (i.inputs[0]->isNull() ||
     leftType == KindOfBoolean ||
     i.inputs[0]->isInt());

  HHIR_UNIMPLEMENTED_WHEN(!ok, LtGtOp);
  switch (op) {
    case OpLt  : HHIR_EMIT(Lt);
    case OpLte : HHIR_EMIT(Lte);
    case OpGt  : HHIR_EMIT(Gt);
    case OpGte : HHIR_EMIT(Gte);
    default    : HHIR_UNIMPLEMENTED(LtGtOp);
  }
}

void
Translator::translateUnaryBooleanOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpCastBool || op == OpEmptyL);
  if (op == OpCastBool) {
    HHIR_EMIT(CastBool);
  } else {
    HHIR_EMIT(EmptyL, i.inputs[0]->location.offset);
  }
}

void
Translator::translateBranchOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpJmpZ || op == OpJmpNZ);
  assert(!i.next);

  if (op == OpJmpZ) {
    HHIR_EMIT(JmpZ,  i.offset() + i.imm[0].u_BA);
  } else {
    HHIR_EMIT(JmpNZ, i.offset() + i.imm[0].u_BA);
  }
}

void
Translator::translateCGetL(const NormalizedInstruction& i) {
  const DEBUG_ONLY Opcode op = i.op();
  assert(op == OpFPassL || OpCGetL);
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(inputs[0]->isLocal());

  HHIR_EMIT(CGetL, inputs[0]->location.offset);
}

void
Translator::translateCGetL2(const NormalizedInstruction& ni) {
  const int locIdx   = 1;

  HHIR_EMIT(CGetL2, ni.inputs[locIdx]->location.offset);
}

void
Translator::translateVGetL(const NormalizedInstruction& i) {
  HHIR_EMIT(VGetL, i.inputs[0]->location.offset);
}

void
Translator::translateAssignToLocalOp(const NormalizedInstruction& ni) {
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
Translator::translatePopC(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(!i.outStack && !i.outLocal);

  if (i.inputs[0]->rtt.isVagueValue()) {
    HHIR_EMIT(PopR);
  } else {
    HHIR_EMIT(PopC);
  }
}

void
Translator::translatePopV(const NormalizedInstruction& i) {
  assert(i.inputs[0]->rtt.isVagueValue() || i.inputs[0]->isRef());
  HHIR_EMIT(PopV);
}

void
Translator::translatePopR(const NormalizedInstruction& i) {
  translatePopC(i);
}

void
Translator::translateUnboxR(const NormalizedInstruction& i) {
  if (i.noOp) {
    // statically proved to be unboxed -- just pass that info to the IR
    TRACE(1, "HHIR: translateUnboxR: output inferred to be Cell\n");
    m_hhbcTrans->assertTypeStack(0, JIT::Type::Cell);
  } else {
    HHIR_EMIT(UnboxR);
  }
}

void
Translator::translateNull(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(Null);
}

void
Translator::translateNullUninit(const NormalizedInstruction& i) {
  HHIR_EMIT(NullUninit);
}

void
Translator::translateTrue(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(True);
}

void
Translator::translateFalse(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(False);
}

void
Translator::translateInt(const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);
  assert(!i.outLocal);

  HHIR_EMIT(Int, i.imm[0].u_I64A);
}

void
Translator::translateDouble(const NormalizedInstruction& i) {
  HHIR_EMIT(Double, i.imm[0].u_DA);
}

void
Translator::translateString(const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);
  assert(!i.outLocal);

  HHIR_EMIT(String, (i.imm[0].u_SA));
}

void
Translator::translateArray(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);

  HHIR_EMIT(Array, i.imm[0].u_AA);
}

void
Translator::translateNewArray(const NormalizedInstruction& i) {
  HHIR_EMIT(NewArray, i.imm[0].u_IVA);
}

void
Translator::translateNop(const NormalizedInstruction& i) {
  HHIR_EMIT(Nop);
}

void
Translator::translateAddElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(AddElemC);
}

void
Translator::translateAddNewElemC(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);
  assert(i.inputs[0]->isStack());
  assert(i.inputs[1]->isStack());

  HHIR_EMIT(AddNewElemC);
}

void
Translator::translateCns(const NormalizedInstruction& i) {
  HHIR_EMIT(Cns, i.imm[0].u_SA);
}

void
Translator::translateDefCns(const NormalizedInstruction& i) {
  HHIR_EMIT(DefCns, (i.imm[0].u_SA));
}

void
Translator::translateClsCnsD(const NormalizedInstruction& i) {
  HHIR_EMIT(ClsCnsD, (i.imm[0].u_SA), (i.imm[1].u_SA));
}

void
Translator::translateConcat(const NormalizedInstruction& i) {
  HHIR_EMIT(Concat);
}

void
Translator::translateAdd(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);

  if (i.inputs[0]->valueType() == KindOfArray &&
      i.inputs[1]->valueType() == KindOfArray) {
    HHIR_EMIT(ArrayAdd);
    return;
  }
  HHIR_EMIT(Add);
}

void
Translator::translateXor(const NormalizedInstruction& i) {
  HHIR_EMIT(Xor);
}

void
Translator::translateNot(const NormalizedInstruction& i) {
  HHIR_EMIT(Not);
}

void
Translator::translateBitNot(const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);

  HHIR_EMIT(BitNot);
}

void
Translator::translateCastInt(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack && !i.outLocal);

  HHIR_EMIT(CastInt);
  /* nop */
}

void
Translator::translateCastArray(const NormalizedInstruction& i) {
  HHIR_EMIT(CastArray);
}

void
Translator::translateCastObject(const NormalizedInstruction& i) {
  HHIR_EMIT(CastObject);
}

void
Translator::translateCastDouble(const NormalizedInstruction& i) {
  HHIR_EMIT(CastDouble);
}

void
Translator::translateCastString(const NormalizedInstruction& i) {
  HHIR_EMIT(CastString);
}

void
Translator::translatePrint(const NormalizedInstruction& i) {
  HHIR_EMIT(Print);
}

void
Translator::translateJmp(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, i.noSurprise);
}

void
Translator::translateSwitch(const NormalizedInstruction& i) {
  HHIR_EMIT(Switch, i.immVec, i.imm[1].u_I64A, i.imm[2].u_IVA);
}

void
Translator::translateSSwitch(const NormalizedInstruction& i) {
  HHIR_EMIT(SSwitch, i.immVec);
}

/*
 * translateRetC --
 *
 *   Return to caller with the current activation record replaced with the
 *   top-of-stack return value.
 */
void
Translator::translateRetC(const NormalizedInstruction& i) {
  HHIR_EMIT(RetC, i.inlineReturn);
}

void
Translator::translateRetV(const NormalizedInstruction& i) {
  HHIR_EMIT(RetV, i.inlineReturn);
}

void
Translator::translateNativeImpl(const NormalizedInstruction& ni) {
  HHIR_EMIT(NativeImpl);
}

// emitClsLocalIndex --
// emitStringToClass --
// emitStringToKnownClass --
// emitObjToClass --
// emitClsAndPals --
//   Helpers for AGetC/AGetL.

const int kEmitClsLocalIdx = 0;

void Translator::translateAGetC(const NormalizedInstruction& ni) {
  const StringData* clsName =
    ni.inputs[kEmitClsLocalIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(AGetC, clsName);
}

void Translator::translateAGetL(const NormalizedInstruction& i) {
  assert(i.inputs[kEmitClsLocalIdx]->isLocal());
  const DynLocation* dynLoc = i.inputs[kEmitClsLocalIdx];
  const StringData* clsName = dynLoc->rtt.valueStringOrNull();
  HHIR_EMIT(AGetL, dynLoc->location.offset, clsName);
}

void Translator::translateSelf(const NormalizedInstruction& i) {
  HHIR_EMIT(Self);
}

void Translator::translateParent(const NormalizedInstruction& i) {
  HHIR_EMIT(Parent);
}

void Translator::translateDup(const NormalizedInstruction& ni) {
  HHIR_EMIT(Dup);
}

void Translator::translateCreateCont(const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCont, i.imm[0].u_IVA, i.imm[1].u_SA);
}

void Translator::translateContEnter(const NormalizedInstruction& i) {
  auto after = nextSrcKey(i).offset();

  // ContEnter can't exist in an inlined function right now.  (If it
  // ever can, this curFunc() needs to change.)
  assert(!m_hhbcTrans->isInlining());
  const Func* srcFunc = curFunc();
  int32_t callOffsetInUnit = after - srcFunc->base();

  HHIR_EMIT(ContEnter, callOffsetInUnit);
}

void Translator::translateContExit(const NormalizedInstruction& i) {
  HHIR_EMIT(ContExit);
}

void Translator::translateUnpackCont(const NormalizedInstruction& i) {
  HHIR_EMIT(UnpackCont);
}

void Translator::translatePackCont(const NormalizedInstruction& i) {
  HHIR_EMIT(PackCont, i.imm[0].u_IVA);
}

void Translator::translateContRetC(const NormalizedInstruction& i) {
  HHIR_EMIT(ContRetC);
}

void Translator::translateContNext(const NormalizedInstruction& i) {
  HHIR_EMIT(ContNext);
}

void Translator::translateContSend(const NormalizedInstruction& i) {
  HHIR_EMIT(ContSend);
}

void Translator::translateContRaise(const NormalizedInstruction& i) {
  HHIR_EMIT(ContRaise);
}

void Translator::translateContValid(const NormalizedInstruction& i) {
  HHIR_EMIT(ContValid);
}

void Translator::translateContCurrent(const NormalizedInstruction& i) {
  HHIR_EMIT(ContCurrent);
}

void Translator::translateContStopped(const NormalizedInstruction& i) {
  HHIR_EMIT(ContStopped);
}

void Translator::translateContHandle(const NormalizedInstruction& i) {
  HHIR_EMIT(ContHandle);
}

void Translator::translateStrlen(const NormalizedInstruction& i) {
  HHIR_EMIT(Strlen);
}

void Translator::translateIncStat(const NormalizedInstruction& i) {
  HHIR_EMIT(IncStat, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void Translator::translateArrayIdx(const NormalizedInstruction& i) {
  HHIR_EMIT(ArrayIdx);
}

void Translator::translateClassExists(const NormalizedInstruction& i) {
  const StringData* clsName = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(ClassExists, clsName);
}

void Translator::translateInterfaceExists(const NormalizedInstruction& i) {
  const StringData* ifaceName = i.inputs[1]->rtt.valueStringOrNull();

  HHIR_EMIT(InterfaceExists, ifaceName);
}

void Translator::translateTraitExists(const NormalizedInstruction& i) {
  const StringData* traitName = i.inputs[1]->rtt.valueStringOrNull();

  HHIR_EMIT(TraitExists, traitName);
}

void Translator::translateVGetS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetS, propName);
}

void
Translator::translateVGetG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetG, name);
}

void Translator::translateBindS(const NormalizedInstruction& i) {
  const int kPropIdx = 2;
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(BindS, propName);
}

void Translator::translateEmptyS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(EmptyS, propName);
}

void Translator::translateEmptyG(const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(EmptyG, gblName);
}

void
Translator::translateIssetS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(IssetS, propName);
}

void
Translator::translateIssetG(const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(IssetG, gblName);
}

void
Translator::translateUnsetG(const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(UnsetG, gblName);
}

void
Translator::translateUnsetN(const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetN);
}

void Translator::translateCGetS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetS, propName,
            getInferredOrPredictedType(i), isInferredType(i));
}

void Translator::translateSetS(const NormalizedInstruction& i) {
  const int kPropIdx = 2;
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(SetS, propName);
}

void
Translator::translateCGetG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetG, name, getInferredOrPredictedType(i), isInferredType(i));
}

void Translator::translateSetG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(SetG, name);
}

void Translator::translateBindG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(BindG, name);
}

void
Translator::translateLateBoundCls(const NormalizedInstruction&i) {
  HHIR_EMIT(LateBoundCls);
}

void Translator::translateFPassL(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetL(ni);
  } else {
    translateCGetL(ni);
  }
}

void Translator::translateFPassS(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetS(ni);
  } else {
    translateCGetS(ni);
  }
}

void Translator::translateFPassG(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetG(ni);
  } else {
    translateCGetG(ni);
  }
}

void
Translator::translateCheckTypeOp(const NormalizedInstruction& ni) {
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
Translator::translateAKExists(const NormalizedInstruction& ni) {
  HHIR_EMIT(AKExists);
}

void
Translator::translateSetOpL(const NormalizedInstruction& i) {
  JIT::Opcode opc;
  switch (i.imm[1].u_OA) {
    case SetOpPlusEqual:   opc = JIT::OpAdd;    break;
    case SetOpMinusEqual:  opc = JIT::OpSub;    break;
    case SetOpMulEqual:    opc = JIT::OpMul;    break;
    case SetOpDivEqual:    HHIR_UNIMPLEMENTED(SetOpL_Div);
    case SetOpConcatEqual: opc = JIT::Concat;   break;
    case SetOpModEqual:    HHIR_UNIMPLEMENTED(SetOpL_Mod);
    case SetOpAndEqual:    opc = JIT::OpBitAnd; break;
    case SetOpOrEqual:     opc = JIT::OpBitOr;  break;
    case SetOpXorEqual:    opc = JIT::OpBitXor; break;
    case SetOpSlEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shl);
    case SetOpSrEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shr);
    default: not_reached();
  }
  const int localIdx = 1;
  HHIR_EMIT(SetOpL, opc, i.inputs[localIdx]->location.offset);
}

void
Translator::translateIncDecL(const NormalizedInstruction& i) {
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
Translator::translateUnsetL(const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetL, i.inputs[0]->location.offset);
}

void
Translator::translateReqDoc(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(ReqDoc, name);
}

void Translator::translateDefCls(const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  HHIR_EMIT(DefCls, cid, i.source.offset());
}

void Translator::translateDefFunc(const NormalizedInstruction& i) {
  int fid = i.imm[0].u_IVA;
  HHIR_EMIT(DefFunc, fid);
}

void
Translator::translateFPushFunc(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFunc, (i.imm[0].u_IVA));
}

void
Translator::translateFPushClsMethodD(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushClsMethodD,
             (i.imm[0].u_IVA),
             (i.imm[1].u_SA),
             (i.imm[2].u_SA));
}

void
Translator::translateFPushClsMethodF(const NormalizedInstruction& i) {
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
Translator::translateFPushObjMethodD(const NormalizedInstruction& i) {
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

void Translator::translateFPushCtor(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCtor, (i.imm[0].u_IVA));
}

void Translator::translateFPushCtorD(const NormalizedInstruction& i) {

  HHIR_EMIT(FPushCtorD, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void Translator::translateCreateCl(const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCl, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

// static void fatalNullThis() { raise_error(Strings::FATAL_NULL_THIS); }

void
Translator::translateThis(const NormalizedInstruction &i) {
  HHIR_EMIT(This);
}

void
Translator::translateBareThis(const NormalizedInstruction &i) {
  HHIR_EMIT(BareThis, (i.imm[0].u_OA));
}

void
Translator::translateCheckThis(const NormalizedInstruction& i) {
  HHIR_EMIT(CheckThis);
}

void
Translator::translateInitThisLoc(const NormalizedInstruction& i) {
  HHIR_EMIT(InitThisLoc, i.outLocal->location.offset);
}

void
Translator::translateFPushFuncD(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFuncD, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void
Translator::translateFPassCOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  if (i.preppedByRef && (op == OpFPassCW || op == OpFPassCE)) {
    // These cases might have to raise a warning or an error
    HHIR_UNIMPLEMENTED(FPassCW_FPassCE_byref);
  } else {
    HHIR_EMIT(FPassCOp);
  }
}

void
Translator::translateFPassV(const NormalizedInstruction& i) {
  if (i.preppedByRef || i.noOp) {
    TRACE(1, "HHIR: translateFPassV: noOp\n");
    return;
  }
  HHIR_EMIT(FPassV);
}

void
Translator::translateFPushCufIter(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCufIter, i.imm[0].u_IVA, i.imm[1].u_IA);
}

static const Func*
findCuf(const NormalizedInstruction& ni,
                    Class*& cls, StringData*& invName, bool& forward) {
  forward = (ni.op() == OpFPushCufF);
  cls = nullptr;
  invName = nullptr;

  DynLocation* callable = ni.inputs[ni.op() == OpFPushCufSafe ? 1 : 0];

  const StringData* str =
    callable->isString() ? callable->rtt.valueString() : nullptr;
  const ArrayData* arr =
    callable->isArray() ? callable->rtt.valueArray() : nullptr;

  StringData* sclass = nullptr;
  StringData* sname = nullptr;
  if (str) {
    Func* f = HPHP::Unit::lookupFunc(str);
    if (f) return f;
    String name(const_cast<StringData*>(str));
    int pos = name.find("::");
    if (pos <= 0 || pos + 2 >= name.size() ||
        name.find("::", pos + 2) != String::npos) {
      return nullptr;
    }
    sclass = StringData::GetStaticString(name.substr(0, pos).get());
    sname = StringData::GetStaticString(name.substr(pos + 2).get());
  } else if (arr) {
    if (arr->size() != 2) return nullptr;
    CVarRef e0 = arr->get(int64_t(0), false);
    CVarRef e1 = arr->get(int64_t(1), false);
    if (!e0.isString() || !e1.isString()) return nullptr;
    sclass = e0.getStringData();
    sname = e1.getStringData();
    String name(sname);
    if (name.find("::") != String::npos) return nullptr;
  } else {
    return nullptr;
  }

  Class* ctx = curFunc()->cls();

  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    return nullptr;
  } else {
    cls = Unit::lookupUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(cls, sname, magicCall, true);
  if (!f || (forward && !ctx->classof(f->cls()))) {
    /*
     * To preserve the invariant that the lsb class
     * is an instance of the context class, we require
     * that f's class is an instance of the context class.
     * This is conservative, but without it, we would need
     * a runtime check to decide whether or not to forward
     * the lsb class
     */
    return nullptr;
  }
  if (magicCall) invName = sname;
  return f;
}

void
Translator::translateFPushCufOp(const NormalizedInstruction& i) {
  Class* cls = nullptr;
  StringData* invName = nullptr;
  bool forward = false;
  const Func* func = findCuf(i, cls, invName, forward);
  HHIR_EMIT(FPushCufOp, i.op(), cls, invName, func, i.imm[0].u_IVA);
}

void
Translator::translateFPassR(const NormalizedInstruction& i) {
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
Translator::translateFCallBuiltin(const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  int numNonDefault  = i.imm[1].u_IVA;
  Id funcId = i.imm[2].u_SA;

  HHIR_EMIT(FCallBuiltin, numArgs, numNonDefault, funcId);
}

bool shouldIRInline(const Func* curFunc,
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
    return refuse("more locals than params (unsupported)");
  }
  if (func->numIterators() != 0) {
    return refuse("iterators");
  }
  if (func->maxStackCells() >= kStackCheckLeafPadding) {
    FTRACE(1, "{} >= {}\n", func->maxStackCells(), kStackCheckLeafPadding);
    return refuse("too many stack cells");
  }

  /////////////

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

  // Simple operations that just put a Cell on the stack.  There must
  // either be no inputs, or a single local as an input.  For now
  // avoid CreateCont because it depends on the frame.
  auto simpleCell = [&]() -> bool {
    if (current == OpCreateCont) return false;
    if (cursor->outStack && cursor->inputs.empty()) {
      next();
      return true;
    }
    if (current == OpCGetL || current == OpVGetL) {
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

  resetCursor();

  ////////////

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
    if (func->numParams()) {
      FTRACE(1, "CreateCont with {} args\n", func->numParams());
    }
    next();
    if (atRet()) return accept("continuation creator");
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
Translator::translateFCall(const NormalizedInstruction& i) {
  auto const numArgs = i.imm[0].u_IVA;

  always_assert(!m_hhbcTrans->isInlining() && "curUnit and curFunc calls");
  const Opcode* after = curUnit()->at(nextSrcKey(i).offset());
  const Func* srcFunc = curFunc();
  Offset returnBcOffset =
    srcFunc->unit()->offsetOf(after - srcFunc->base());

  /*
   * If we have a calleeTrace, we're going to see if we should inline
   * the call.
   */
  if (i.calleeTrace) {
    if (!m_hhbcTrans->isInlining()) {
      assert(shouldIRInline(curFunc(), i.funcd, *i.calleeTrace));

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
        translateInstr(*ni);
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
Translator::translateFCallArray(const NormalizedInstruction& i) {
  const Offset pcOffset = i.offset();
  SrcKey next = nextSrcKey(i);
  const Offset after = next.offset();

  HHIR_EMIT(FCallArray, pcOffset, after);
}

void
Translator::translateNewTuple(const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  HHIR_EMIT(NewTuple, numArgs);
}

void
Translator::translateNewCol(const NormalizedInstruction& i) {
  HHIR_EMIT(NewCol, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void
Translator::translateColAddNewElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(ColAddNewElemC);
}

void
Translator::translateColAddElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(ColAddElemC);
}

void
Translator::translateStaticLocInit(const NormalizedInstruction& i) {
  HHIR_EMIT(StaticLocInit, i.imm[0].u_IVA, i.imm[1].u_SA);
}

// check class hierarchy and fail if no match
void
Translator::translateVerifyParamType(const NormalizedInstruction& i) {
  int param = i.imm[0].u_IVA;
  HHIR_EMIT(VerifyParamType, param);
}

void
Translator::translateInstanceOfD(const NormalizedInstruction& i) {
  HHIR_EMIT(InstanceOfD, (i.imm[0].u_SA));
}

void
Translator::translateIterInit(const NormalizedInstruction& i) {
  HHIR_EMIT(IterInit,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
Translator::translateIterInitK(const NormalizedInstruction& i) {
  HHIR_EMIT(IterInitK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
Translator::translateIterNext(const NormalizedInstruction& i) {

  HHIR_EMIT(IterNext,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
Translator::translateIterNextK(const NormalizedInstruction& i) {

  HHIR_EMIT(IterNextK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
Translator::translateWIterInit(const NormalizedInstruction& i) {
  HHIR_EMIT(WIterInit,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
Translator::translateWIterInitK(const NormalizedInstruction& i) {
  HHIR_EMIT(WIterInitK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
Translator::translateWIterNext(const NormalizedInstruction& i) {

  HHIR_EMIT(WIterNext,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
Translator::translateWIterNextK(const NormalizedInstruction& i) {

  HHIR_EMIT(WIterNextK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
Translator::translateIterFree(const NormalizedInstruction& i) {

  HHIR_EMIT(IterFree, i.imm[0].u_IVA);
}

void
Translator::translateDecodeCufIter(const NormalizedInstruction& i) {

  HHIR_EMIT(DecodeCufIter, i.imm[0].u_IVA, i.offset() + i.imm[1].u_BA);
}

void
Translator::translateCIterFree(const NormalizedInstruction& i) {

  HHIR_EMIT(CIterFree, i.imm[0].u_IVA);
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
  void Translator::translate##instr##M(const NormalizedInstruction& ni) { \
    m_hhbcTrans->emitMInstr(ni);                                        \
  }
MINSTRS
MII(FPass)
#undef MII

void
Translator::translateInstrDefault(const NormalizedInstruction& i) {
  const char *opNames[] = {
#define O(name, imm, push, pop, flags) \
"Unimplemented" #name,
  OPCODES
#undef O
  };
  const Opcode op = i.op();

  HHIR_UNIMPLEMENTED_OP(opNames[op]);
  assert(false);
}

void
Translator::translateInstrWork(const NormalizedInstruction& i) {
  const Opcode op = i.op();

  switch (op) {
#define CASE(iNm)                               \
    case Op ## iNm:                             \
      translate ## iNm(i);                    \
      break;
#define TRANSLATE(a, b, c) translate ## a(c); break;
    INSTRS
      PSEUDOINSTR_DISPATCH(TRANSLATE)
#undef TRANSLATE
#undef CASE
  default:
      translateInstrDefault(i);
  }
}

static bool isPop(Opcode opc) {
  return opc == OpPopC || opc == OpPopR;
}

void
Translator::passPredictedAndInferredTypes(const NormalizedInstruction& i) {
  if (!i.outStack || i.breaksTracelet) return;

  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);
  JIT::Type jitType = JIT::Type::fromRuntimeType(i.outStack->rtt);

  if (u == NormalizedInstruction::OutputInferred) {
    TRACE(1, "irPassPredictedAndInferredTypes: output inferred as %s\n",
          jitType.toString().c_str());
    m_hhbcTrans->assertTypeStack(0, jitType);

  } else if ((u == NormalizedInstruction::OutputUsed && i.outputPredicted)) {
    // If the value was predicted statically by the front-end, it
    // means that it's either the predicted type or null.  In this
    // case, if the predicted value is not ref-counted and it's simply
    // going to be popped, then pass the information as an assertion
    // that the type is not ref-counted.  This avoid both generating a
    // type check and dec-refing the value.
    if (i.outputPredictionStatic && isPop(i.next->op()) &&
        !jitType.isCounted()) {
      TRACE(1, "irPassPredictedAndInferredTypes: output inferred as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans->assertTypeStack(0, JIT::Type::Uncounted);
    } else {
      TRACE(1, "irPassPredictedAndInferredTypes: output predicted as %s\n",
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

void Translator::interpretInstr(const NormalizedInstruction& i) {
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

void Translator::translateInstr(const NormalizedInstruction& i) {
  assert(!i.outStack || i.outStack->isStack());
  assert(!i.outLocal || i.outLocal->isLocal());
  FTRACE(1, "translating: {}\n", opcodeToName(i.op()));

  m_hhbcTrans->setBcOff(i.source.offset(),
                        i.breaksTracelet && !m_hhbcTrans->isInlining());

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
    interpretInstr(i);
  } else {
    translateInstrWork(i);
  }

  passPredictedAndInferredTypes(i);
}

void TranslatorX64::irAssertType(const Location& l,
                                 const RuntimeType& rtt) {
  if (rtt.isVagueValue()) return;

  switch (l.space) {
    case Location::Stack: {
      // tx64LocPhysicalOffset returns positive offsets for stack values,
      // relative to rVmSp
      uint32_t stackOffset = locPhysicalOffset(l);
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
        SKTRACE(1, ni->source, "HHIR: translateInstr\n");
        Nuller<NormalizedInstruction> niNuller(&m_curNI);
        m_curNI = ni;
        translateInstr(*ni);
      } catch (JIT::FailedIRGen& fcg) {
        // If we haven't tried interpreting ni yet, flag it to be interpreted
        // and retry
        if (!ni->interp && supportedInterpOne(ni)) {
          ni->interp = true;
          transResult = Retry;
        }
        break;
      }
      assert(ni->source.offset() >= curFunc()->base());
      // We sometimes leave the tail of a truncated tracelet in place to aid
      // analysis, but breaksTracelet is authoritative.
      if (ni->breaksTracelet) break;
    }

    hhirTraceEnd();
    if (transResult != Retry) {
      try {
        transResult = Success;
        hhirTraceCodeGen(bcMap);
      } catch (JIT::FailedCodeGen& fcg) {
        // Code-gen failed. Search for the bytecode instruction that caused the
        // problem, flag it to be interpreted, and retranslate the tracelet.
        NormalizedInstruction *ni;
        for (ni = t.m_instrStream.first; ni; ni = ni->next) {
          if (ni->source.offset() == fcg.bcOff) break;
        }
        if (ni && !ni->interp && supportedInterpOne(ni)) {
          ni->interp = true;
          transResult = Retry;
          TRACE(1, "HHIR: RETRY Translation %d: will interpOne BC instr %s "
                "after failing to code-gen \n\n",
                getCurrentTransID(), ni->toString().c_str());
        } else {
          throw fcg;
        }
      }
      if (transResult == Success) {
        TRACE(1, "HHIR: SUCCEEDED to generate code for Translation %d\n\n\n",
              getCurrentTransID());
      }
    }
  } catch (JIT::FailedCodeGen& fcg) {
    transResult = Failure;
    TRACE(1, "HHIR: FAILED to generate code for Translation %d "
          "@ %s:%d (%s)\n", getCurrentTransID(),
          fcg.file, fcg.line, fcg.func);
    // HHIR:TODO Remove extra TRACE and adjust tools
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          fcg.file, fcg.line, fcg.func);
  } catch (JIT::FailedIRGen& x) {
    transResult = Failure;
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          x.file, x.line, x.func);
  } catch (TranslationFailedExc& tfe) {
    not_reached();
  } catch (const FailedAssertion& fa) {
    fa.print();
    StackTraceNoHeap::AddExtraLogging(
      "Assertion failure",
      folly::format("{}\n\nActive Trace:\n{}\n",
                    fa.summary, m_hhbcTrans->trace()->toString()).str());
    abort();
  } catch (const std::exception& e) {
    transResult = Failure;
    FTRACE(1, "HHIR: FAILED with exception: {}\n", e.what());
    assert(0);
  }

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

void TranslatorX64::hhirTraceStart(Offset bcStartOffset,
                                   Offset nextTraceletOffset) {
  assert(!m_irFactory);

  Cell* fp = vmfp();
  if (curFunc()->isGenerator()) {
    fp = (Cell*)Stack::generatorStackBase((ActRec*)fp);
  }
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         " HHIR during translation ",
         color(ANSI_COLOR_END));

  m_irFactory.reset(new JIT::IRFactory());
  m_hhbcTrans.reset(new JIT::HhbcTranslator(
    *m_irFactory, bcStartOffset, nextTraceletOffset, fp - vmsp(), curFunc()));
}

void TranslatorX64::hhirTraceEnd() {
  m_hhbcTrans->end();
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         "",
         color(ANSI_COLOR_END));
}

void TranslatorX64::hhirTraceCodeGen(vector<TransBCMapping>* bcMap) {
  using namespace JIT;

  HPHP::JIT::IRTrace* trace = m_hhbcTrans->trace();
  auto finishPass = [&](const char* msg, int level,
                        const RegAllocInfo* regs = nullptr,
                        const LifetimeInfo* lifetime = nullptr) {
    dumpTrace(level, trace, msg, regs, lifetime);
    assert(checkCfg(trace, *m_irFactory));
  };

  finishPass(" after initial translation ", kIRLevel);
  optimizeTrace(trace, m_hhbcTrans->traceBuilder());
  finishPass(" after optimizing ", kOptLevel);

  auto* factory = m_irFactory.get();
  if (dumpIREnabled() || RuntimeOption::EvalJitCompareHHIR) {
    LifetimeInfo lifetime(factory);
    RegAllocInfo regs = allocRegsForTrace(trace, factory, &lifetime);
    finishPass(" after reg alloc ", kRegAllocLevel, &regs, &lifetime);
    assert(checkRegisters(trace, *factory, regs));
    AsmInfo ai(factory);
    genCodeForTrace(trace, a, astubs, factory, bcMap, this, regs,
                    &lifetime, &ai);
    if (RuntimeOption::EvalJitCompareHHIR) {
      std::ostringstream out;
      dumpTraceImpl(trace, out, &regs, &lifetime, &ai);
    } else {
      dumpTrace(kCodeGenLevel, trace, " after code gen ", &regs,
                &lifetime, &ai);
    }
  } else {
    RegAllocInfo regs = allocRegsForTrace(trace, factory);
    finishPass(" after reg alloc ", kRegAllocLevel);
    assert(checkRegisters(trace, *factory, regs));
    genCodeForTrace(trace, a, astubs, factory, bcMap, this, regs);
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

}}
