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

#include "hphp/runtime/vm/jit/mcgen-prologue.h"

#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"

#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/resumable.h"

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

template <class F>
void withThis(const Func* func, F f) {
  if (!func->hasThisVaries()) {
    f(func->mayHaveThis());
    return;
  }
  f(true);
  f(false);
}

/**
 * Given the proflogueTransId for a TransProflogue translation, regenerate the
 * prologue (as a TransPrologue).
 *
 * Returns true iff the prologue had an associated dvInit funclet that was
 * successfully retranslated as an Optimize translation.
 */
bool regeneratePrologue(TransID prologueTransId, tc::FuncMetaInfo& info) {
  auto rec = profData()->transRec(prologueTransId);
  auto func = rec->func();
  auto nArgs = rec->prologueArgs();

  func->resetPrologue(nArgs);

  // If we're regenerating a prologue, and we want to check shouldTranslate()
  // but ignore the code size limits.  We still want to respect the global
  // translation limit and other restrictions, though.
  if (!tc::shouldTranslateNoSizeLimit(func, TransKind::OptPrologue)) {
    return false;
  }

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, nArgs)) return false;

  if (retranslateAllEnabled()) {
    info.prologues.emplace_back(rec);
  } else {
    tc::emitFuncPrologueOpt(rec);
  }

  // If this prologue has a DV funclet, then invalidate it and return its SrcKey
  // and TransID
  if (nArgs < func->numNonVariadicParams()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      bool ret = false;
      auto genPrologue = [&] (bool hasThis) {
        SrcKey funcletSK(func, paramInfo.funcletOff, ResumeMode::None, hasThis);
        if (profData()->optimized(funcletSK)) return;
        auto funcletTransId = profData()->dvFuncletTransId(funcletSK);
        if (funcletTransId == kInvalidTransID) return;
        auto args = TransArgs{funcletSK};
        args.transId = funcletTransId;
        args.kind = TransKind::Optimize;
        args.region = selectHotRegion(funcletTransId);
        auto const spOff = args.region->entry()->initialSpOffset();
        if (auto res = translate(args, spOff, info.tcBuf.view())) {
          // Flag that this translation has been retranslated, so that
          // it's not retranslated again along with the function body.
          profData()->setOptimized(funcletSK);
          ret = true;
          info.translations.emplace_back(std::move(res.value()));
        }
      };
      withThis(func, genPrologue);
      if (ret) return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
}

bool regeneratePrologues(Func* func, tc::FuncMetaInfo& info) {
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
  auto const includedBody = prologTransIDs.size() <= 1 &&
    func->past() - func->base() <= RuntimeOption::EvalJitPGOMaxFuncSizeDupBody;

  auto funcBodySk = [&] (bool hasThis) {
    return SrcKey{func, func->base(), ResumeMode::None, hasThis};
  };
  if (!includedBody) {
    withThis(func,
            [&] (bool hasThis) {
              profData()->setOptimized(funcBodySk(hasThis));
            });
  }
  SCOPE_EXIT{
    withThis(func,
             [&] (bool hasThis) {
               profData()->clearOptimized(funcBodySk(hasThis));
             });
  };

  bool emittedAnyDVInit = false;
  for (TransID tid : prologTransIDs) {
    emittedAnyDVInit |= regeneratePrologue(tid, info);
  }

  // If we tried to include the function body along with a DV init, but didn't
  // end up generating any DV init, then flag that the function body was not
  // included.
  return emittedAnyDVInit && includedBody;
}

TCA getFuncPrologue(Func* func, int nPassed) {
  VMProtect _;

  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int const numParams = func->numNonVariadicParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  // Do a quick test before grabbing the write lease
  if (auto const p = checkCachedPrologue(func, paramIndex)) return p;

  auto computeKind = [&] {
    return tc::profileFunc(func) ? TransKind::ProfPrologue :
                                   TransKind::LivePrologue;
  };

  const auto funcId = func->getFuncId();

  // Avoid a race where we would create a LivePrologue while retranslateAll is
  // in flight and we haven't generated an OptPrologue for the function yet.
  if (retranslateAllPending() && computeKind() == TransKind::LivePrologue &&
      profData() && profData()->profiling(funcId)) {
    return nullptr;
  }

  LeaseHolder writer(func, computeKind());
  if (!writer) return nullptr;

  auto const kind = computeKind();
  // Check again now that we have the write lease, in case the answer to
  // profileFunc() changed.
  if (!writer.checkKind(kind)) return nullptr;

  if (!tc::shouldTranslate(func, kind)) return nullptr;

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (auto const p = checkCachedPrologue(func, paramIndex)) return p;

  return tc::emitFuncPrologue(func, nPassed, kind);
}

}}}
