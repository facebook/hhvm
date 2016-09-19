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

#include "hphp/runtime/vm/jit/mcgen-translate.h"

#include "hphp/runtime/vm/jit/mcgen.h"

#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen { namespace {
/*
 * Returns true iff we already have Eval.JitMaxTranslations translations
 * recorded in srcRec.
 */
bool reachedTranslationLimit(SrcKey sk, const SrcRec& srcRec) {
  if (srcRec.translations().size() != RuntimeOption::EvalJitMaxTranslations) {
    return false;
  }
  INC_TPC(max_trans);

  if (debug && Trace::moduleEnabled(Trace::mcg, 2)) {
    const auto& tns = srcRec.translations();
    TRACE(1, "Too many (%zd) translations: %s, BC offset %d\n",
          tns.size(), sk.unit()->filepath()->data(),
          sk.offset());
    SKTRACE(2, sk, "{\n");
    TCA topTrans = srcRec.getTopTranslation();
    for (size_t i = 0; i < tns.size(); ++i) {
      auto const rec = transdb::getTransRec(tns[i].mainStart());
      assertx(rec);
      SKTRACE(2, sk, "%zd %p\n", i, tns[i].mainStart());
      if (tns[i].mainStart() == topTrans) {
        SKTRACE(2, sk, "%zd: *Top*\n", i);
      }
      if (rec->kind == TransKind::Anchor) {
        SKTRACE(2, sk, "%zd: Anchor\n", i);
      } else {
        SKTRACE(2, sk, "%zd: guards {\n", i);
        for (unsigned j = 0; j < rec->guards.size(); ++j) {
          FTRACE(2, "{}\n", rec->guards[j]);
        }
        SKTRACE(2, sk, "%zd } guards\n", i);
      }
    }
    SKTRACE(2, sk, "} /* Too many translations */\n");
  }
  return true;
}

void populateLiveContext(RegionContext& ctx) {
  auto const fp = vmfp();
  auto const sp = vmsp();

  always_assert(ctx.func == fp->m_func);

  auto const ctxClass = ctx.func->cls();
  // Track local types.
  for (uint32_t i = 0; i < fp->m_func->numLocals(); ++i) {
    ctx.liveTypes.push_back(
      { Location::Local{i}, typeFromTV(frame_local(fp, i), ctxClass) }
    );
    FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
  }

  // Track stack types and pre-live ActRecs.
  int32_t stackOff = 0;
  visitStackElems(
    fp, sp, ctx.bcOffset,
    [&] (const ActRec* ar, Offset) {
      auto const objOrCls =
        !ar->func()->cls() ? TNullptr :
        (ar->hasThis()  ?
         Type::SubObj(ar->getThis()->getVMClass()) :
         Type::SubCls(ar->getClass()));

      ctx.preLiveARs.push_back({ stackOff, ar->func(), objOrCls });
      FTRACE(2, "added prelive ActRec {}\n", show(ctx.preLiveARs.back()));
      stackOff += kNumActRecCells;
    },
    [&] (const TypedValue* tv) {
      ctx.liveTypes.push_back(
        { Location::Stack{ctx.spOffset - stackOff}, typeFromTV(tv, ctxClass) }
      );
      stackOff++;
      FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
    }
  );
}

/*
 * Analyze the given TransArgs and return the region to translate, or nullptr
 * if one could not be selected.
 */
RegionDescPtr prepareRegion(const TransArgs& args) {
  if (args.kind == TransKind::Optimize) {
    assertx(RuntimeOption::EvalJitPGO);
    if (args.region) return args.region;

    assertx(isValidTransID(args.transId));
    return selectHotRegion(args.transId);
  }

  // Attempt to create a region at this SrcKey
  assertx(args.kind == TransKind::Profile || args.kind == TransKind::Live);
  auto const sk = args.sk;
  RegionContext rContext { sk.func(), sk.offset(), liveSpOff(),
                           sk.resumed() };
  FTRACE(2, "populating live context for region\n");
  populateLiveContext(rContext);
  return selectRegion(rContext, args.kind);
}

bool liveFrameIsPseudoMain() {
  auto const ar = vmfp();
  if (!(ar->func()->attrs() & AttrMayUseVV)) return false;
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

////////////////////////////////////////////////////////////////////////////////
}

TCA translate(TransArgs args) {
  INC_TPC(translate);

  assert(args.kind != TransKind::Invalid);
  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  if (!tc::shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  Timer timer(Timer::mcg_translate);

  auto const srcRec = tc::findSrcRec(args.sk);
  always_assert(srcRec);

  args.region = reachedTranslationLimit(args.sk, *srcRec) ? nullptr
                                                          : prepareRegion(args);
  TransEnv env{args};
  env.initSpOffset = args.region ? args.region->entry()->initialSpOffset()
                                 : liveSpOff();
  env.annotations.insert(env.annotations.end(),
                         args.annotations.begin(), args.annotations.end());

  // Lower the RegionDesc to an IRUnit, then lower that to a Vunit.
  if (args.region) {
    if (args.kind == TransKind::Profile || (profData() && transdb::enabled())) {
      env.transID = profData()->allocTransID();
    }
    auto const transContext =
      TransContext{env.transID, args.kind, args.flags, args.sk,
                   env.initSpOffset};

    env.unit = irGenRegion(*args.region, transContext,
                           env.pconds, env.annotations);
    if (env.unit) {
      env.vunit = irlower::lowerUnit(*env.unit);
    }
  }

  timer.stop();
  return tc::emitTranslation(std::move(env));
}

/*
 * Return an existing translation for `args', or nullptr if one can't be found.
 */
TCA findTranslation(const TransArgs& args) {
  auto sk = args.sk;
  sk.func()->validate();

  if (liveFrameIsPseudoMain() && !RuntimeOption::EvalJitPseudomain) {
    SKTRACE(2, sk, "punting on pseudoMain\n");
    return nullptr;
  }

  if (auto const sr = tc::findSrcRec(sk)) {
    if (auto const tca = sr->getTopTranslation()) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return tca;
    }
  }

  return nullptr;
}

TCA createTranslation(TransArgs args) {
  args.kind = tc::profileSrcKey(args.sk) ? TransKind::Profile : TransKind::Live;

  if (!tc::shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  auto sk = args.sk;
  LeaseHolder writer(GetWriteLease(), sk.func(), args.kind);
  if (!writer || !tc::shouldTranslate(sk.func(), args.kind)) return nullptr;

  if (RuntimeOption::EvalFailJitPrologs && sk.op() == Op::FCallAwait) {
    return nullptr;
  }

  tc::createSrcRec(sk);

  auto sr = tc::findSrcRec(sk);
  always_assert(sr);

  if (auto const tca = sr->getTopTranslation()) {
    // Handle extremely unlikely race; someone may have just added the first
    // translation for this SrcRec while we did a non-blocking wait on the
    // write lease in createSrcRec().
    return tca;
  }

  return retranslate(args);
}

}}}
