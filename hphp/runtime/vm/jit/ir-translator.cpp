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

#include "hphp/runtime/vm/jit/ir-translator.h"

#include <stdint.h>
#include <algorithm>
#include "hphp/runtime/base/strings.h"

#include "folly/Format.h"
#include "folly/Conv.h"
#include "hphp/util/trace.h"
#include "hphp/util/stack-trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/tracelet.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP {
namespace JIT {

using namespace reg;
using namespace Trace;
using std::max;

TRACE_SET_MOD(hhir);

#define HHIR_EMIT(op, ...)                      \
  do {                                          \
    m_hhbcTrans.emit ## op(__VA_ARGS__);        \
    return;                                     \
  } while (0)

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

IRTranslator::IRTranslator(TransContext context)
  : m_hhbcTrans{context}
{}

void IRTranslator::checkType(const JIT::Location& l,
                             const JIT::RuntimeType& rtt,
                             bool outerOnly) {
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue() || l.isThis()) return;

  switch (l.space) {
    case Location::Stack: {
      uint32_t stackOffset = locPhysicalOffset(l);
      JIT::Type type = JIT::Type(rtt);
      if (type <= Type::Cls) {
        m_hhbcTrans.assertTypeStack(stackOffset, type);
      } else {
        m_hhbcTrans.guardTypeStack(stackOffset, type, outerOnly);
      }
      break;
    }
    case Location::Local:
      m_hhbcTrans.guardTypeLocal(l.offset, Type(rtt), outerOnly);
      break;

    case Location::Iter:
    case Location::Invalid:
    case Location::Litstr:
    case Location::Litint:
    case Location::This:
      not_reached();
  }
}

void IRTranslator::assertType(const JIT::Location& l,
                              const JIT::RuntimeType& rtt) {
  if (rtt.isVagueValue()) return;

  switch (l.space) {
    case Location::Stack: {
      // locPhysicalOffset returns positive offsets for stack values,
      // relative to rVmSp
      uint32_t stackOffset = locPhysicalOffset(l);
      m_hhbcTrans.assertTypeStack(stackOffset, Type(rtt));
      break;
    }
    case Location::Local:  // Stack frame's registers; offset == local register
      m_hhbcTrans.assertTypeLocal(l.offset, Type(rtt));
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

void
IRTranslator::translateBinaryArithOp(const NormalizedInstruction& i) {
  switch (i.op()) {
  case Op::Add:    HHIR_EMIT(Add);
  case Op::Sub:    HHIR_EMIT(Sub);
  case Op::Mul:    HHIR_EMIT(Mul);
  case Op::AddO:   HHIR_EMIT(AddO);
  case Op::SubO:   HHIR_EMIT(SubO);
  case Op::MulO:   HHIR_EMIT(MulO);
  case Op::BitAnd: HHIR_EMIT(BitAnd);
  case Op::BitOr:  HHIR_EMIT(BitOr);
  case Op::BitXor: HHIR_EMIT(BitXor);
  default: break;
  }
  not_reached();
}

void
IRTranslator::translateSameOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == Op::Same || op == Op::NSame);
  if (op == Op::Same) {
    HHIR_EMIT(Same);
  } else {
    HHIR_EMIT(NSame);
  }
}

void
IRTranslator::translateEqOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == Op::Eq || op == Op::Neq);
  if (op == Op::Eq) {
    HHIR_EMIT(Eq);
  } else {
    HHIR_EMIT(Neq);
  }
}

void
IRTranslator::translateLtGtOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == Op::Lt || op == Op::Lte || op == Op::Gt || op == Op::Gte);

  auto leftType = m_hhbcTrans.topType(1, DataTypeGeneric);
  auto rightType = m_hhbcTrans.topType(0, DataTypeGeneric);
  bool ok = equivDataTypes(leftType.toDataType(), rightType.toDataType()) &&
    leftType.subtypeOfAny(Type::Null, Type::Bool, Type::Int);

  HHIR_UNIMPLEMENTED_WHEN(!ok, LtGtOp);
  switch (op) {
    case Op::Lt  : HHIR_EMIT(Lt);
    case Op::Lte : HHIR_EMIT(Lte);
    case Op::Gt  : HHIR_EMIT(Gt);
    case Op::Gte : HHIR_EMIT(Gte);
    default    : HHIR_UNIMPLEMENTED(LtGtOp);
  }
}

