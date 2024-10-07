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

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-prologue.h"
#include "hphp/runtime/vm/jit/tc-region.h"

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
  AsyncPrologueContext(Func* func, int nPassed)
    : func(func)
    , nPassed(nPassed)
  {}

  Func* func;
  int nPassed;
};

using AsyncTranslationContext =
  std::variant<AsyncRegionTranslationContext, AsyncPrologueContext>;

struct AsyncTranslationWorker
  : JobQueueWorker<AsyncTranslationContext, void*, true, true> {

  AsyncTranslationWorker()
    : m_codeBuffer(std::make_unique<uint8_t[]>(initialSize))
  {}

  void doAsyncRegionTranslation(AsyncRegionTranslationContext& rctx) {

    auto const ctx = rctx.ctx;

    SCOPE_EXIT {
      FTRACE(2, "Finished background jit attempt for sk {}\n", show(ctx.sk));
      s_enqueuedSKs.assign(ctx.sk, false);
    };

    FTRACE(2, "Background jit attempt started for sk {}\n", show(ctx.sk));

    auto const srcRec = tc::createSrcRec(ctx.sk, ctx.spOffset);
    if (!srcRec) {
      FTRACE(2, "createSrcRec failed for sk {}\n", show(ctx.sk));
       return;
    }
    auto const numTrans = srcRec->numTrans();
    // If new translations were created since this request was enqueued,
    // let the next execution run through the retranslation chain.
    if (rctx.currNumTranslations != numTrans) {
      FTRACE(2, "New translation found for sk {}\n", show(ctx.sk));
      return;
    }

    ProfileNonVMThread nonVM;
    HphpSession hps{Treadmill::SessionKind::TranslateWorker};
    VMProtect _;

    tc::RegionTranslator translator(ctx.sk, TransKind::Live);
    translator.spOff = ctx.spOffset;
    translator.liveTypes = ctx.liveTypes;

    if (auto const s = translator.shouldTranslate();
      s != TranslationResult::Scope::Success) {
      FTRACE(2, "shouldTranslate failed for sk {}\n", show(ctx.sk));
      if (s == TranslationResult::Scope::Process) {
        translator.setCachedForProcessFail();
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
      }
      FTRACE(2, "Successfully generated translation for sk {}\n", show(ctx.sk));
    } else {
      FTRACE(2, "Background jitting failed for sk {}\n", show(ctx.sk));
    }
  }

  void doAsyncPrologueGen(AsyncPrologueContext& ctx) {

    SCOPE_EXIT {
      FTRACE(2, "Finished prologue generation attempt for func {}\n",
             ctx.func->name());
      ctx.func->atomicFlags().unset(Func::Flags::LockedForPrologueGen);
    };

    FTRACE(2, "Background prologue generation attempt started for func {}\n",
           ctx.func->name());

    ProfileNonVMThread nonVM;
    HphpSession hps{Treadmill::SessionKind::TranslateWorker};
    VMProtect _;

    tc::PrologueTranslator translator(ctx.func, ctx.nPassed);

    auto const tcAddr = translator.getCached();
    if (tcAddr) {
      FTRACE(2, "getCached returned true for prologue of func {}\n",
             ctx.func->name());
      return;
    }

 		if (auto const s = translator.shouldTranslate();
        s != TranslationResult::Scope::Success) {
      FTRACE(2, "shouldTranslate failed for func prologue {}\n",
             ctx.func->name());
      if (s == TranslationResult::Scope::Process) {
        translator.setCachedForProcessFail();
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
             ctx.func->name());
    } else {
      FTRACE(2, "Background prologue generation failed for func {}\n",
             ctx.func->name());
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
    auto const p = std::make_shared<AsyncPrologueContext>(func, nPassed);
    auto& dispatcher = asyncTranslationDispatcher();
    dispatcher.enqueue(AsyncPrologueContext {func, nPassed});
    FTRACE(2, "Enqueued func {} for prologue generation\n", func->name());
  } else {
    FTRACE(2,
      "In progress prologue genertion, skipping enqueue for func {}\n",
      func->name()
    );
  }
}

}
