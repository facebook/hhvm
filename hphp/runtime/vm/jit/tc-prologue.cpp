/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/tc-prologue.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc-region.h"

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/func-order.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/util/configs/jit.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP::jit::tc {
namespace {
/*
 * Smash the callers of the ProfPrologue associated with `rec' to call a new
 * prologue at `start' address.
 */
void smashFuncCallers(TCA start, ProfTransRec* rec) {
  assertOwnsMetadataLock();
  assertx(rec->isProflogue());

  auto lock = rec->lockCallerList();

  for (auto toSmash : rec->mainCallers()) {
    smashCall(toSmash, start);
  }

  rec->clearAllCallers();
}
}
////////////////////////////////////////////////////////////////////////////////

void PrologueTranslator::computeKind() {
  // Update the translation kind if it is invalid, or if it may
  // have changed (original kind was a profililng kind)
  if (kind == TransKind::Invalid || kind == TransKind::ProfPrologue) {
    kind = profileFunc(func) ? TransKind::ProfPrologue
                             : TransKind::LivePrologue;
  }
}

uint32_t PrologueTranslator::paramIndex() const {
  return paramIndexHelper(func, nPassed);
}

uint32_t PrologueTranslator::paramIndexHelper(const Func* f, uint32_t passed) {
  auto const numParams = f->numNonVariadicParams();
  return passed <= numParams ? passed : numParams + 1;
}

Optional<TranslationResult> PrologueTranslator::getCached() {
  if (UNLIKELY(RuntimeOption::EvalFailJitPrologs)) {
    return TranslationResult::failTransiently();
  }

  auto const paramIdx = paramIndex();
  TCA prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue == tc::ustubs().fcallHelperNoTranslateThunk) {
    return TranslationResult::failForProcess();
  }
  if (prologue != ustubs().fcallHelperThunk) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assertx(isValidCodeAddress(prologue));
    return TranslationResult{prologue};
  }
  return std::nullopt;
}

void PrologueTranslator::resetCached() {
  func->resetPrologue(paramIndex());
}

void PrologueTranslator::setCachedForProcessFail() {
  TRACE(2, "funcPrologue %s(%u) setting prologue %p\n",
        func->fullName()->data(), nPassed,
        tc::ustubs().fcallHelperNoTranslateThunk);
  func->setPrologue(paramIndex(), tc::ustubs().fcallHelperNoTranslateThunk);
}

void PrologueTranslator::smashBackup() {
  if (kind == TransKind::OptPrologue) {
    assertx(proflogueTransId != kInvalidTransID);
    auto const rec = profData()->transRec(proflogueTransId);
    smashFuncCallers(jit::tc::ustubs().fcallHelperThunk, rec);
  }
}

void PrologueTranslator::gen() {
  if (profData() && isProfiling(kind)) {
    assertx(transId != kInvalidTransID);
    profData()->addTransProfPrologue(transId, sk, paramIndex(),
      0 /* asmSize: updated below after machine code is generated */);
  }
  auto const context = TransContext{
    transId == kInvalidTransID ? TransIDSet{} : TransIDSet{transId},
    0,  // optIndex
    kind,
    sk,
    nullptr,
    sk.packageInfo(),
    PrologueID(func, nPassed),
  };
  tracing::Block _b{
    kind == TransKind::OptPrologue ? "emit-func-prologue-opt"
                                   : "emit-func-prologue",
      [&] {
        return traceProps(func)
          .add("sk", show(sk))
          .add("argc", paramIndex())
          .add("trans_kind", show(kind));
      }
  };
  tracing::Pause _p;

  unit = std::make_unique<IRUnit>(context, std::make_unique<AnnotationData>());
  irgen::IRGS env{*unit, nullptr, 0, nullptr};

  irgen::emitFuncPrologue(env, func, nPassed, transId);
  irgen::sealUnit(env);
  optimize(*unit, kind);

  printUnit(2, *unit, "After initial prologue generation");

  vunit = irlower::lowerUnit(env.unit, CodeKind::Prologue);
}

void PrologueTranslator::publishMetaImpl() {
  // Update the profiling prologue size now that we generated it.
  if (isProfiling(kind)) {
    profData()->transRec(transId)->setAsmSize(
      transMeta->range.loc().mainSize());
  }

  assertOwnsMetadataLock();
  assertx(translateSuccess());
  assertx(code().isValidCodeAddress(entry()));

  transMeta->fixups.process(nullptr);

  auto const& loc = transMeta->range.loc();
  TransRec tr{sk, transId, kind, loc.mainStart(), loc.mainSize(),
              loc.coldCodeStart(), loc.coldCodeSize(),
              loc.frozenCodeStart(), loc.frozenCodeSize(),
              std::move(annotations)};
  transdb::addTranslation(tr);
  FuncOrder::recordTranslation(tr);
  if (Cfg::Jit::UseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }
  recordTranslationSizes(tr);
  recordGdbTranslation(sk, transMeta->view.main(), loc.mainStart(),
                       loc.mainEnd());
  recordGdbTranslation(sk, transMeta->view.cold(), loc.coldStart(),
                       loc.coldEnd());
  recordBCInstr(OpFuncPrologue, loc.mainStart(), loc.mainEnd(), false);
}

void PrologueTranslator::publishCodeImpl() {
  assertOwnsMetadataLock();
  assertOwnsCodeLock();

  if (RuntimeOption::EvalEnableReusableTC) {
    auto const& loc = transMeta->range.loc();
    recordFuncPrologue(func, loc);
  }

  const auto start = entry();
  assertx(start);
  TRACE(2, "funcPrologue %s(%u) setting prologue %p\n",
        func->fullName()->data(), nPassed, start);
  func->setPrologue(paramIndex(), start);

  // If we are optimizing, smash the callers of the proflogue.
  if (kind == TransKind::OptPrologue) {
    assertx(proflogueTransId != kInvalidTransID);
    auto const rec = profData()->transRec(proflogueTransId);
    smashFuncCallers(start, rec);
  }
}

}
