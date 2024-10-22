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

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-prologue.h"
#include "hphp/runtime/vm/jit/tc-region.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/util/alloc.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/trace.h"

#include <folly/system/ThreadName.h>

TRACE_SET_MOD(async_jit);

namespace HPHP::jit::mcgen {

namespace {

folly_concurrent_hash_map_simd<SrcKey, bool, SrcKey::Hasher> s_enqueuedSKs;

struct AsyncRegionTranslationContext {
  AsyncRegionTranslationContext(const RegionContext& ctx,
                                int currNumTranslations)
    : ctx(ctx)
    , currNumTranslations(currNumTranslations)
  {}

  RegionContext ctx;
  int currNumTranslations;
};

struct AsyncPrologueContext {
  AsyncPrologueContext(FuncId funcId, int nPassed)
    : funcId(funcId)
    , nPassed(nPassed)
  {}

  FuncId funcId;
  int nPassed;
};

namespace {
std::atomic<size_t> s_successTrans;
std::atomic<size_t> s_failTrans;
std::atomic<size_t> s_failSrcRec;
std::atomic<size_t> s_permFailTrans;
std::atomic<size_t> s_rejectTrans;
std::atomic<size_t> s_activeTrans;
std::atomic<size_t> s_interpTrans;

std::atomic<size_t> s_successPrologue;
std::atomic<size_t> s_failPrologue;
std::atomic<size_t> s_permFailPrologue;
std::atomic<size_t> s_rejectPrologue;
std::atomic<size_t> s_activePrologue;

InitFiniNode s_logJitStats([]{
  if (!RO::EvalEnableAsyncJIT ||
      !isStandardRequest() ||
      !StructuredLog::coinflip(Cfg::Eval::AsyncJitLogStatsRate)) return;
  StructuredLogEntry ent;

  constexpr auto MO = std::memory_order_relaxed;
  ent.setInt("succeeded_trans", s_successTrans.load(MO));
  ent.setInt("failed_src_rec", s_failSrcRec.load(MO));
  ent.setInt("failed_trans", s_failTrans.load(MO));
  ent.setInt("permanent_failed_trans", s_permFailTrans.load(MO));
  ent.setInt("rejected_trans", s_rejectTrans.load(MO));
  ent.setInt("active_trans", s_activeTrans.load(MO));
  ent.setInt("interp_trans", s_interpTrans.load(MO));

  ent.setInt("succeeded_prologue", s_successPrologue.load(MO));
  ent.setInt("failed_prologue", s_failPrologue.load(MO));
  ent.setInt("permanent_failed_prologue", s_permFailPrologue.load(MO));
  ent.setInt("rejected_prologue", s_rejectPrologue.load(MO));
  ent.setInt("active_prologue", s_activePrologue.load(MO));

  ent.setInt("main_used_bytes", tc::code().main().used());
  ent.setInt("cold_used_bytes", tc::code().cold().used());
  ent.setInt("frozen_used_bytes", tc::code().frozen().used());
  ent.setInt("data_used_bytes", tc::code().data().used());

  ent.setInt("main_max_bytes", CodeCache::AMaxUsage);
  ent.setInt("cold_max_bytes", CodeCache::AColdMaxUsage);
  ent.setInt("frozen_max_bytes", CodeCache::AFrozenMaxUsage);
  ent.setInt("data_max_bytes", CodeCache::GlobalDataSize);

  ent.setInt("request_index", requestCount());
  ent.setInt("reached_limit", tc::canTranslate() ? 0 : 1);

  ent.setInt("sample_rate", Cfg::Eval::AsyncJitLogStatsRate);

  StructuredLog::log("hhvm_async_jit_stats", ent);
}, InitFiniNode::When::RequestFini, "logJitStats");
}

using AsyncTranslationContext =
  std::variant<AsyncRegionTranslationContext, AsyncPrologueContext>;

struct AsyncTranslationWorker
  : JobQueueWorker<AsyncTranslationContext, void*, true, true> {

  AsyncTranslationWorker()
    : m_codeBuffer(std::make_unique<uint8_t[]>(initialSize))
  {}

  void doAsyncRegionTranslation(AsyncRegionTranslationContext& rctx) {

    auto const ctx = rctx.ctx;

    s_activeTrans++;
    SCOPE_EXIT {
      s_enqueuedSKs.assign(ctx.sk, false);
      s_activeTrans--;
    };

    ProfileNonVMThread nonVM;
    HphpSession hps{Treadmill::SessionKind::TranslateWorker};
    VMProtect _;

    if (!Func::isFuncIdValid(ctx.sk.funcID())) {
      FTRACE(2, "Invalid func id {}\n", ctx.sk.funcID().toInt());
      return;
    }

    if (ctx.sk.func()->atomicFlags().check(Func::Flags::Zombie)) {
      FTRACE(2, "Zombie function {}\n", ctx.sk.func()->fullName());
      return;
    }

    auto const srcRec = tc::createSrcRec(ctx.sk, ctx.spOffset);
    if (!srcRec) {
      FTRACE(2, "createSrcRec failed for sk {}\n", show(ctx.sk));
      s_failSrcRec++;
      return;
    }
    auto const numTrans = srcRec->numTrans();
    // If new translations were created since this request was enqueued,
    // let the next execution run through the retranslation chain.
    if (rctx.currNumTranslations != numTrans) {
      FTRACE(2, "New translation found for sk {}\n", show(ctx.sk));
      return;
    }

    tc::RegionTranslator translator(ctx.sk, TransKind::Live);
    translator.spOff = ctx.spOffset;
    translator.liveTypes = ctx.liveTypes;

    if (auto const s = translator.shouldTranslate();
      s != TranslationResult::Scope::Success) {
      s_rejectTrans++;
      FTRACE(2, "shouldTranslate failed for sk {}\n", show(ctx.sk));
      if (s == TranslationResult::Scope::Process) {
        translator.setCachedForProcessFail();
        s_permFailTrans++;
      }
      return;
    }

    tc::LocalTCBuffer buf {m_codeBuffer.get(), initialSize};
    auto const result = [&] {
      if (auto const res = translator.translate(buf.view())) return *res;
      if (auto const res = translator.relocate(false)) return *res;
      if (auto const res = translator.bindOutgoingEdges()) return *res;
      return  translator.publish();
    }();
    if (result.scope() == TranslationResult::Scope::Success) {
      if (translator.region && !translator.region->empty()) {
        FTRACE(2, "start: {} | end: {}\n",
               show(translator.region->start()),
               show(translator.region->lastSrcKey()));
        s_successTrans++;
      }
      if (translator.kind == TransKind::Interp) s_interpTrans++;
      FTRACE(2, "Successfully generated translation for sk {}\n", show(ctx.sk));
    } else {
      FTRACE(2, "Background jitting failed for sk {}\n", show(ctx.sk));
      s_failTrans++;
    }
  }

  void doAsyncPrologueGen(AsyncPrologueContext& ctx) {
    ProfileNonVMThread nonVM;
    HphpSession hps{Treadmill::SessionKind::TranslateWorker};
    VMProtect _;

    if (!Func::isFuncIdValid(ctx.funcId)) {
      FTRACE(2, "Invalid func id {}\n", ctx.funcId.toInt());
      return;
    }

    auto const func = const_cast<Func*>(Func::fromFuncId(ctx.funcId));

    SCOPE_EXIT {
      func->atomicFlags().unset(Func::Flags::LockedForPrologueGen);
      s_activePrologue--;
    };

    s_activePrologue++;

    if (func->atomicFlags().check(Func::Flags::Zombie)) {
      FTRACE(2, "Zombie function {}\n", func->fullName());
      return;
    }

    tc::PrologueTranslator translator(func, ctx.nPassed);

    auto const tcAddr = translator.getCached();
    if (tcAddr) {
      FTRACE(2, "getCached returned true for prologue of func {}\n",
             func->name());
      return;
    }

 		if (auto const s = translator.shouldTranslate();
        s != TranslationResult::Scope::Success) {
      FTRACE(2, "shouldTranslate failed for func prologue {}\n",
             func->name());
      s_rejectPrologue++;
      if (s == TranslationResult::Scope::Process) {
        translator.setCachedForProcessFail();
        s_permFailPrologue++;
      }
      return;
    }

    tc::LocalTCBuffer buf {m_codeBuffer.get(), initialSize};
    auto const result = [&] {
      if (auto const res = translator.translate(buf.view())) return *res;
      if (auto const res = translator.relocate(true)) return *res;
      if (auto const res = translator.bindOutgoingEdges()) return *res;
      return translator.publish();
    }();
    if (result.scope() == TranslationResult::Scope::Success) {
      FTRACE(2, "Successfully generated prologue for func {}\n",
             func->name());
      s_successPrologue++;
    } else {
      FTRACE(2, "Background prologue generation failed for func {}\n",
             func->name());
      s_failPrologue++;
    }
  }

  void doJob(AsyncTranslationContext ctx) override {
    if (std::holds_alternative<AsyncRegionTranslationContext>(ctx)) {
      doAsyncRegionTranslation(std::get<AsyncRegionTranslationContext>(ctx));
    } else if (std::holds_alternative<AsyncPrologueContext>(ctx)) {
      doAsyncPrologueGen(std::get<AsyncPrologueContext>(ctx));
    } else {
      not_reached();
    }
  }

  void onThreadEnter() override {
    folly::setThreadName("asyncjitworker");
#if USE_JEMALLOC_EXTENT_HOOKS
    if (auto arena = next_extra_arena(s_numaNode)) {
      arena->bindCurrentThread();
    }
#endif
  }

  static constexpr int initialSize = 4096;
  std::unique_ptr<uint8_t[]> m_codeBuffer;
};

using AsyncTranslationDispatcher = JobQueueDispatcher<AsyncTranslationWorker>;
std::atomic<AsyncTranslationDispatcher*> s_asyncTranslationDispatcher;

AsyncTranslationDispatcher& asyncTranslationDispatcher() {
  auto const dispatcher = s_asyncTranslationDispatcher.load(std::memory_order_acquire);
  assertx(dispatcher);
  return *dispatcher;
}
} // namespace

void initAsyncTranslationDispatcher() {
  auto const dispatcher = new AsyncTranslationDispatcher(
    RuntimeOption::EvalAsyncJitWorkerThreads,
    RuntimeOption::EvalAsyncJitWorkerThreads, 0, false, nullptr
  );
  dispatcher->start();
  s_asyncTranslationDispatcher.store(dispatcher, std::memory_order_release);
}

void joinAsyncTranslationWorkerThreads() {
  auto const dispatcher =
    s_asyncTranslationDispatcher.load(std::memory_order_acquire);
  if (!dispatcher) return;
  s_asyncTranslationDispatcher.store(nullptr, std::memory_order_release);
  FTRACE(2, "Waiting for background jit worker threads\n");
  dispatcher->waitEmpty(true);
  delete dispatcher;
  // Clearing the ConcurrentHashMap here avoids a crash in the destructor.
  s_enqueuedSKs.clear();
}

void enqueueAsyncTranslateRequest(const RegionContext& ctx,
                                  int currNumTranslations) {
  if (!s_enqueuedSKs.insert(ctx.sk, true).second &&
      !s_enqueuedSKs.assign_if_equal(ctx.sk, false, true)) {
    FTRACE(2,
      "In progress jitting found, skipping enqueue for sk {}\n",
      show(ctx.sk)
    );
  } else {
    auto& dispatcher = asyncTranslationDispatcher();
    dispatcher.enqueue(
      AsyncRegionTranslationContext {ctx, currNumTranslations}
    );
    FTRACE(2, "Enqueued sk {} for jitting\n", show(ctx.sk));
  }
}

void enqueueAsyncPrologueRequest(Func* func, int nPassed) {
  if (!func->atomicFlags().set(Func::Flags::LockedForPrologueGen)) {
    auto& dispatcher = asyncTranslationDispatcher();
    dispatcher.enqueue(AsyncPrologueContext {func->getFuncId(), nPassed});
    FTRACE(2, "Enqueued func {} for prologue generation\n", func->name());
  } else {
    FTRACE(2,
      "In progress prologue genertion, skipping enqueue for func {}\n",
      func->name()
    );
  }
}

}
