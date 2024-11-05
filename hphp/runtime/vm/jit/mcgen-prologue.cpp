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
#include "hphp/runtime/vm/jit/mcgen-async.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"

#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc-prologue.h"
#include "hphp/runtime/vm/jit/tc-region.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/resumable.h"

#include "hphp/util/configs/eval.h"
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
namespace HPHP::jit::mcgen {

namespace {

/**
 * Given the proflogueTransId for a TransProflogue translation, regenerate the
 * prologue (as a TransPrologue).
 */
void regeneratePrologue(TransID prologueTransId, tc::FuncMetaInfo& info) {
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

  // The prologue translator may hold the code and metadata locks after a
  // succesuful translation.
  auto translatorPtr = std::make_unique<tc::PrologueTranslator>(
    func, nArgs, TransKind::OptPrologue
  );
  auto& translator = *translatorPtr;
  translator.proflogueTransId = prologueTransId;
  translator.resetCached();

  // We don't acquire requisite paperwork etc. here since we are assuming that
  // a lease/lock is already held for full function optimization.
  if (retranslateAllEnabled()) {
    translator.translate(info.tcBuf.view());
    info.add(std::move(translatorPtr));
    return;
  }

  if (translator.shouldTranslate(true) != TranslationResult::Scope::Success) {
    return;
  }

  UNUSED auto const result = [&] {
    if (auto const res = translator.translate()) return *res;
    if (auto const res = translator.relocate(true)) return *res;
    if (auto const res = translator.bindOutgoingEdges()) return *res;
    return translator.publish();
  }();
}

////////////////////////////////////////////////////////////////////////////////
}

void regeneratePrologues(Func* func, tc::FuncMetaInfo& info) {
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

  std::sort(
    prologTransIDs.begin(), prologTransIDs.end(),
    [&](TransID t1, TransID t2) -> bool {
      // This will sort in ascending order.
      return profData()->transCounter(t2) > profData()->transCounter(t1);
    }
  );

  for (TransID tid : prologTransIDs) {
    regeneratePrologue(tid, info);
  }
}

TranslationResult getFuncPrologue(Func* func, int nPassed) {
  VMProtect _;

  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);

  tc::PrologueTranslator translator(func, nPassed);

  if (Cfg::Eval::EnableAsyncJIT) {
    assertx(!RuntimeOption::RepoAuthoritative);
    assertx(!tc::profileFunc(func));
    FTRACE_MOD(Trace::async_jit, 2,
               "Attempting async prologue generation for func {}\n", func->name());
    if (auto const tcAddr = translator.getCached()) {
      FTRACE_MOD(Trace::async_jit, 2,
                 "Found prologue for func {}, skipping enqueue\n", func->name());
      return *tcAddr;
    }
    FTRACE_MOD(Trace::async_jit, 2,
               "Enqueueing func {} for prologue generation\n", func->name());
    mcgen::enqueueAsyncPrologueRequest(func, nPassed);
    return TranslationResult::failTransiently();
  }

  auto const tcAddr = translator.acquireLeaseAndRequisitePaperwork();
  if (tcAddr) return *tcAddr;

  if (auto const res = translator.translate()) return *res;
  if (auto const res = translator.relocate(true)) return *res;
  if (auto const res = translator.bindOutgoingEdges()) return *res;
  return translator.publish();
}

}
