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
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <strstream>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string>
#include <queue>
#include <zlib.h>
#include <unwind.h>

#ifdef __FreeBSD__
# include <ucontext.h>
typedef __sighandler_t *sighandler_t;
# define RIP_REGISTER(v) (v).mc_rip
#else
# define RIP_REGISTER(v) (v).gregs[REG_RIP]
#endif

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <boost/scoped_ptr.hpp>

#include <util/pathtrack.h>
#include <util/trace.h>
#include <util/bitops.h>
#include <util/debug.h>
#include <util/ringbuffer.h>
#include <util/rank.h>
#include <util/timer.h>

#include <runtime/base/tv_macros.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/php_debug.h>
#include <runtime/vm/runtime.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/strings.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/ext/ext_continuation.h>
#include <runtime/vm/debug/debug.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/translator/log.h>
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/translator/asm-x64.h>
#include <runtime/vm/translator/srcdb.h>
#include <runtime/vm/translator/x64-util.h>
#include <runtime/vm/translator/unwind-x64.h>
#include <runtime/vm/pendq.h>
#include <runtime/vm/treadmill.h>
#include <runtime/vm/stats.h>
#include <runtime/vm/pendq.h>
#include <runtime/vm/treadmill.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/type-profile.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/vm/translator/hopt/ir.h>
#include <runtime/vm/translator/hopt/linearscan.h>
#include <runtime/vm/translator/hopt/codegen.h>

namespace HPHP {
namespace VM {
namespace Transl {

using namespace reg;
using namespace Util;
using namespace Trace;
using std::max;

static const Trace::Module TRACEMOD = Trace::tx64;
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
tx64LocPhysicalOffset(const Location& l, const Func *f = NULL) {
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
  return (i.outputIsUsed(i.outStack) == NormalizedInstruction::OutputInferred);
}

JIT::Type::Tag getInferredOrPredictedType(const NormalizedInstruction& i) {
  NormalizedInstruction::OutputUse u = i.outputIsUsed(i.outStack);
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
  ASSERT(m_useHHIR);
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue()) return;