void
IRTranslator::translateUnaryBooleanOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == OpCastBool || op == OpEmptyL);
  if (op == OpCastBool) {
    HHIR_EMIT(CastBool);
  } else {
    HHIR_EMIT(EmptyL, i.imm[0].u_LA);
  }
}

void
IRTranslator::translateBranchOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == OpJmpZ || op == OpJmpNZ);

  Offset takenOffset    = i.offset() + i.imm[0].u_BA;
  Offset fallthruOffset = i.offset() + instrLen((Op*)(i.pc()));
  assert(i.breaksTracelet ||
         i.nextOffset == takenOffset ||
         i.nextOffset == fallthruOffset);
  assert(!i.includeBothPaths || !i.breaksTracelet);

  if (i.breaksTracelet || i.nextOffset == fallthruOffset) {
    if (op == OpJmpZ) {
      HHIR_EMIT(JmpZ,  takenOffset, fallthruOffset, i.includeBothPaths,
                i.breaksTracelet);
    } else {
      HHIR_EMIT(JmpNZ, takenOffset, fallthruOffset, i.includeBothPaths,
                i.breaksTracelet);
    }
    return;
  }
  assert(i.nextOffset == takenOffset);
  // invert the branch
  if (op == OpJmpZ) {
    HHIR_EMIT(JmpNZ, fallthruOffset, takenOffset, i.includeBothPaths,
              i.breaksTracelet);
  } else {
    HHIR_EMIT(JmpZ,  fallthruOffset, takenOffset, i.includeBothPaths,
              i.breaksTracelet);
  }
}

void
IRTranslator::translateAssignToLocalOp(const NormalizedInstruction& ni) {
  auto const op = ni.op();
  assert(op == OpSetL || op == OpBindL);

  if (op == OpSetL) {
    HHIR_EMIT(SetL, ni.imm[0].u_LA);
  } else {
    HHIR_EMIT(BindL, ni.imm[0].u_LA);
  }
}

void
IRTranslator::translateUnboxR(const NormalizedInstruction& i) {
  if (i.noOp) {
    // statically proved to be unboxed -- just pass that info to the IR
    TRACE(1, "HHIR: translateUnboxR: output inferred to be Cell\n");
    m_hhbcTrans.assertTypeStack(0, JIT::Type::Cell);
  } else {
    HHIR_EMIT(UnboxR);
  }
}

void IRTranslator::translateUnboxRNop(const NormalizedInstruction& i) {
  TRACE(1, "HHIR: translateUnboxR: output inferred to be Cell\n");
  assert(i.noOp);
  m_hhbcTrans.assertTypeStack(0, JIT::Type::Cell);
}

void
IRTranslator::translateBoxR(const NormalizedInstruction& i) {
  if (i.noOp) {
    // statically proved to be unboxed -- just pass that info to the IR
    TRACE(1, "HHIR: translateBoxR: output inferred to be Box\n");
    m_hhbcTrans.assertTypeStack(0, JIT::Type::BoxedCell);
  } else {
    HHIR_UNIMPLEMENTED(BoxR);
  }
}

void IRTranslator::translateBoxRNop(const NormalizedInstruction& i) {
  assert(i.noOp);
  m_hhbcTrans.assertTypeStack(0, JIT::Type::BoxedCell);
}

void
IRTranslator::translateInitProp(const NormalizedInstruction& i) {
  HHIR_EMIT(InitProp, i.imm[0].u_SA, static_cast<InitPropOp>(i.imm[1].u_OA));
}

void IRTranslator::translateAssertRATL(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertRATL, i.imm[0].u_IVA, i.imm[1].u_RATA);
}

void IRTranslator::translateAssertRATStk(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertRATStk, i.imm[0].u_IVA, i.imm[1].u_RATA);
}

void IRTranslator::translateBreakTraceHint(const NormalizedInstruction&) {
}

void
IRTranslator::translateClsCnsD(const NormalizedInstruction& i) {
  HHIR_EMIT(ClsCnsD, (i.imm[0].u_SA), (i.imm[1].u_SA), i.outPred);
}

void
IRTranslator::translateAdd(const NormalizedInstruction& i) {
  auto leftType = m_hhbcTrans.topType(1);
  auto rightType = m_hhbcTrans.topType(0);
  if (leftType <= Type::Arr && rightType <= Type::Arr) {
    HHIR_EMIT(ArrayAdd);
  } else {
    HHIR_EMIT(Add);
  }
}

