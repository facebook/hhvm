/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/mcgen-prologue.h"

#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"

#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

/*
 * Given a callee and a number of args, match up to the callee's
 * argument expectations and dispatch.
 *
 * Call/return hand-shaking is a bit funny initially. At translation time,
 * we don't necessarily know what function we're calling. For instance,
 *
 *   f(g());
 *
 * Will lead to a set of basic blocks like:
 *
 * b1: pushfuncd "f"
 *     pushfuncd "g"
 *     fcall
 * b2: fcall
 *
 * The fcall labelled "b2" above may not be statically bindable in our
 * execution model.
 *
 * We decouple the call work into a per-callsite portion, responsible
 * for recording the return address, and a per-(callee, numArgs) portion,
 * responsible for fixing up arguments and dispatching to remaining
 * code. We call the per-callee portion a "prologue."
 *
 * Also, we are called from two distinct environments. From REQ_BIND_CALL,
 * we're running "between" basic blocks, with all VM registers sync'ed.
 * However, we're also called in the middle of basic blocks, when dropping
 * entries into func->m_prologues. So don't go around using the
 * translation-time values of vmfp()/vmsp(), since they have an
 * unpredictable relationship to the source.
 */
namespace HPHP { namespace jit { namespace mcgen {

namespace {

TCA checkCachedPrologue(const Func* func, int paramIdx) {
  TCA prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue != tc::ustubs().fcallHelperThunk) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assertx(tc::isValidCodeAddress(prologue));
    return prologue;
  }
  return nullptr;
}

/**
 * Given the proflogueTransId for a TransProflogue translation, regenerate the
 * prologue (as a TransPrologue).  Returns the starting address for the
 * translation corresponding to triggerSk, if such translation is generated;
 * otherwise returns nullptr.
 */
TCA regeneratePrologue(TransID prologueTransId, SrcKey triggerSk,
                       bool& emittedDVInit) {
  auto rec = profData()->transRec(prologueTransId);
  auto func = rec->func();
  auto nArgs = rec->prologueArgs();
  emittedDVInit = false;

  func->resetPrologue(nArgs);

  // If we're regenerating a prologue, and we want to check shouldTranslate()
  // but ignore the code size limits.  We still want to respect the global
  // translation limit and other restrictions, though.
  if (!tc::shouldTranslateNoSizeLimit(func)) return nullptr;

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (auto const p = checkCachedPrologue(func, nArgs)) return p;

  // If this prologue has a DV funclet, then invalidate it and return its SrcKey
  // and TransID
  if (nArgs < func->numNonVariadicParams()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      SrcKey funcletSK(func, paramInfo.funcletOff, false);
      auto funcletTransId = profData()->dvFuncletTransId(func, nArgs);
      if (funcletTransId != kInvalidTransID) {
        tc::invalidateSrcKey(funcletSK);
        auto args = TransArgs{funcletSK};
        args.transId = funcletTransId;
        args.kind = TransKind::Optimize;
        args.region = selectHotRegion(funcletTransId);
        auto const spOff = args.region->entry()->initialSpOffset();
        auto dvStart = translate(args, spOff, rec);
        emittedDVInit |= dvStart != nullptr;

        // Flag that this translation has been retranslated, so that
        // it's not retranslated again along with the function body.
        profData()->setOptimized(funcletSK);
        return dvStart && funcletSK == triggerSk ? dvStart : nullptr;
      }
    }
  }

  tc::emitFuncPrologueOpt(rec);
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
}

TCA regeneratePrologues(Func* func, SrcKey triggerSk, bool& includedBody) {
  TCA triggerStart = nullptr;
  std::vector<TransID> prologTransIDs;

  VMProtect _;

  for (int nArgs = 0; nArgs < func->numPrologues(); nArgs++) {
    TransID tid = profData()->proflogueTransId(func, nArgs);
    if (tid != kInvalidTransID) {
      prologTransIDs.push_back(tid);
    }
  }

  std::sort(prologTransIDs.begin(), prologTransIDs.end(),
          [&](TransID t1, TransID t2) -> bool {
            // This will sort in ascending order.
            return profData()->transCounter(t2) >
                   profData()->transCounter(t1);
          });

  // Next, we're going to regenerate each prologue along with its DV funclet.
  // We consider the option of either including the DV funclets in the same
  // region as the function body or not.  Including them in the same region
  // enables some type information to flow and thus can eliminate some stores
  // and type checks, but it can also increase the code size by duplicating the
  // whole function body.  Therefore, we only include the function body along
  // with the DV init if both (a) the function has a single proflogue, and (b)
  // the size of the function is within a certain threshold.
  //
  // The mechanism used to keep the function body separate from the DV init is
  // to temporarily mark the SrcKey for the function body as already optimized.
  // (The region selectors break a region whenever they hit a SrcKey that has
  // already been optimized.)
  SrcKey funcBodySk(func, func->base(), false);
  includedBody = prologTransIDs.size() <= 1 &&
    func->past() - func->base() <= RuntimeOption::EvalJitPGOMaxFuncSizeDupBody;

  if (!includedBody) profData()->setOptimized(funcBodySk);
  SCOPE_EXIT{ profData()->clearOptimized(funcBodySk); };

  bool emittedAnyDVInit = false;
  for (TransID tid : prologTransIDs) {
    bool emittedDVInit = false;
    TCA start = regeneratePrologue(tid, triggerSk, emittedDVInit);
    if (triggerStart == nullptr && start != nullptr) {
      triggerStart = start;
    }
    emittedAnyDVInit |= emittedDVInit;
  }

  // If we tried to include the function body along with a DV init, but didn't
  // end up generating any DV init, then flag that the function body was not
  // included.
  if (!emittedAnyDVInit) includedBody = false;

  return triggerStart;
}

TCA getFuncPrologue(Func* func, int nPassed) {
  VMProtect _;

  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int const numParams = func->numNonVariadicParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  // Do a quick test before grabbing the write lease
  if (auto const p = checkCachedPrologue(func, paramIndex)) return p;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(nPassed), false};
  auto computeKind = [&] {
    return tc::profileSrcKey(funcBody) ? TransKind::ProfPrologue :
                                         TransKind::LivePrologue;
  };
  LeaseHolder writer(func, computeKind());
  if (!writer) return nullptr;

  auto const kind = computeKind();
  // Check again now that we have the write lease, in case the answer to
  // profileSrcKey() changed.
  if (!writer.checkKind(kind)) return nullptr;

  if (!tc::shouldTranslate(func, TransKind::LivePrologue)) return nullptr;

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (auto const p = checkCachedPrologue(func, paramIndex)) return p;

  return tc::emitFuncPrologue(func, nPassed, kind);
}

}}}