  if (l.space == Location::Stack) {
    // tx64LocPhysicalOffset returns:
    // negative offsets for locals accessed via rVmFp
    // positive offsets for stack values, relative to rVmSp
    uint32 stackOffset = tx64LocPhysicalOffset(l);
    m_hhbcTrans->guardTypeStack(stackOffset, JIT::Type::fromRuntimeType(rtt));
  } else {
    if (l.space == Location::Invalid) {
      HHIR_UNIMPLEMENTED(Invalid);
    }
    if (l.space == Location::Iter) {
      HHIR_UNIMPLEMENTED(IterGuard);
    }
    // Convert negative offset to a positive offset for convenience
    m_hhbcTrans->guardTypeLocal(l.offset, JIT::Type::fromRuntimeType(rtt));
  }
  return;
}


void
TranslatorX64::irTranslateBinaryArithOp(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  const Opcode op = i.op();
  switch (op) {
#define CASE(OpBc, x64op)                                          \
    case Op ## OpBc:   HHIR_EMIT(OpBc);
    CASE(Add,    add)
    CASE(Sub,    sub)
    CASE(BitAnd, and)
    CASE(BitOr,  or)
    CASE(BitXor, xor)
    CASE(Mul,    imul)
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
  ASSERT(op == OpSame || op == OpNSame);
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
  ASSERT(op == OpEq || op == OpNeq);
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
  ASSERT(op == OpLt || op == OpLte || op == OpGt || op == OpGte);
  ASSERT(i.inputs.size() == 2);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->outerType() != KindOfRef);
  ASSERT(i.inputs[1]->outerType() != KindOfRef);

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
  ASSERT(op == OpCastBool || op == OpEmptyL);
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
  ASSERT(op == OpJmpZ || op == OpJmpNZ);
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
  ASSERT(op == OpFPassL || OpCGetL);
  const vector<DynLocation*>& inputs = i.inputs;
  ASSERT(inputs.size() == 1);
  ASSERT(inputs[0]->isLocal());

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
  ASSERT(op == OpSetL || op == OpBindL);
  ASSERT(ni.inputs.size() == 2);
  ASSERT((op == OpBindL) ==
         (ni.inputs[rhsIdx]->outerType() == KindOfRef));

  ASSERT(!ni.outStack || ni.inputs[locIdx]->location != ni.outStack->location);
  ASSERT(ni.outLocal);
  ASSERT(ni.inputs[locIdx]->location == ni.outLocal->location);
  ASSERT(ni.inputs[rhsIdx]->isStack());

  if (op == OpSetL) {
    ASSERT(ni.inputs[locIdx]->isLocal());
    HHIR_EMIT(SetL, ni.inputs[locIdx]->location.offset);
  } else {
    ASSERT(op == OpBindL);
    HHIR_EMIT(BindL, ni.inputs[locIdx]->location.offset);
  }
}

void
TranslatorX64::irTranslatePopC(const Tracelet& t,
                             const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(!i.outStack && !i.outLocal);

  if (i.inputs[0]->rtt.isVagueValue()) {
    HHIR_EMIT(PopR);
  } else {
    HHIR_EMIT(PopC);
  }
}

void
TranslatorX64::irTranslatePopV(const Tracelet& t,
                               const NormalizedInstruction& i) {
  ASSERT(i.inputs[0]->rtt.isVagueValue() ||
         i.inputs[0]->isVariant());

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
  ASSERT(i.inputs.size() == 0);
  ASSERT(!i.outLocal);

  HHIR_EMIT(Null);
}

void
TranslatorX64::irTranslateTrue(const Tracelet& t,
                             const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(!i.outLocal);

  HHIR_EMIT(True);
}

void
TranslatorX64::irTranslateFalse(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(!i.outLocal);

  HHIR_EMIT(False);
}

void
TranslatorX64::irTranslateInt(const Tracelet& t,
                            const NormalizedInstruction& i) {
  ASSERT(i.inputs.size()  == 0);
  ASSERT(!i.outLocal);

  HHIR_EMIT(Int, i.imm[0].u_I64A);
}

void
TranslatorX64::irTranslateString(const Tracelet& t,
                               const NormalizedInstruction& i) {
  ASSERT(i.inputs.size()  == 0);
  ASSERT(!i.outLocal);

  HHIR_EMIT(String, (i.imm[0].u_SA));
}

void
TranslatorX64::irTranslateArray(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(!i.outLocal);

  HHIR_EMIT(Array, i.imm[0].u_AA);
}

void
TranslatorX64::irTranslateNewArray(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_EMIT(NewArray);
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
  ASSERT(i.inputs.size() == 2);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->outerType() != KindOfRef);
  ASSERT(i.inputs[1]->outerType() != KindOfRef);
  ASSERT(i.inputs[0]->isStack());
  ASSERT(i.inputs[1]->isStack());

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
  ASSERT(i.inputs.size() == 2);

  if (planInstrAdd_Array(i)) {
    HHIR_EMIT(ArrayAdd);
    return;
  }

  HHIR_UNIMPLEMENTED_WHEN(!planInstrAdd_Int(i), Add);
  ASSERT(planInstrAdd_Int(i));
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
  ASSERT(i.outStack && !i.outLocal);

  HHIR_EMIT(BitNot);
}

void
TranslatorX64::irTranslateCastInt(const Tracelet& t,
                                const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);

  HHIR_EMIT(CastInt);
  /* nop */
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
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA);
}

void
TranslatorX64::irTranslateSwitch(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(Switch);
}

// translateRetC --
//
//   Return to caller with the current activation record replaced with the
//   top-of-stack return value. Call with outputs sync'ed, so the code
//   we're emmitting runs "in between" basic blocks.
void
TranslatorX64::irTranslateRetC(const Tracelet& t,
                             const NormalizedInstruction& i) {
  /**
   * This method chooses one of two ways to generate machine code for RetC
   * depending on whether we are generating a specialized return (where we
   * free the locals inline when possible) or a generic return (where we call
   * a helper function to free locals).
   *
   * For the specialized return, we emit the following flow:
   *
   *   Check if varenv is NULL
   *   If it's not NULL, branch to label 2
   *   Free each local variable
   * 1:
   *   Teleport the return value to appropriate memory location
   *   Restore the old values for rVmFp and rVmSp, and
   *   unconditionally transfer control back to the caller
   * 2:
   *   Call the frame_free_locals helper
   *   Jump to label 1
   *
   * For a generic return, we emit the following flow:
   *
   *   Call the frame_free_locals helper
   *   Teleport the return value to appropriate memory location
   *   Restore the old values for rVmFp and rVmSp, and
   *   unconditionally transfer control back to the caller
   */

  HHIR_EMIT(RetC);
}