void
IRTranslator::translateAddO(const NormalizedInstruction& i) {
  auto leftType = m_hhbcTrans.topType(1);
  auto rightType = m_hhbcTrans.topType(0);
  if (leftType <= Type::Arr && rightType <= Type::Arr) {
    HHIR_EMIT(ArrayAdd);
  } else {
    HHIR_EMIT(AddO);
  }
}

void IRTranslator::translateConcatN(const NormalizedInstruction& i) {
  HHIR_EMIT(ConcatN, i.imm[0].u_IVA);
}

void IRTranslator::translateJmp(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, false);
}

void IRTranslator::translateJmpNS(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, true);
}

void
IRTranslator::translateSwitch(const NormalizedInstruction& i) {
  HHIR_EMIT(Switch, i.immVec, i.imm[1].u_I64A, i.imm[2].u_IVA);
}

void
IRTranslator::translateSSwitch(const NormalizedInstruction& i) {
  HHIR_EMIT(SSwitch, i.immVec);
}

/*
 * translateRetC --
 *
 *   Return to caller with the current activation record replaced with the
 *   top-of-stack return value.
 */
void
IRTranslator::translateRetC(const NormalizedInstruction& i) {
  HHIR_EMIT(RetC, i.inlineReturn);
}

void
IRTranslator::translateRetV(const NormalizedInstruction& i) {
  HHIR_EMIT(RetV, i.inlineReturn);
}

void IRTranslator::translateCreateCont(const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCont, i.nextSk().offset());
}

void IRTranslator::translateContEnter(const NormalizedInstruction& i) {
  HHIR_EMIT(ContEnter, i.nextSk().offset());
}

void IRTranslator::translateContRaise(const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(ContRaise);
}

void IRTranslator::translateYield(const NormalizedInstruction& i) {
  HHIR_EMIT(Yield, i.nextSk().offset());
}

void IRTranslator::translateYieldK(const NormalizedInstruction& i) {
  HHIR_EMIT(YieldK, i.nextSk().offset());
}

void IRTranslator::translateAwait(const NormalizedInstruction& i) {
  HHIR_EMIT(Await, i.nextSk().offset(), i.imm[0].u_IVA);
}

void IRTranslator::translateIncStat(const NormalizedInstruction& i) {
  HHIR_EMIT(IncStat, i.imm[0].u_IVA, i.imm[1].u_IVA, false);
}

void IRTranslator::translateFPassL(const NormalizedInstruction& ni) {
  auto locId = ni.imm[1].u_LA;
  if (ni.preppedByRef) {
    HHIR_EMIT(VGetL, locId);
  } else {
    HHIR_EMIT(FPassL, locId);
  }
}

void IRTranslator::translateFPassS(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    unpackVGetS(nullptr, ni);
  } else {
    unpackCGetS(nullptr, ni);
  }
}

void IRTranslator::translateFPassG(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    unpackVGetG(nullptr, ni);
  } else {
    unpackCGetG(nullptr, ni);
  }
}

static inline DataType typeOpToDataType(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:  return KindOfNull;
  case IsTypeOp::Int:   return KindOfInt64;
  case IsTypeOp::Dbl:   return KindOfDouble;
  case IsTypeOp::Bool:  return KindOfBoolean;
  case IsTypeOp::Str:   return KindOfString;
  case IsTypeOp::Arr:   return KindOfArray;
  case IsTypeOp::Obj:   return KindOfObject;
  case IsTypeOp::Scalar: not_reached();
  }
  not_reached();
}

void
IRTranslator::translateCheckTypeLOp(const NormalizedInstruction& ni) {
  auto const locId = ni.imm[0].u_LA;
  auto const op    = static_cast<IsTypeOp>(ni.imm[1].u_OA);
  if (op == IsTypeOp::Scalar) {
    HHIR_EMIT(IsScalarL, locId);
  } else {
    DataType t = typeOpToDataType(op);
    HHIR_EMIT(IsTypeL, locId, t);
  }
}

void
IRTranslator::translateCheckTypeCOp(const NormalizedInstruction& ni) {
  auto const op = static_cast<IsTypeOp>(ni.imm[0].u_OA);
  if (op == IsTypeOp::Scalar) {
    HHIR_EMIT(IsScalarC);
  } else {
    DataType t = typeOpToDataType(op);
    HHIR_EMIT(IsTypeC, t);
  }
}

