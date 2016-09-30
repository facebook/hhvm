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


#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc-relocate.h"

#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

namespace {

/*
 * Attempt to emit code for the given IRUnit to `code'. Returns true on
 * success, false if codegen failed.
 */
bool mcGenUnit(TransEnv& env, CodeCache::View codeView, CGMeta& fixups) {
  auto const& unit = *env.unit;
  try {
    emitVunit(*env.vunit, unit, codeView, fixups,
              mcgen::dumpTCAnnotation(*env.args.sk.func(), env.args.kind)
              ? &env.annotations
              : nullptr);
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      code().disableHot();
      return false;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }

  auto const startSk = unit.context().srcKey();
  if (unit.context().kind == TransKind::Profile) {
    profData()->setProfiling(startSk.func()->getFuncId());
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
}

SrcRec* findSrcRec(SrcKey sk) {
  return srcDB().find(sk);
}

void createSrcRec(SrcKey sk, FPInvOffset spOff) {
  if (srcDB().find(sk)) return;

  auto const srcRecSPOff = sk.resumed() ? folly::none
                                        : folly::make_optional(spOff);

  // We put retranslate requests at the end of our slab to more frequently
  // allow conditional jump fall-throughs
  auto codeLock = lockCode();
  auto codeView = code().view();
  TCA astart = codeView.main().frontier();
  TCA coldStart = codeView.cold().frontier();
  TCA frozenStart = codeView.frozen().frontier();
  TCA req;
  if (!RuntimeOption::EvalEnableReusableTC) {
    req = svcreq::emit_persistent(codeView.cold(),
                                  codeView.data(),
                                  srcRecSPOff,
                                  REQ_RETRANSLATE,
                                  sk.offset(),
                                  TransFlags().packed);
  } else {
    auto const stubsize = svcreq::stub_size();
    auto newStart = codeView.cold().allocInner(stubsize);
    if (!newStart) {
      newStart = codeView.cold().frontier();
    }
    // Ensure that the anchor translation is a known size so that it can be
    // reclaimed when the function is freed
    req = svcreq::emit_ephemeral(codeView.cold(),
                                 codeView.data(),
                                 (TCA)newStart,
                                 srcRecSPOff,
                                 REQ_RETRANSLATE,
                                 sk.offset(),
                                 TransFlags().packed);
  }
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          sk.unit(), sk.offset(), req);

  auto metaLock = lockMetadata();
  always_assert(srcDB().find(sk) == nullptr);
  auto const sr = srcDB().insert(sk);
  if (RuntimeOption::EvalEnableReusableTC) {
    recordFuncSrcRec(sk.func(), sr);
  }
  sr->setFuncInfo(sk.func());
  sr->setAnchorTranslation(req);

  if (srcRecSPOff) always_assert(sr->nonResumedSPOff() == *srcRecSPOff);

  size_t asize      = codeView.main().frontier()   - astart;
  size_t coldSize   = codeView.cold().frontier()   - coldStart;
  size_t frozenSize = codeView.frozen().frontier() - frozenStart;
  assertx(asize == 0);
  if (coldSize && RuntimeOption::EvalDumpTCAnchors) {
    auto const transID =
      profData() && transdb::enabled() ? profData()->allocTransID()
                                       : kInvalidTransID;
    TransRec tr(sk, transID, TransKind::Anchor,
                astart, asize, coldStart, coldSize,
                frozenStart, frozenSize);
    transdb::addTranslation(tr);
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportTraceletToVtune(sk.unit(), sk.func(), tr);
    }

    assertx(!transdb::enabled() ||
            transdb::getTransRec(coldStart)->kind == TransKind::Anchor);
  }
}


////////////////////////////////////////////////////////////////////////////////