void
TranslatorX64::irTranslateRetV(const Tracelet& t,
                             const NormalizedInstruction& i) {
  HHIR_EMIT(RetV);
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
  ASSERT(i.inputs[kEmitClsLocalIdx]->isLocal());
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
  HHIR_UNIMPLEMENTED(CreateCont);
}

void TranslatorX64::irTranslateContEnter(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(ContEnter);
}

void TranslatorX64::irTranslateContExit(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(ContExit);
}

void TranslatorX64::irTranslateUnpackCont(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(UnpackCont);
}

void TranslatorX64::irTranslatePackCont(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(PackCont);
}

void TranslatorX64::irTranslateContReceive(const Tracelet& t,
                                           const NormalizedInstruction& i) {
  HHIR_EMIT(ContReceive);
}

void TranslatorX64::irTranslateContRaised(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  HHIR_EMIT(ContRaised);
}

void TranslatorX64::irTranslateContDone(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_EMIT(ContDone);
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

void TranslatorX64::irTranslateCGetS(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  const int kClassIdx = 0;
  const int kPropNameIdx = 1;
  const Class* cls = i.inputs[kClassIdx]->rtt.valueClass();
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetS, cls, propName,
            getInferredOrPredictedType(i), isInferredType(i));
}

void TranslatorX64::irTranslateSetS(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  const int kClassIdx = 1;
  const int kPropIdx = 2;
  const Class* cls = i.inputs[kClassIdx]->rtt.valueClass();
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(SetS, cls, propName);
}

void TranslatorX64::irTranslateSetG(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(SetG);
}

void
TranslatorX64::irTranslateCGetMProp(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  using namespace TargetCache;
  ASSERT(i.inputs.size() == 2 && i.outStack);

  const DynLocation& prop = *i.inputs[1];
  const Location& propLoc = prop.location;
  const int propOffset    = getNormalPropertyOffset(i,
                              getMInstrInfo(OpCGetM), 1, 0);

  LocationCode locCode = i.immVec.locationCode();
  if (propOffset != -1 && (locCode == LC || locCode == LH)) {
    HHIR_EMIT(CGetProp, locCode, propOffset, propLoc.isStack(),
              getInferredOrPredictedType(i), isInferredType(i));
  } else {
    HHIR_UNIMPLEMENTED(CGetMSlow);
  }
}

void
TranslatorX64::irTranslateCGetM_LEE(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  ASSERT(isSupportedCGetM_LEE(i));

  const DEBUG_ONLY DynLocation& key1  = *i.inputs[1];
  const DEBUG_ONLY DynLocation& key2  = *i.inputs[2];
  ASSERT(key1.isInt() && key2.isString());

  HHIR_UNIMPLEMENTED(CGetM_LEE);
}

void TranslatorX64::irTranslateCGetM_GE(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(CGetM_GE);
}

static bool
isSupportedCGetMProp(const NormalizedInstruction& i) {
  if (i.inputs.size() != 2) return false;
  SKTRACE(2, i.source, "CGetM prop candidate: prop supported: %d, "
                       "in[0] %s in[1] %s\n",
          mcodeMaybePropName(i.immVecM[0]),
          i.inputs[0]->rtt.pretty().c_str(),
          i.inputs[1]->rtt.pretty().c_str());
  return isNormalPropertyAccess(i, 1, 0) && i.immVec.locationCode() != LL;
}