void
IRTranslator::translateSetOpL(const NormalizedInstruction& i) {
  auto const opc = [&] {
    switch (static_cast<SetOpOp>(i.imm[1].u_OA)) {
    case SetOpOp::PlusEqual:   return Op::Add;
    case SetOpOp::MinusEqual:  return Op::Sub;
    case SetOpOp::MulEqual:    return Op::Mul;
    case SetOpOp::PlusEqualO:  return Op::AddO;
    case SetOpOp::MinusEqualO: return Op::SubO;
    case SetOpOp::MulEqualO:   return Op::MulO;
    case SetOpOp::DivEqual:    HHIR_UNIMPLEMENTED(SetOpL_Div);
    case SetOpOp::ConcatEqual: return Op::Concat;
    case SetOpOp::ModEqual:    HHIR_UNIMPLEMENTED(SetOpL_Mod);
    case SetOpOp::PowEqual:    HHIR_UNIMPLEMENTED(SetOpL_Pow);;
    case SetOpOp::AndEqual:    return Op::BitAnd;
    case SetOpOp::OrEqual:     return Op::BitOr;
    case SetOpOp::XorEqual:    return Op::BitXor;
    case SetOpOp::SlEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shl);
    case SetOpOp::SrEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shr);
    }
    not_reached();
  }();
  HHIR_EMIT(SetOpL, opc, i.imm[0].u_LA);
}

void
IRTranslator::translateIncDecL(const NormalizedInstruction& i) {
  auto const op = static_cast<IncDecOp>(i.imm[1].u_OA);
  HHIR_EMIT(IncDecL, isPre(op), isInc(op), isIncDecO(op), i.imm[0].u_LA);
}

void IRTranslator::translateDefCls(const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  HHIR_EMIT(DefCls, cid, i.source.offset());
}

void IRTranslator::translateNopDefCls(const NormalizedInstruction&) {}

void
IRTranslator::translateFPassCOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  if (i.noOp) return;
  if (i.preppedByRef && (op == OpFPassCW || op == OpFPassCE)) {
    // These cases might have to raise a warning or an error
    HHIR_UNIMPLEMENTED(FPassCW_FPassCE_byref);
  } else {
    HHIR_EMIT(FPassCOp);
  }
}

void
IRTranslator::translateFPassV(const NormalizedInstruction& i) {
  if (i.preppedByRef || i.noOp) {
    TRACE(1, "HHIR: translateFPassV: noOp\n");
    return;
  }
  HHIR_EMIT(FPassV);
}

void IRTranslator::translateFPassVNop(const NormalizedInstruction& i) {
  assert(i.noOp);
}

void
IRTranslator::translateFPushCufOp(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCufOp, i.op(), i.imm[0].u_IVA);
}

