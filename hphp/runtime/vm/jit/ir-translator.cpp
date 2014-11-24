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
#include <functional>
#include "hphp/runtime/base/strings.h"

#include <folly/Format.h>
#include <folly/Conv.h>
#include "hphp/util/trace.h"
#include "hphp/util/stack-trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/bc-pattern.h"
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
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/print.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP { namespace jit {

using namespace reg;
using namespace Trace;
using std::max;

TRACE_SET_MOD(hhir);

#define HHIR_EMIT(op, ...)                      \
  do {                                          \
    m_hhbcTrans.emit ## op(__VA_ARGS__);        \
    return;                                     \
  } while (0)

IRTranslator::IRTranslator(TransContext context)
  : m_hhbcTrans{context}
{}

bool
IRTranslator::tryTranslateSingletonInline(const NormalizedInstruction& i,
                                          const Func* funcd) {
  using Atom = BCPattern::Atom;
  using Captures = BCPattern::CaptureVec;

  if (!funcd) return false;

  // Make sure we have an acceptable FPush and non-null callee.
  assert(i.op() == Op::FPushFuncD ||
         i.op() == Op::FPushClsMethodD);

  auto fcall = i.nextSk();

  // Check if the next instruction is an acceptable FCall.
  if ((fcall.op() != Op::FCall && fcall.op() != Op::FCallD) ||
      funcd->isResumable() || funcd->isReturnRef()) {
    return false;
  }

  // First, check for the static local singleton pattern...

  // Lambda to check if CGetL and StaticLocInit refer to the same local.
  auto has_same_local = [] (PC pc, const Captures& captures) {
    if (captures.size() == 0) return false;

    auto cgetl = (const Op*)pc;
    auto sli = (const Op*)captures[0];

    assert(*cgetl == Op::CGetL);
    assert(*sli == Op::StaticLocInit);

    return (getImm(sli, 0).u_IVA == getImm(cgetl, 0).u_IVA);
  };

  auto cgetl = Atom(Op::CGetL).onlyif(has_same_local);
  auto retc  = Atom(Op::RetC);

  // Look for a static local singleton pattern.
  auto result = BCPattern {
    Atom(Op::Null),
    Atom(Op::StaticLocInit).capture(),
    Atom(Op::IsTypeL),
    Atom::alt(
      Atom(Op::JmpZ).taken({cgetl, retc}),
      Atom::seq(Atom(Op::JmpNZ), cgetl, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      hhbcTrans().emitSingletonSLoc(
        funcd,
        (const Op*)result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sloc] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  // Not found; check for the static property pattern.

  // Factory for String atoms that are required to match another captured
  // String opcode.
  auto same_string_as = [&] (int i) {
    return Atom(Op::String).onlyif([=] (PC pc, const Captures& captures) {
      auto string1 = (const Op*)pc;
      auto string2 = (const Op*)captures[i];
      assert(*string1 == Op::String);
      assert(*string2 == Op::String);

      auto const unit = funcd->unit();
      auto sd1 = unit->lookupLitstrId(getImmPtr(string1, 0)->u_SA);
      auto sd2 = unit->lookupLitstrId(getImmPtr(string2, 0)->u_SA);

      return (sd1 && sd1 == sd2);
    });
  };

  auto stringProp = same_string_as(0);
  auto stringCls  = same_string_as(1);
  auto agetc = Atom(Op::AGetC);
  auto cgets = Atom(Op::CGetS);

  // Look for a class static singleton pattern.
  result = BCPattern {
    Atom(Op::String).capture(),
    Atom(Op::String).capture(),
    Atom(Op::AGetC),
    Atom(Op::CGetS),
    Atom(Op::IsTypeC),
    Atom::alt(
      Atom(Op::JmpZ).taken({stringProp, stringCls, agetc, cgets, retc}),
      Atom::seq(Atom(Op::JmpNZ), stringProp, stringCls, agetc, cgets, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      hhbcTrans().emitSingletonSProp(
        funcd,
        (const Op*)result.getCapture(1),
        (const Op*)result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sprop] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  return false;
}

/*
 * Generate HhbcTranslator method callers for all bytecodes, using its
 * table-defined signature.
 */

/*
 * TODO: turn SA into const StringData* automatically?
 */

#define IMM_MA(n)      0 /* ignored, but we need something (for commas) */
#define IMM_BLA(n)     ni.immVec
#define IMM_SLA(n)     ni.immVec
#define IMM_ILA(n)     ni.immVec
#define IMM_VSA(n)     ni.immVec
#define IMM_IVA(n)     ni.imm[n].u_IVA
#define IMM_I64A(n)    ni.imm[n].u_I64A
#define IMM_LA(n)      ni.imm[n].u_LA
#define IMM_IA(n)      ni.imm[n].u_IA
#define IMM_DA(n)      ni.imm[n].u_DA
#define IMM_SA(n)      ni.imm[n].u_SA
#define IMM_RATA(n)    ni.imm[n].u_RATA
#define IMM_AA(n)      ni.imm[n].u_AA
#define IMM_BA(n)      ni.imm[n].u_BA
#define IMM_OA_IMPL(n) ni.imm[n].u_OA
#define IMM_OA(subop)  (subop)IMM_OA_IMPL

#define ONE(x0)              IMM_##x0(0)
#define TWO(x0, x1)          IMM_##x0(0), IMM_##x1(1)
#define THREE(x0, x1, x2)    IMM_##x0(0), IMM_##x1(1), IMM_##x2(2)
#define FOUR(x0, x1, x2, x3) IMM_##x0(0), IMM_##x1(1), IMM_##x2(2), IMM_##x3(3)
#define NA                   /*  */

/*
 * The conversion to HT& is to make it so that the emit##nm call is a dependent
 * name, and not checked until the template is instantiated (which will fail if
 * supports##nm is not true).
 */
#define O(nm, imms, pop, push, flags)                                   \
  void                                                                  \
  IRTranslator::unpack##nm(std::nullptr_t, const NormalizedInstruction& ni) { \
    m_hhbcTrans.emit##nm(imms);                                         \
  }

OPCODES
#undef O

#undef FOUR
#undef THREE
#undef TWO
#undef ONE
#undef NA

#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA

void IRTranslator::translateInstrWork(const NormalizedInstruction& i) {
  auto const op = i.op();
#define O(name, ...) case Op::name: return unpack ## name(nullptr, i);
  switch (op) { OPCODES }
#undef O
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
  ht.setBcOff(&ni,
              ni.source.offset(),
              ni.endsRegion && !m_hhbcTrans.isInlining());
  FTRACE(1, "\n{:-^60}\n", folly::format("Translating {}: {} with stack:\n{}",
                                         ni.offset(), ni.toString(),
                                         ht.showStack()));
  // When profiling, we disable type predictions to avoid side exits
  assert(IMPLIES(mcg->tx().mode() == TransKind::Profile, !ni.outputPredicted));

  ht.emitRB(RBTypeBytecodeStart, ni.source, 2);
  ht.emitIncStat(Stats::Instr_TC, 1);

  auto pc = reinterpret_cast<const Op*>(ni.pc());
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type = flavorToType(instrInputFlavor(pc, i));
    if (type != Type::Gen) m_hhbcTrans.assertTypeStack(i, type);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts >= 2) {
    ht.emitDbgAssertRetAddr();
  }

  if (isAlwaysNop(ni.op())) {
    // Do nothing
  } else if (ni.interp) {
    m_hhbcTrans.emitInterpOne(ni);
  } else {
    translateInstrWork(ni);
  }
}

}}