void
TranslatorX64::irTranslateCGetM(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  ASSERT(i.outStack);

  if (isSupportedCGetMProp(i)) {
    irTranslateCGetMProp(t, i);
    return;
  }
  if (isSupportedCGetM_LEE(i)) {
    HHIR_UNIMPLEMENTED(CGetM_LEE);
    irTranslateCGetM_LEE(t, i);
    return;
  }
  if (isSupportedCGetM_GE(i)) {
    HHIR_UNIMPLEMENTED(CGetM_GE);
    irTranslateCGetM_GE(t, i);
    return;
  }
  HHIR_UNIMPLEMENTED(CGetM);
  // Even when basic CGetM is implemented, following may not be
  HHIR_UNIMPLEMENTED_WHEN(!(isSupportedCGetM_LE(i) || isSupportedCGetM_RE(i)), CGetM);
}

void
TranslatorX64::irTranslateVGetG(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetG, name);
}

void
TranslatorX64::irTranslateVGetM(const Tracelet& t,
                                const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(VGetM);

  NOT_REACHED();
}

void
TranslatorX64::irTranslateCGetG(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetG, name, getInferredOrPredictedType(i), isInferredType(i));
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
    HHIR_UNIMPLEMENTED(FPassS);
    ASSERT(false);
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
TranslatorX64::irTranslateIssetM(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(IssetM);
}

void TranslatorX64::irTranslateEmptyM(const Tracelet& t,
                                      const NormalizedInstruction& ni) {
  HHIR_UNIMPLEMENTED(EmptyM);
  // irTranslateEmptyMGeneric(t, ni); // HHIR:TODO:MERGE new translation
}

void
TranslatorX64::irTranslateCheckTypeOp(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  ASSERT(ni.inputs.size() == 1);
  ASSERT(ni.outStack);

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
  HHIR_UNIMPLEMENTED(AKExists);
}

void
TranslatorX64::irTranslateSetMProp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  using namespace TargetCache;
  ASSERT(i.inputs.size() == 3);

  UNUSED const int kRhsIdx       = 0;
  const int kBaseIdx      = 1;
  const int kPropIdx      = 2;
  const DynLocation& base = *i.inputs[kBaseIdx];
  const DynLocation& prop = *i.inputs[kPropIdx];
  const int propOffset    = getNormalPropertyOffset(i, getMInstrInfo(OpSetM),
                                                    kPropIdx, 1);

  const Location& propLoc = prop.location;

  bool fastSet = propOffset != -1 && i.immVec.locationCode() == LC;

  HHIR_UNIMPLEMENTED_WHEN(!fastSet || base.valueType() != KindOfObject,
                          SetMPropSlow);
  HHIR_EMIT(SetProp, propOffset, propLoc.isStack());
}

static bool isSupportedSetMProp(const NormalizedInstruction& i) {
  if (i.inputs.size() != 3) return false;
  SKTRACE(2, i.source, "setM prop candidate: prop supported: %d, rtt %s\n",
          mcodeMaybePropName(i.immVecM[0]),
          i.inputs[2]->rtt.pretty().c_str());
  return isNormalPropertyAccess(i, 2, 1);
}

void
TranslatorX64::irTranslateSetM(const Tracelet& t,
                             const NormalizedInstruction& i) {
  if (isSupportedSetMProp(i)) {
    irTranslateSetMProp(t, i);
    return;
  }

  HHIR_UNIMPLEMENTED(SetM);
}

void
TranslatorX64::irTranslateSetOpL(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(SetOpL);
}

void
TranslatorX64::irTranslateIncDecM(const Tracelet& t,
                                const NormalizedInstruction& i) {
  if (isNormalPropertyAccess(i, 1, 0) && !curFunc()->isPseudoMain()) {
    int offset = getNormalPropertyOffset(i, getMInstrInfo(OpIncDecM), 1, 0);
    if (offset != -1 && i.immVec.locationCode() == LC) {
      const IncDecOp oplet = (IncDecOp) *(i.pc() + 1);
      ASSERT(oplet == PreInc || oplet == PostInc || oplet == PreDec ||
             oplet == PostDec);
      bool post = (oplet == PostInc || oplet == PostDec);
      bool pre  = !post;
      bool inc  = (oplet == PostInc || oplet == PreInc);
      const DynLocation& prop = *i.inputs[1];
      const Location& propLoc = prop.location;
      HHIR_EMIT(IncDecProp, pre, inc, offset, propLoc.isStack());
    } else {
      HHIR_UNIMPLEMENTED(IncDecMPropSlow);
    }
  }
  HHIR_UNIMPLEMENTED(IncDecM);
}