void
IRTranslator::translateFPassR(const NormalizedInstruction& i) {
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
IRTranslator::translateFCallBuiltin(const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  int numNonDefault  = i.imm[1].u_IVA;
  Id funcId = i.imm[2].u_SA;

  HHIR_EMIT(FCallBuiltin, numArgs, numNonDefault, funcId,
            JIT::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

static bool isInlinableCPPBuiltin(const Func* f) {
  if (f->attrs() & (AttrNoFCallBuiltin|AttrStatic) ||
      f->numParams() > Native::maxFCallBuiltinArgs() ||
      !f->nativeFuncPtr()) {
    return false;
  }
  if (f->returnType() == KindOfDouble && !Native::allowFCallBuiltinDoubles()) {
    return false;
  }
  if (!f->methInfo()) {
    // TODO(#4313939): hni builtins
    return false;
  }
  auto const info = f->methInfo();
  if (info->attribute & (ClassInfo::NoFCallBuiltin |
                         ClassInfo::VariableArguments |
                         ClassInfo::RefVariableArguments |
                         ClassInfo::MixedVariableArguments)) {
    return false;
  }

  // Don't do this for things which require this pointer adjustments
  // for now.
  if (f->cls() && f->cls()->preClass()->builtinODOffset() != 0) {
    return false;
  }

  /*
   * Right now the IR isn't prepared to do parameter coercing during
   * an inlining of NativeImpl for any of our param modes.  We'll want
   * to expand this in the short term, but for now we're targeting
   * collection member functions, which are a) idl-based (so no hni
   * support here yet), and b) take const Variant&'s for every param.
   *
   * So for now, we only inline cases where the params are Variants.
   */
  for (auto i = uint32_t{0}; i < f->numParams(); ++i) {
    if (f->params()[i].builtinType() != KindOfUnknown) {
      return false;
    }
  }

  return true;
}

// Conservative whitelist for hhbc opcodes we know are safe to inline,
// even if the entire callee body required a AttrMayUseVV.  This
// affects cases where we're able to eliminate control flow while
// inlining due to the parameter types, and the AttrMayUseVV flag was
// due to something happening in a block we won't inline.
static bool isInliningVVSafe(Op op) {
  switch (op) {
  case Op::Null:
  case Op::AssertRATL:
  case Op::AssertRATStk:
  case Op::SetL:
  case Op::CGetL:
  case Op::PopC:
  case Op::JmpNS:
  case Op::JmpNZ:
  case Op::JmpZ:
  case Op::VerifyParamType:
  case Op::VerifyRetTypeC:
  case Op::IsTypeL:
  case Op::RetC:
    return true;
  default:
    break;
  }
  return false;
}

bool shouldIRInline(const Func* caller, const Func* callee, RegionIter& iter) {
  if (!RuntimeOption::EvalHHIREnableGenTimeInlining) {
    return false;
  }
  if (arch() == Arch::ARM) {
    // TODO(#3331014): hack until more ARM codegen is working.
    return false;
  }
  if (caller->isPseudoMain()) {
    // TODO(#4238160): Hack inlining into pseudomain callsites is still buggy
    return false;
  }

  auto refuse = [&](const char* why) -> bool {
    FTRACE(1, "shouldIRInline: refusing {} <reason: {}> [NI = {}]\n",
           callee->fullName()->data(), why,
           iter.finished() ? "<end>" : iter.sk().showInst());
    return false;
  };
  auto accept = [&](const char* kind) -> bool {
    FTRACE(1, "shouldIRInline: inlining {} <kind: {}>\n",
           callee->fullName()->data(), kind);
    return true;
  };

  if (callee->isCPPBuiltin() &&
      static_cast<Op>(*callee->getEntry()) == Op::NativeImpl) {
    if (isInlinableCPPBuiltin(callee)) {
      return accept("inlinable CPP builtin");
    }
    return refuse("non-inlinable CPP builtin");
  }

  // If the function may use a varenv or may be variadic, we only
  // support certain whitelisted instructions which we know won't
  // actually require this.
  bool const needCheckVVSafe = callee->attrs() & AttrMayUseVV;

  if (callee->numIterators() != 0) {
    return refuse("iterators");
  }
  if (callee->isMagic() || Func::isSpecial(callee->name())) {
    return refuse("special or magic function");
  }
  if (callee->isResumable()) {
    return refuse("resumables");
  }
  if (callee->maxStackCells() >= kStackCheckLeafPadding) {
    return refuse("function stack depth too deep");
  }
  if (callee->isMethod() && callee->cls() == c_Generator::classof()) {
    return refuse("Generator member function");
  }

  ////////////

  /*
   * Note: this code contains a stack of Func*'s and looks like it's
   * trying to track multi-level inlining of calls, but it doesn't
   * ever happen if you are using a Tracelet that came from analyze().
   *
   * Don't rely on it for correctness.
   */
  assert(!iter.finished() && "shouldIRInline given empty region");
  const bool hotCallingCold = !(callee->attrs() & AttrHot) &&
                               (caller->attrs() & AttrHot);
  uint64_t cost = 0;
  int inlineDepth = 0;
  auto op = Op::LowInvalid;
  smart::vector<const Func*> funcs;
  auto func = callee;
  funcs.push_back(func);

  for (; !iter.finished(); iter.advance()) {
    // If func has changed after an FCall, we've started an inlined call. This
    // will have to change when we support inlining recursive calls.
    if (func != iter.sk().func()) {
      assert(isRet(op) || op == Op::FCall || op == Op::FCallD);
      if (op == Op::FCall || op == Op::FCallD) {
        funcs.push_back(iter.sk().func());
        int totalDepth = 0;
        for (auto* f : funcs) {
          totalDepth += f->maxStackCells();
        }
        if (totalDepth >= kStackCheckLeafPadding) {
          // NB: for correctness a situation like this /must/ also be
          // refused earlier in analyzeCallee if you are using a
          // Tracelet---this code is not going to run if 'iter' is a
          // TraceletIter.
          return refuse("stack too deep after nested inlining");
        }
        ++inlineDepth;
      }
    }
    op = iter.sk().op();
    func = iter.sk().func();

    if (needCheckVVSafe && !isInliningVVSafe(op)) {
      FTRACE(2, "shouldIRInline: {} is not VV safe\n", opcodeToName(op));
      return refuse("may use dynamic environment");
    }

    // If we hit a RetC/V while inlining, leave that level and
    // continue. Otherwise, accept the tracelet.
    if (isRet(op)) {
      if (inlineDepth > 0) {
        --inlineDepth;
        funcs.pop_back();
        continue;
      } else {
        assert(inlineDepth == 0);
        return accept("entire function fits in one region");
      }
    }

    if (op == Op::FCallArray) return refuse("FCallArray");

    // These opcodes don't indicate any additional work in the callee,
    // so they shouldn't count toward the inlining cost.
    if (op == Op::AssertRATL || op == Op::AssertRATStk) {
      continue;
    }

    cost += 1;

    // Check for an immediate vector, and if it's present add its size to the
    // cost.
    auto const pc = reinterpret_cast<const Op*>(iter.sk().pc());
    if (hasMVector(op)) {
      cost += getMVector(pc).size();
    } else if (hasImmVector(op)) {
      cost += getImmVector(pc).size();
    }

    if (cost > RuntimeOption::EvalHHIRInliningMaxCost) {
      return refuse("too expensive");
    }

    if (cost > RuntimeOption::EvalHHIRAlwaysInlineMaxCost &&
        hotCallingCold) {
      return refuse("inlining sizeable cold func into hot func");
    }
  }

  return refuse("region doesn't end in RetC/RetV");
}

struct TraceletIter : public RegionIter {
  explicit TraceletIter(const Tracelet& tlet)
    : m_current(tlet.m_instrStream.first)
  {}

  bool finished() const { return m_current == nullptr; }

  SrcKey sk() const {
    assert(!finished());
    return m_current->source;
  }

  void advance() {
    assert(!finished());
    m_current = m_current->next;
  }

 private:
  const NormalizedInstruction* m_current;
};

bool shouldIRInline(const Func* caller, const Func* callee,
                    const Tracelet& tlet) {
  TraceletIter iter(tlet);
  return shouldIRInline(caller, callee, iter);
}

void
IRTranslator::translateFCall(const NormalizedInstruction& i) {
  auto const numArgs = i.imm[0].u_IVA;

  const PC after = m_hhbcTrans.curUnit()->at(i.nextSk().offset());
  const Func* srcFunc = m_hhbcTrans.curFunc();
  Offset returnBcOffset =
    srcFunc->unit()->offsetOf(after - srcFunc->base());

  /*
   * If we have a calleeTrace, we're going to see if we should inline
   * the call.
   */
  if (i.calleeTrace) {
    if (!i.calleeTrace->m_inliningFailed) {
      assert(shouldIRInline(m_hhbcTrans.curFunc(), i.funcd, *i.calleeTrace));

      m_hhbcTrans.beginInlining(numArgs, i.funcd, returnBcOffset, Type::Gen);
      static const bool shapeStats = Stats::enabledAny() &&
                                     getenv("HHVM_STATS_INLINESHAPE");
      if (shapeStats) {
        m_hhbcTrans.profileInlineFunctionShape(traceletShape(*i.calleeTrace));
      }

      for (auto* ni = i.calleeTrace->m_instrStream.first; ni; ni = ni->next) {
        if (isAlwaysNop(ni->op())) {
          // This might not be necessary---but for now it's preserving
          // side effects of the call to readMetaData that used to
          // exist here.
          ni->noOp = true;
        }
        translateInstr(*ni);
      }
      return;
    }

    static const auto enabled = Stats::enabledAny() &&
                                getenv("HHVM_STATS_FAILEDINL");
    if (enabled) {
      m_hhbcTrans.profileFunctionEntry("FailedCandidate");
      m_hhbcTrans.profileFailedInlShape(traceletShape(*i.calleeTrace));
    }
  }

  HHIR_EMIT(FCall, numArgs, returnBcOffset, i.funcd,
            JIT::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void IRTranslator::translateFCallD(const NormalizedInstruction& i) {
  translateFCall(i);
}

void
IRTranslator::translateFCallArray(const NormalizedInstruction& i) {
  const Offset pcOffset = i.offset();
  SrcKey next = i.nextSk();
  const Offset after = next.offset();

  HHIR_EMIT(FCallArray, pcOffset, after,
            JIT::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void
IRTranslator::translateNewStructArray(const NormalizedInstruction& i) {
  auto numArgs = i.immVec.size();
  auto ids = i.immVec.vec32();
  auto unit = m_hhbcTrans.curUnit();
  StringData* keys[MixedArray::MaxMakeSize];
  for (size_t i = 0; i < numArgs; i++) {
    keys[i] = unit->lookupLitstrId(ids[i]);
  }
  HHIR_EMIT(NewStructArray, numArgs, keys);
}

/*
 * This function returns the offset of instruction i's branch target.
 * This is normally the offset corresponding to the branch being
 * taken.  However, if i does not break a trace and it's followed in
 * the trace by the instruction in the taken branch, then this
 * function returns the offset of the i's fall-through instruction.
 * In that case, the invertCond output argument is set to true;
 * otherwise it's set to false.
 */
static Offset getBranchTarget(const NormalizedInstruction& i,
                              bool& invertCond) {
  assert(instrJumpOffset((Op*)(i.pc())) != nullptr);
  Offset targetOffset = i.offset() + i.imm[1].u_BA;
  invertCond = false;

  if (!i.breaksTracelet && i.nextOffset == targetOffset) {
    invertCond = true;
    Offset fallthruOffset = i.offset() + instrLen((Op*)i.pc());
    targetOffset = fallthruOffset;
  }

  return targetOffset;
}

void
IRTranslator::translateIterInit(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterInit,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
}

void
IRTranslator::translateIterInitK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterInitK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond);
}

void
IRTranslator::translateIterNext(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterNext,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
}

void
IRTranslator::translateIterNextK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterNextK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond);
}

void
IRTranslator::translateMIterInit(const NormalizedInstruction& i) {
  HHIR_EMIT(MIterInit,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
IRTranslator::translateMIterInitK(const NormalizedInstruction& i) {
  HHIR_EMIT(MIterInitK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
IRTranslator::translateMIterNext(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterNext,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
IRTranslator::translateMIterNextK(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterNextK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
IRTranslator::translateWIterInit(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterInit,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
}

void
IRTranslator::translateWIterInitK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterInitK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond);
}

void
IRTranslator::translateWIterNext(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterNext,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
}

void
IRTranslator::translateWIterNextK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterNextK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond);
}

void
IRTranslator::translateIterBreak(const NormalizedInstruction& i) {

  assert(i.breaksTracelet);
  HHIR_EMIT(IterBreak, i.immVec, i.offset() + i.imm[1].u_BA, i.breaksTracelet);
}

void
IRTranslator::translateDecodeCufIter(const NormalizedInstruction& i) {

  HHIR_EMIT(DecodeCufIter, i.imm[0].u_IVA, i.offset() + i.imm[1].u_BA);
}


/*
 * Generate HhbcTranslator method callers for all bytecodes, using its
 * table-defined signature.
 *
 * The static_cast is to make it so that the invalid emit##nm call is due to
 * template parameter substitution failure and thus Not An Error.
 */
#define O(nm, imms, pop, push, flags) \
  template<class HT> \
  typename std::enable_if<HT::supports##nm, void>::type \
  IRTranslator::unpack##nm(std::nullptr_t, const NormalizedInstruction& ni) { \
    static_cast<HT&>(m_hhbcTrans).emit##nm(imms);  \
  }
#define NA /**/
#define ONE(T) ni.imm[0].u_##T
#define TWO(T1, T2) ni.imm[0].u_##T1, ni.imm[1].u_##T2
#define THREE(T1, T2, T3) \
  ni.imm[0].u_##T1, ni.imm[1].u_##T2, ni.imm[2].u_##T3
#define FOUR(T1, T2, T3, T4) \
  ni.imm[0].u_##T1, ni.imm[1].u_##T2, ni.imm[2].u_##T3, ni.imm[3].u_##T4
#define u_OA(_) u_OA

OPCODES

#undef FOUR
#undef THREE
#undef TWO
#undef ONE
#undef NA
#undef u_OA
#undef O

// All vector instructions are handled by one HhbcTranslator method.
#define MII(instr, ...)                                                 \
  void IRTranslator::translate##instr##M(const NormalizedInstruction& ni) { \
    m_hhbcTrans.emitMInstr(ni);                                        \
  }
MINSTRS
MII(FPass)
#undef MII

void
IRTranslator::translateInstrWork(const NormalizedInstruction& i) {
  auto const op = i.op();

  switch (op) {
#define CASE(iNm) \
    case Op::iNm: return unpack ## iNm(nullptr, i);

    REGULAR_INSTRS
#undef CASE

#define CASE(nm) \
    case Op::nm: return translate ## nm(i); break;
#define TRANSLATE(name, inst) translate ## name(i); break;
    IRREGULAR_INSTRS
    PSEUDOINSTR_DISPATCH(TRANSLATE)
#undef TRANSLATE
#undef CASE
    default:
      always_assert(false);
  }
}

static bool isPop(Op opc) {
  return opc == OpPopC || opc == OpPopR;
}

void
IRTranslator::passPredictedAndInferredTypes(const NormalizedInstruction& i) {
  if (!i.outStack || i.breaksTracelet) return;
  auto const jitType = Type(i.outStack->rtt);

  m_hhbcTrans.setBcOff(i.next->offset(), false);
  if (shouldHHIRRelaxGuards()) {
    if (i.outputPredicted) {
      if (i.outputPredictionStatic && jitType.notCounted()) {
        // If the prediction is from static analysis it really means jitType |
        // InitNull. When jitType is an uncounted type, we know that the value
        // will always be an uncounted type, so we assert that fact before
        // doing the real check. This allows us to relax the CheckType away
        // while still eliminating some refcounting operations.
        m_hhbcTrans.assertTypeStack(0, Type::Uncounted);
      }
      m_hhbcTrans.checkTypeTopOfStack(jitType, i.next->offset());
    }
    return;
  }

  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);

  if (u == NormalizedInstruction::OutputUse::Inferred) {
    TRACE(1, "irPassPredictedAndInferredTypes: output inferred as %s\n",
          jitType.toString().c_str());
    m_hhbcTrans.assertTypeStack(0, jitType);

  } else if (u == NormalizedInstruction::OutputUse::Used && i.outputPredicted) {
    // If the value was predicted statically by the front-end, it
    // means that it's either the predicted type or null.  In this
    // case, if the predicted value is not ref-counted and it's simply
    // going to be popped, then pass the information as an assertion
    // that the type is not ref-counted.  This avoid both generating a
    // type check and dec-refing the value.
    if (i.outputPredictionStatic && isPop(i.next->op()) &&
        !jitType.maybeCounted()) {
      TRACE(1, "irPassPredictedAndInferredTypes: output inferred as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans.assertTypeStack(0, JIT::Type::Uncounted);
    } else {
      TRACE(1, "irPassPredictedAndInferredTypes: output predicted as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans.checkTypeTopOfStack(jitType, i.next->offset());
    }
  }
}

void IRTranslator::interpretInstr(const NormalizedInstruction& i) {
  FTRACE(5, "HHIR: BC Instr {}\n",  i.toString());
  m_hhbcTrans.emitInterpOne(i);
}

static Type flavorToType(FlavorDesc f) {
  switch (f) {
    case NOV: not_reached();

    case CV: return Type::Cell;  // TODO(#3029148) this could be InitCell
    case UV: return Type::Uninit;
    case VV: return Type::BoxedCell;
    case AV: return Type::Cls;
    case RV: case FV: case CVV: case CVUV: return Type::Gen;
  }
  not_reached();
}

void IRTranslator::translateInstr(const NormalizedInstruction& ni) {
  auto& ht = m_hhbcTrans;
  ht.setBcOff(ni.source.offset(),
              ni.breaksTracelet && !m_hhbcTrans.isInlining());
  FTRACE(1, "\n{:-^60}\n", folly::format("Translating {}: {} with stack:\n{}",
                                         ni.offset(), ni.toString(),
                                         ht.showStack()));
  // When profiling, we disable type predictions to avoid side exits
  assert(IMPLIES(JIT::tx->mode() == TransKind::Profile, !ni.outputPredicted));

  if (ni.guardedThis) {
    // Task #2067635: This should really generate an AssertThis
    ht.setThisAvailable();
  }

  ht.emitRB(RBTypeBytecodeStart, ni.source, 2);

  auto pc = reinterpret_cast<const Op*>(ni.pc());
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type = flavorToType(instrInputFlavor(pc, i));
    if (type != Type::Gen) m_hhbcTrans.assertTypeStack(i, type);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts >= 2) {
    ht.emitDbgAssertRetAddr();
  }

  if (instrMustInterp(ni) || ni.interp) {
    interpretInstr(ni);
  } else {
    translateInstrWork(ni);
  }

  passPredictedAndInferredTypes(ni);
}

}}