TCA emitTranslation(TransEnv env) {
  Timer timer(Timer::mcg_finishTranslation);

  VMProtect _;

  if (env.prologue) {
    assertx(env.args.kind == TransKind::Optimize);
    emitFuncPrologueOpt(env.prologue);
  }

  auto& args = env.args;
  auto const sk = args.sk;

  profileSetHotFuncAttr();

  auto codeLock = lockCode();
  auto codeView = code().view(args.kind);

  CGMeta fixups;
  TransLocMaker maker{codeView};
  maker.markStart();

  // mcGenUnit emits machine code from vasm
  if (env.vunit && !mcGenUnit(env, codeView, fixups)) {
    // mcGenUnit() failed. Roll back, drop the unit and region, and clear
    // fixups.
    maker.rollback();
    maker.markStart();
    env.unit.reset();
    env.vunit.reset();
    args.region.reset();
    fixups.clear();
  }

  if (env.vunit) {
    if (!newTranslation()) {
      return nullptr;
    }
  } else {
    args.kind = TransKind::Interp;
    FTRACE(1, "emitting dispatchBB interp request for failed "
           "translation (spOff = {})\n", env.initSpOffset.offset);
    vwrap(codeView.main(), codeView.data(), fixups,
          [&] (Vout& v) { emitInterpReq(v, sk, env.initSpOffset); },
          CodeKind::Helper);
  }

  Timer metaTimer(Timer::mcg_finishTranslation_metadata);
  auto metaLock = lockMetadata();
  auto loc = maker.markEnd();

  tryRelocateNewTranslation(sk, loc, codeView, fixups);

  // Finally, record various metadata about the translation and add it to the
  // SrcRec.
  if (RuntimeOption::EvalProfileBC) {
    auto const vmUnit = sk.unit();
    TransBCMapping prev{};
    for (auto& cur : fixups.bcMap) {
      if (!cur.aStart) continue;
      if (prev.aStart) {
        if (prev.bcStart < vmUnit->bclen()) {
          recordBCInstr(uint32_t(vmUnit->getOp(prev.bcStart)),
                        prev.aStart, cur.aStart, false);
        }
      } else {
        recordBCInstr(OpTraceletGuard, loc.mainStart(), cur.aStart, false);
      }
      prev = cur;
    }
  }

  auto const srcRec = srcDB().find(args.sk);
  always_assert(srcRec);
  recordRelocationMetaData(sk, *srcRec, loc, fixups);
  recordGdbTranslation(sk, sk.func(), codeView.main(), loc.mainStart(),
                       false, false);
  recordGdbTranslation(sk, sk.func(), codeView.cold(), loc.coldStart(),
                       false, false);
  if (args.kind == TransKind::Profile) {
    always_assert(args.region);
    profData()->addTransProfile(env.transID, args.region, env.pconds);
  }

  auto tr = maker.rec(sk, env.transID, args.kind, args.region, fixups.bcMap,
                      std::move(env.annotations),
                      env.unit && cfgHasLoop(*env.unit));
  transdb::addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  GrowableVector<IncomingBranch> inProgressTailBranches;
  fixups.process(&inProgressTailBranches);

  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
        loc.mainStart(), sk.funcID(), sk.offset());
  srcRec->newTranslation(loc, inProgressTailBranches);

  TRACE(1, "mcg: %zd-byte tracelet\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getTCSpace().c_str());
  }

  reportJitMaturity(code());

  if (env.unit && env.unit->logEntry()) {
    logTranslation(env);
  }

  if (env.unit) {
    auto func = const_cast<Func*>(env.unit->context().func);

    auto const regionSk = env.unit->context().srcKey();
    if (env.args.kind == TransKind::Optimize &&
        regionSk.offset() == func->base() &&
        func->getDVFunclets().size() == 0 &&
        func->getFuncBody() == ustubs().funcBodyHelperThunk) {
      func->setFuncBody(loc.mainStart());
    }
  }

  return loc.mainStart();
}

static void invalidateSrcKeyNoLock(SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  auto const sr = srcDB().find(sk);
  always_assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  FTRACE_MOD(Trace::reusetc, 1, "Replacing translations from func {} (id = {}) "
             "@ sk({}) in SrcRec addr={}\n",
             sk.func()->fullName()->data(), sk.func()->getFuncId(), sk.offset(),
             (void*)sr);
  Trace::Indent _i;
  sr->replaceOldTranslations();
}

void invalidateSrcKey(SrcKey sk) {
  auto codeLock = lockCode();
  invalidateSrcKeyNoLock(sk);
}

void invalidateFuncProfSrcKeys(const Func* func) {
  assertx(profData());
  auto const funcId = func->getFuncId();
  auto codeLock = lockCode();
  for (auto tid : profData()->funcProfTransIDs(funcId)) {
    invalidateSrcKeyNoLock(profData()->transRec(tid)->srcKey());
  }
}

}}}