void TranslatorX64::irTranslateSetOpM(const Tracelet& t,
                                      const NormalizedInstruction& ni) {
  HHIR_UNIMPLEMENTED(SetOpM);
  // translateSetOpMGeneric(t, ni); // HHIR:TODO:MERGE new translation
}

void
TranslatorX64::irTranslateIncDecL(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs = i.inputs;
  ASSERT(inputs.size() == 1);
  ASSERT(i.outLocal);
  ASSERT(inputs[0]->isLocal());
  const IncDecOp oplet = IncDecOp(i.imm[1].u_OA);
  ASSERT(oplet == PreInc || oplet == PostInc || oplet == PreDec ||
         oplet == PostDec);
  bool post = (oplet == PostInc || oplet == PostDec);
  bool pre  = !post;
  bool inc  = (oplet == PostInc || oplet == PreInc);

  HHIR_UNIMPLEMENTED_WHEN(!(i.inputs[0]->isInt()), IncDecL_non_int);
  HHIR_EMIT(IncDecL, pre, inc, inputs[0]->location.offset);
}

void
TranslatorX64::irTranslateUnsetL(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetL, i.inputs[0]->location.offset);
}

void
TranslatorX64::irTranslateUnsetM(const Tracelet& t,
                               const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(UnsetM);
}

