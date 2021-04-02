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

#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc-prologue.h"
#include "hphp/runtime/vm/jit/tc-region.h"
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
  auto sk = rec->srcKey();

  tracing::Block _b{
    "regenerate-prologue",
    [&] {
      return traceProps(func)
        .add("nargs", nArgs)
        .add("sk", show(sk));
    }
  };

  {
    // The prologue translator may hold the code and metadata locks after a
    // succesuful translation.
    auto translatorPtr = std::make_unique<tc::PrologueTranslator>(
      func, nArgs, TransKind::OptPrologue
    );
    auto& translator = *translatorPtr;
    translator.transId = prologueTransId;
    translator.resetCached();

    // We don't acquire requisite paperwork etc. here since we are assuming that
    // a lease/lock is already held for full function optimization.
    if (retranslateAllEnabled()) {
      translator.translate(info.tcBuf.view());
      info.add(std::move(translatorPtr));
    } else {
      if (translator.shouldTranslate(true) !=
          TranslationResult::Scope::Success) {
        return false;
      }
      translator.translate();
      if (translator.translateSuccess()) {
        translator.relocate();
        translator.publish();
      }
    }
  }

  // If this prologue has a DV funclet, then invalidate it and return its SrcKey
  // and TransID
  if (nArgs < func->numNonVariadicParams()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      bool ret = false;
      SrcKey funcletSK(func, paramInfo.funcletOff, ResumeMode::None);
      if (!profData()->optimized(funcletSK)) {
        auto funcletTransId = profData()->dvFuncletTransId(funcletSK);
        if (funcletTransId != kInvalidTransID) {
          auto translator = std::make_unique<tc::RegionTranslator>(
            funcletSK, TransKind::Optimize
          );
          translator->transId = funcletTransId;
          auto const region = selectHotRegion(funcletTransId);
          translator->region = region;
          auto const spOff = region->entry()->initialSpOffset();
          translator->spOff = spOff;
          tc::createSrcRec(funcletSK, spOff);

          translator->translate(info.tcBuf.view());
          if (translator->translateSuccess()) {
            // Flag that this translation has been retranslated, so that
            // it's not retranslated again along with the function body.
            profData()->setOptimized(funcletSK);
            ret = true;
            info.add(std::move(translator));
          }
        }
      }
      if (ret) return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
}

bool regeneratePrologues(Func* func, tc::FuncMetaInfo& info) {
  std::vector<TransID> prologTransIDs;

  tracing::Block _b{
    "regenerate-prologues",
    [&] {
      return traceProps(func)
        .add("num_prologues", func->numPrologues());
    }
  };

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
    func->bclen() <= RuntimeOption::EvalJitPGOMaxFuncSizeDupBody;

  auto funcBodySk = SrcKey{func, 0, ResumeMode::None};
  if (!includedBody) {
    profData()->setOptimized(funcBodySk);
  }
  SCOPE_EXIT{
    profData()->clearOptimized(funcBodySk);
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

TranslationResult getFuncPrologue(Func* func, int nPassed) {
  VMProtect _;

  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);

  tc::PrologueTranslator translator(func, nPassed);

  auto const tcAddr = translator.acquireLeaseAndRequisitePaperwork();
  if (tcAddr) return *tcAddr;

  translator.translate();
  if (!translator.translateSuccess()) {
    return TranslationResult::failTransiently();
  }

  translator.relocate();
  return TranslationResult{translator.publish()};
}

}}}