void TranslatorX64::irTranslateBindM(const Tracelet& t,
                                     const NormalizedInstruction& ni) {
  HHIR_UNIMPLEMENTED(BindM);
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

void
TranslatorX64::irTranslateReqMod(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(ReqMod, name);
}

void
TranslatorX64::irTranslateReqSrc(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(ReqSrc, name);
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
  using namespace TargetCache;
  const StringData* meth = curUnit()->lookupLitstrId(i.imm[1].u_SA);
  const NamedEntityPair& np = curUnit()->lookupNamedEntityPairId(i.imm[2].u_SA);
  DEBUG_ONLY const StringData* cls = np.first;
  ASSERT(meth && meth->isStatic() &&
         cls && cls->isStatic());
  ASSERT(i.inputs.size() == 0);

  const Class* baseClass = Unit::lookupClass(np.second);
  bool magicCall = false;
  const Func* func = lookupImmutableMethod(baseClass, meth, magicCall,
                                           true /* staticLookup */);

  bool mightNotBeStatic = false;
  if (func &&
      !(func->attrs() & AttrStatic) &&
      !(curFunc()->attrs() & AttrStatic) &&
      curFunc()->cls() &&
      curFunc()->cls()->classof(baseClass)) {
    mightNotBeStatic = true;
  }

  HHIR_EMIT(FPushClsMethodD,
             (i.imm[0].u_IVA),
             (i.imm[1].u_SA),
             (i.imm[2].u_SA),
             (mightNotBeStatic));
}

void
TranslatorX64::irTranslateFPushClsMethodF(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(FPushClsMethodF);
}

void
TranslatorX64::irTranslateFPushObjMethodD(const Tracelet &t,
                                        const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  HHIR_UNIMPLEMENTED_WHEN((i.inputs[0]->valueType() != KindOfObject),
                          FPushObjMethod_nonObj);
  ASSERT(i.inputs[0]->valueType() == KindOfObject);
  int id = i.imm[1].u_IVA;
  UNUSED const StringData* name = curUnit()->lookupLitstrId(id);
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

// static void fatalNullThis() { raise_error(Strings::FATAL_NULL_THIS); }

void
TranslatorX64::irTranslateThis(const Tracelet &t,
                             const NormalizedInstruction &i) {
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(curFunc()->isPseudoMain() || curFunc()->cls());

  HHIR_EMIT(This);
}

void
TranslatorX64::irTranslateBareThis(const Tracelet &t,
                                  const NormalizedInstruction &i) {
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(curFunc()->isPseudoMain() || curFunc()->cls());

  HHIR_UNIMPLEMENTED(BareThis);
}

void
TranslatorX64::irTranslateCheckThis(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(CheckThis);
}

void
TranslatorX64::irTranslateInitThisLoc(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  ASSERT(i.outLocal && !i.outStack);
  ASSERT(curFunc()->isPseudoMain() || curFunc()->cls());

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
TranslatorX64::irTranslateFPassM(const Tracelet& t,
                               const NormalizedInstruction& i) {

  ASSERT(i.inputs.size() >= 1);
  ASSERT(i.outStack && !i.outLocal);
  HHIR_UNIMPLEMENTED_WHEN((i.preppedByRef), FPassM);
  irTranslateCGetM(t, i);
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
TranslatorX64::irTranslateFCall(const Tracelet& t,
                              const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  const Opcode* after = curUnit()->at(nextSrcKey(t, i).offset());
  const Func* srcFunc = curFunc();

  int32 callOffsetInUnit =
    srcFunc->unit()->offsetOf(after - srcFunc->base());
  if (i.funcd) {
    // add ni.source?
    HHIR_EMIT(FCallD, numArgs, i.funcd, callOffsetInUnit);
  } else {
    HHIR_EMIT(FCall, numArgs, callOffsetInUnit);
  }
}

void
TranslatorX64::irTranslateFCallArray(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(FCallArray);
}

void
TranslatorX64::irTranslateNewTuple(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  // todo: test
  int numArgs = i.imm[0].u_IVA;
  HHIR_EMIT(NewTuple, numArgs);
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
  const TypeConstraint& tc = curFunc()->params()[param].typeConstraint();

  // not quite a nop. The guards should have verified that the m_type field
  // is compatible, but for objects we need to go one step further and
  // ensure that we're dealing with the right class.
  // NULL inputs only get traced when constraint is nullable.
  ASSERT(i.inputs.size() == 1);
  if (!i.inputs[0]->isObject()) {
    HHIR_UNIMPLEMENTED_WHEN(i.m_txFlags == Interp, VerifyParamType);
    return; // nop.
  }

  bool isSelf   = tc.isSelf();
  bool isParent = tc.isParent();
  const Class *constraint = NULL;

  UNUSED TargetCache::CacheHandle ch = 0;
  if (isSelf) {
    tc.selfToClass(curFunc(), &constraint);
  } else if (isParent) {
    tc.parentToClass(curFunc(), &constraint);
  } else {
    const StringData* clsName = tc.typeName();
    ch = TargetCache::allocKnownClass(clsName);
  }

  HHIR_EMIT(VerifyParamType, param, (constraint ? constraint->name() : NULL));
}

void
TranslatorX64::irTranslateInstanceOfD(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  HHIR_EMIT(InstanceOfD, (i.imm[0].u_SA));
}

void
TranslatorX64::irTranslateIterInit(const Tracelet& t,
                                 const NormalizedInstruction& i) {

  HHIR_EMIT(IterInit, i.imm[0].u_IVA, i.imm[1].u_BA);
}

void
TranslatorX64::irTranslateIterValueC(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  HHIR_EMIT(IterValueC, i.imm[0].u_IVA);
}

void
TranslatorX64::irTranslateIterKey(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  HHIR_EMIT(IterKey, i.imm[0].u_IVA);
}

void
TranslatorX64::irTranslateIterNext(const Tracelet& t,
                                 const NormalizedInstruction& i) {

  HHIR_EMIT(IterNext, i.imm[0].u_IVA, i.imm[1].u_BA);
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
    case OpUnsetN:
      m_hhbcTrans->emitUnsetN();
      break;
    case OpUnsetG:
      m_hhbcTrans->emitUnsetG();
      break;
    default:
      // GO: if you hit this, check opNames[op] and add support for it
      HHIR_UNIMPLEMENTED_OP(opNames[op]);
      ASSERT(false);
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
TranslatorX64::irTranslateInstr(const Tracelet& t,
                                const NormalizedInstruction& i) {
  /**
   * irTranslateInstr() translates an individual instruction in a tracelet,
   * either by directly emitting machine code for that instruction or by
   * emitting a call to the interpreter. (emitInterpOne not yet supported)
   *
   * If the instruction ends the current tracelet, we must emit machine code
   * to transfer control to some target that will continue to make forward
   * progress. This target may be the beginning of another tracelet, or it may
   * be a translator service request. Before transferring control, a tracelet
   * must ensure the following invariants hold:
   *   1) The machine registers rVmFp and rVmSp are in sync with vmfp()
   *      and vmsp().
   *   2) All "dirty" values are synced in memory. This includes the
   *      evaluation stack, locals, globals, statics, and any other program
   *      accessible locations. This also means that all refcounts must be
   *      up to date.
   */
  ASSERT(m_useHHIR);
  ASSERT(!i.outStack || i.outStack->isStack());
  ASSERT(!i.outLocal || i.outLocal->isLocal());
  const char *opNames[] = {
#define O(name, imm, push, pop, flags) \
#name,
  OPCODES
#undef O
  };
  SpaceRecorder sr(opNames[i.op()], a);
  SKTRACE(1, i.source, "translate %#lx\n", long(a.code.frontier));
  const Opcode op = i.op();

  m_hhbcTrans->setBcOff(i.source.offset(), i.breaksTracelet);

  if (!i.grouped) {
    emitVariantGuards(t, i);
    const NormalizedInstruction* n = &i;
    while (n->next && n->next->grouped) {
      n = n->next;
      emitVariantGuards(t, *n);
    }
  }

  if (i.guardedThis) {
    m_hhbcTrans->setThisAvailable();
  }

  // Actually translate the instruction's body.
  Stats::emitIncTranslOp(a, op);

  irTranslateInstrWork(t, i);

  emitPredictionGuards(i);
}

void TranslatorX64::irAssertType(const Location& l,
                                 const RuntimeType& rtt) {
  ASSERT(m_useHHIR);
  if (rtt.isVagueValue()) return;

  switch (l.space) {
    case Location::Stack: {
      // tx64LocPhysicalOffset returns positive offsets for stack values,
      // relative to rVmSp
      uint32 stackOffset = tx64LocPhysicalOffset(l);
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
      HHIR_UNIMPLEMENTED(Iter);
      break;

    case Location::Litstr:            // Literal string pseudo-location
      HHIR_UNIMPLEMENTED(Litstr);
      break;

    case Location::Litint:            // Literal int pseudo-location
      HHIR_UNIMPLEMENTED(Litint);
      break;

    case Location::This:
      HHIR_UNIMPLEMENTED(This);
      break;
  }
}

void TranslatorX64::irEmitResolvedDeps(const ChangeMap& resolvedDeps) {
  for (DepMap::const_iterator dep = resolvedDeps.begin();
       dep != resolvedDeps.end(); dep++) {
    irAssertType(dep->first, dep->second->rtt);
  }
}

bool
TranslatorX64::irTranslateTracelet(const Tracelet& t,
                                   const TCA       start,
                                   const TCA       stubStart) {
  bool hhirSucceeded = false;
  if (!m_useHHIR) {
    return hhirSucceeded;
  }

  const SrcKey &sk = t.m_sk;
  SrcRec&                 srcRec = *getSrcRec(sk);
  vector<TransBCMapping>  bcMapping;
  ASSERT(srcRec.inProgressTailJumps().size() == 0);
  try {
    // Don't translate if we have already reached the maximum # of
    // translations for this tracelet
    HHIR_UNIMPLEMENTED_WHEN(checkTranslationLimit(t.m_sk, srcRec),
                            TOO_MANY_TRANSLATIONS);

    irEmitResolvedDeps(t.m_resolvedDeps);
    emitGuardChecks(a, sk, t.m_dependencies, t.m_refDeps, srcRec);

    dumpTranslationInfo(t, a.code.frontier);

    emitRB(a, RBTypeTraceletBody, t.m_sk);
    Stats::emitInc(a, Stats::Instr_TC, t.m_numOpcodes);
    recordBCInstr(OpTraceletGuard, a, start);
    m_hhbcTrans->setBcOffNextTrace(t.m_nextSk.offset());

    // Translate each instruction in the tracelet
    for (NormalizedInstruction* ni = t.m_instrStream.first; ni;
         ni = ni->next) {
      if (isTransDBEnabled()) {
        bcMapping.push_back((TransBCMapping){ni->offset(),
                                             a.code.frontier,
                                             astubs.code.frontier});
      }

      irTranslateInstr(t, *ni);
      ASSERT(ni->source.offset() >= curFunc()->base());
      // We sometimes leave the tail of a truncated tracelet in place to aid
      // analysis, but breaksTracelet is authoritative.
      if (ni->breaksTracelet) break;
    }

    hhirTraceEnd(t.m_nextSk.offset());
    hhirTraceCodeGen();

    hhirSucceeded = true;
    TRACE(1, "HHIR: SUCCEEDED to generate code for Translation %d\n",
          getCurrentTransID());
  } catch (JIT::FailedCodeGen& fcg) {
    hhirSucceeded = false;
    TRACE(1, "HHIR: FAILED to generate code for Translation %d "
          "@ %s:%d (%s)\n", getCurrentTransID(),
          fcg.file, fcg.line, fcg.func);
    // HHIR:TODO Remove extra TRACE and adjust tools
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          fcg.file, fcg.line, fcg.func);
  } catch (JIT::FailedIRGen& x) {
    hhirSucceeded = false;
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          x.file, x.line, x.func);
  } catch (TranslationFailedExc& tfe) {
    assert(0);
  }

  if (!hhirSucceeded) {
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

  return hhirSucceeded;
}

void TranslatorX64::hhirTraceStart(Offset bcStartOffset) {
  ASSERT(!m_irFactory);
  m_useHHIR      = true;
  m_irFactory    = new JIT::IRFactory();
  m_constTable   = new JIT::CSEHash();
  m_traceBuilder = new JIT::TraceBuilder(bcStartOffset,
                                         *m_irFactory,
                                         *m_constTable,
                                         curFunc());
  m_hhbcTrans    = new JIT::HhbcTranslator(*m_traceBuilder, curFunc());
  Cell* fp = vmfp();
  if (curFunc()->isGenerator()) {
    fp = (Cell*)Stack::generatorStackBase((ActRec*)fp);
  }
  TRACE(1, "hhirTraceStart: bcStartOffset %d   vmfp() - vmsp() = %ld\n",
        bcStartOffset, fp - vmsp());
  m_hhbcTrans->start(bcStartOffset, (fp - vmsp()));
}

void TranslatorX64::hhirTraceEnd(Offset bcSuccOffset) {
  ASSERT(m_useHHIR);
  m_hhbcTrans->end(bcSuccOffset);
}

void TranslatorX64::hhirTraceCodeGen() {
  ASSERT(m_useHHIR);

  m_traceBuilder->finalizeTrace();
  JIT::Trace* trace = m_traceBuilder->getTrace();

  if (RuntimeOption::EvalDumpIR) {
    std::cout << "--------- HHIR before code gen ---------\n";
    trace->print(std::cout, false);
    std::cout << "----------------------------------------\n";
  }

  JIT::optimizeTrace(trace, m_irFactory);

  if (RuntimeOption::EvalDumpIR > 1) {
    std::cout << "--------- HHIR after optimizing ---------\n";
    trace->print(std::cout, false);
    std::cout << "----------------------------------------\n";
  }

  JIT::assignRegsForTrace(trace, m_irFactory, m_traceBuilder);

  if (RuntimeOption::EvalDumpIR) {
    std::cout << "--------- HHIR after reg alloc ---------\n";
    trace->print(std::cout, false);
    std::cout << "----------------------------------------\n";
  }

  JIT::genCodeForTrace(trace, a, astubs, m_irFactory, this);

  if (RuntimeOption::EvalDumpIR) {
    std::cout << "--------- HHIR after code gen ---------\n";
    trace->print(std::cout, true);
    std::cout << "---------------------------------------\n";
  }

  m_numHHIRTrans++;
  hhirTraceFree();
}

void TranslatorX64::hhirTraceFree() {
  // Free data structures
  m_useHHIR = false;
  delete m_irFactory;    m_irFactory = NULL;
  delete m_constTable;   m_constTable = NULL;
  delete m_traceBuilder; m_traceBuilder = NULL;
  delete m_hhbcTrans;    m_hhbcTrans = NULL;
}


} // HPHP::VM::Transl

static const Trace::Module TRACEMOD = Trace::tx64;


} } // HPHP::VM
