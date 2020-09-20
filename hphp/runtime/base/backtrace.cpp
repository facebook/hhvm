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
#include "hphp/runtime/base/backtrace.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/concurrent-scalable-cache.h"
#include "hphp/util/struct-log.h"

#include <folly/small_vector.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_file("file"),
  s_line("line"),
  s_function("function"),
  s_args("args"),
  s_class("class"),
  s_object("object"),
  s_type("type"),
  s_include("include"),
  s_main("{main}"),
  s_metadata("metadata"),
  s_arrow("->"),
  s_double_colon("::");

///////////////////////////////////////////////////////////////////////////////

namespace backtrace_detail {

BTContext::BTContext() {
  // don't attempt to read locals
  auto const flags = 1U << ActRec::LocalsDecRefd;
  auto const handler = (intptr_t)jit::tc::ustubs().retInlHelper;
  fakeAR[0].m_sfp = &fakeAR[1];
  fakeAR[1].m_sfp = &fakeAR[0];
  fakeAR[0].m_savedRip = fakeAR[1].m_savedRip = handler;
  fakeAR[0].m_numArgs = fakeAR[1].m_numArgs = 0;
  fakeAR[0].m_callOffAndFlags = fakeAR[1].m_callOffAndFlags =
    ActRec::encodeCallOffsetAndFlags(0, flags);
}

const ActRec* BTContext::clone(const BTContext& src, const ActRec* fp) {
#ifdef USE_LOWPTR
  fakeAR[0].m_func = src.fakeAR[0].m_func;
  fakeAR[1].m_func = src.fakeAR[1].m_func;
#else
  fakeAR[0].m_funcId = src.fakeAR[0].m_funcId;
  fakeAR[1].m_funcId = src.fakeAR[1].m_funcId;
#endif

  fakeAR[0].m_callOffAndFlags = src.fakeAR[0].m_callOffAndFlags;
  fakeAR[1].m_callOffAndFlags = src.fakeAR[1].m_callOffAndFlags;

  stashedFrm = src.stashedFrm;
  inlineStack = src.inlineStack;
  prevIFID = src.prevIFID;
  hasInlFrames = src.hasInlFrames;
  afwhTailFrameIndex = src.afwhTailFrameIndex;
  assertx(!!stashedFrm == (fp == &src.fakeAR[0] || fp == &src.fakeAR[1]));

  return
    fp == &src.fakeAR[0] ? &fakeAR[0] :
    fp == &src.fakeAR[1] ? &fakeAR[1] :
    fp;
}

}

///////////////////////////////////////////////////////////////////////////////

namespace {

c_WaitableWaitHandle* getParentWH(
  c_WaitableWaitHandle* wh,
  context_idx_t contextIdx,
  folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs
) {
  assertx(!wh->isFinished());
  auto p = wh->getParentChain().firstInContext(contextIdx);
  if (p == nullptr ||
      UNLIKELY(std::find(visitedWHs.begin(), visitedWHs.end(), p)
               != visitedWHs.end())) {
    // If the parent exists in our backtrace, it means we have detected a
    // cycle. We well fall back to savedFP in that case.
    return nullptr;
  }
  visitedWHs.push_back(p);
  return p;
}

BTFrame getARFromWHImpl(
  c_WaitableWaitHandle* currentWaitHandle,
  context_idx_t contextIdx,
  folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs
) {
  while (currentWaitHandle != nullptr) {
    assertx(!currentWaitHandle->isFinished());

    if (currentWaitHandle->getKind() == c_Awaitable::Kind::AsyncFunction) {
      auto const resumable = currentWaitHandle->asAsyncFunction()->resumable();
      return BTFrame { resumable->actRec(), resumable->suspendOffset() };
    }
    if (currentWaitHandle->getKind() == c_Awaitable::Kind::AsyncGenerator) {
      auto const resumable = currentWaitHandle->asAsyncGenerator()->resumable();
      return BTFrame { resumable->actRec(), resumable->suspendOffset() };
    }
    currentWaitHandle = getParentWH(currentWaitHandle, contextIdx, visitedWHs);
  }
  auto const fp = AsioSession::Get()->getContext(contextIdx)->getSavedFP();
  assertx(fp != nullptr && fp->func() != nullptr);
  return BTFrame { fp, fp->func()->base() };
}

}

BTFrame getARFromWH(
  c_WaitableWaitHandle* currentWaitHandle,
  folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs
) {
  if (currentWaitHandle->isFinished()) return BTFrame{};

  auto const contextIdx = currentWaitHandle->getContextIdx();
  if (contextIdx == 0) return BTFrame{};

  return getARFromWHImpl(currentWaitHandle, contextIdx, visitedWHs);
}

///////////////////////////////////////////////////////////////////////////////

namespace backtrace_detail {

BTFrame initBTContextAt(BTContext& ctx, jit::CTCA ip, BTFrame frm) {
  if (auto const stk = jit::inlineStackAt(ip)) {
    assertx(stk->nframes != 0);

    auto const ifr = jit::getInlineFrame(stk->frame);

    auto const prevFP = &ctx.fakeAR[0];
    prevFP->setFunc(ifr.func);
    prevFP->m_callOffAndFlags = ActRec::encodeCallOffsetAndFlags(
      ifr.callOff,
      1 << ActRec::LocalsDecRefd  // don't attempt to read locals
    );

    ctx.prevIFID = stk->frame;
    ctx.inlineStack = *stk;
    ctx.inlineStack.frame = ifr.parent;
    ctx.hasInlFrames = --ctx.inlineStack.nframes > 0;

    ctx.stashedFrm = frm;

    return BTFrame { prevFP, safe_cast<int>(stk->callOff) + ifr.func->base() };
  }

  // If we don't have an inlined frame, it's always safe to use pcOff() if
  // initBTContextAt() is being called for a leaf frame.
  if (frm.pc == kInvalidOffset) frm.pc = pcOff();

  // The bytecode supports a limited form of inlining via FCallBuiltin. If the
  // call itself was interpreted there won't be any fixup information, but
  // we should still be able to extract the target from the bytecode on the
  // stack and use that directly.
  auto const getBuiltin = [&] () -> const Func* {
    if (!frm) return nullptr;

    auto const func = frm.fp->func();
    auto const pc = func->unit()->entry() + frm.pc;
    if (peek_op(pc) != OpFCallBuiltin) return nullptr;

    auto const ne = func->unit()->lookupNamedEntityId(getImm(pc, 3).u_SA);
    return Func::lookup(ne);
  };

  if (auto const f = getBuiltin()) {
    auto const prevFP = &ctx.fakeAR[0];
    prevFP->setFunc(f);
    prevFP->m_callOffAndFlags = ActRec::encodeCallOffsetAndFlags(
      frm.pc - frm.fp->func()->base(),
      1 << ActRec::LocalsDecRefd  // don't attempt to read locals
    );

    ctx.stashedFrm = frm;

    return BTFrame { prevFP, f->base() };
  }

  return frm;
}

BTFrame getPrevActRec(
  BTContext& ctx, BTFrame frm,
  folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs
) {
  auto const fp = frm.fp;

  // If we're already iterating over an AsyncFunctionWaitHandle's tail frames,
  // either create a fake ActRec for the next tail frame, or go to the parent
  // ActRec of the AFWH itself.
  if (ctx.afwhTailFrameIndex) {
    assertx(fp == &ctx.fakeAR[0] || fp == &ctx.fakeAR[1]);
    assertx(fp->m_sfp == &ctx.fakeAR[0] || fp->m_sfp == &ctx.fakeAR[1]);
    assertx(fp != fp->m_sfp);

    auto const wh = frame_afwh(ctx.stashedFrm.fp);
    if (ctx.afwhTailFrameIndex < wh->lastTailFrameIndex()) {
      auto const sk = getAsyncFrame(wh->tailFrame(ctx.afwhTailFrameIndex++));
      auto const prevFP = fp->m_sfp;
      prevFP->setFunc(sk.func());
      return BTFrame{prevFP, sk.offset()};
    }

    // We could use stashedFrm.fp->savedRip as the return TCA here, but for
    // suspended AFWH, that will always be a pointer to an async-ret stub.
    ctx.afwhTailFrameIndex = 0;
    ctx.stashedFrm = BTFrame{};
    auto const contextIdx = wh->getContextIdx();
    auto const parent = getParentWH(wh, contextIdx, visitedWHs);
    auto const prev = getARFromWHImpl(parent, contextIdx, visitedWHs);
    return initBTContextAt(ctx, (jit::TCA)0, prev);
  }

  if (UNLIKELY(ctx.hasInlFrames)) {
    assertx(fp == &ctx.fakeAR[0] || fp == &ctx.fakeAR[1]);
    assertx(fp->m_sfp == &ctx.fakeAR[0] || fp->m_sfp == &ctx.fakeAR[1]);
    assertx(fp != fp->m_sfp);
    assertx(ctx.inlineStack.nframes > 0);

    auto const ifr = jit::getInlineFrame(ctx.inlineStack.frame);

    auto prev = BTFrame{};
    prev.fp = fp->m_sfp;
    prev.fp->setFunc(ifr.func);
    prev.fp->m_callOffAndFlags = ActRec::encodeCallOffsetAndFlags(
      ifr.callOff,
      1 << ActRec::LocalsDecRefd  // don't attempt to read locals
    );
    prev.pc = fp->callOffset() + ifr.func->base();

    ctx.prevIFID = ctx.inlineStack.frame;
    ctx.inlineStack.frame = ifr.parent;
    ctx.hasInlFrames = --ctx.inlineStack.nframes > 0;

    return prev;
  }

  if (auto prev = ctx.stashedFrm) {
    if (prev.pc == kInvalidOffset) {
      assertx(ctx.prevIFID != kInvalidIFrameID);
      auto const ifr = jit::getInlineFrame(ctx.prevIFID);
      prev.pc = prev.fp->func()->base() + ifr.callOff;
    }
    ctx.stashedFrm = BTFrame{};
    ctx.prevIFID = kInvalidIFrameID;
    return prev;
  }

  auto const wh = [&]() -> c_WaitableWaitHandle* {
    if (!fp || !fp->func() || !isResumed(fp)) return nullptr;

    if (fp->func()->isAsyncFunction()) {
      return frame_afwh(fp);
    }
    if (fp->func()->isAsyncGenerator()) {
      auto const gen = frame_async_generator(fp);
      if (gen->isRunning() && !gen->isEagerlyExecuted()) {
        return gen->getWaitHandle();
      }
    }
    return nullptr;
  }();

  auto rip = fp ? fp->m_savedRip : 0;

  auto prev = frm;

  if (wh != nullptr) {
    if (wh->isFinished()) {
      /*
       * It's possible in very rare cases (it will return a truncated stack):
       * 1) async function which WaitHandle is not referenced by anything
       *      else finishes
       * 2) its return value is an object with destructor
       * 3) this destructor gets called as part of destruction of the
       *      WaitHandleobject, which happens right before FP is adjusted
      */
      return BTFrame{};
    }

    // If we merged tail frames into this AsyncFunctionWaitHandle, iterate over
    // those indices and retrieve the backtracing information.
    if (wh->getKind() == c_Awaitable::Kind::AsyncFunction &&
        wh->asAsyncFunction()->hasTailFrames()) {
      auto const afwh = wh->asAsyncFunction();
      auto const index = afwh->firstTailFrameIndex();
      auto const sk = getAsyncFrame(afwh->tailFrame(index));

      ctx.afwhTailFrameIndex = index + 1;
      ctx.stashedFrm.fp = afwh->actRec();
      auto const prevFP = &ctx.fakeAR[0];
      prevFP->setFunc(sk.func());
      prevFP->m_callOffAndFlags =
        ActRec::encodeCallOffsetAndFlags(0, 1 << ActRec::LocalsDecRefd);
      return BTFrame{prevFP, sk.offset()};
    }

    auto const contextIdx = wh->getContextIdx();
    auto const parent = getParentWH(wh, contextIdx, visitedWHs);
    prev = getARFromWHImpl(parent, contextIdx, visitedWHs);
  } else {
    prev.fp = g_context->getPrevVMState(fp, &prev.pc, nullptr, nullptr, &rip);
  }

  return frm
    ? initBTContextAt(ctx, (jit::TCA)rip, prev)
    : prev;
}

}

using namespace backtrace_detail;

///////////////////////////////////////////////////////////////////////////////

Array createBacktrace(const BacktraceArgs& btArgs) {
  ARRPROV_USE_RUNTIME_LOCATION();
  if (btArgs.isCompact()) {
    return createCompactBacktrace(btArgs.m_skipTop)->extract();
  }

  auto bt = Array::CreateVArray();
  folly::small_vector<c_WaitableWaitHandle*, 64> visitedWHs;

  BTContext ctx;

  if (g_context->m_parserFrame) {
    bt.append(
      make_darray(
        s_file, VarNR(g_context->m_parserFrame->filename.get()),
        s_line, g_context->m_parserFrame->lineNumber
      )
    );
  }

  // If there is a parser frame, put it at the beginning of the backtrace.
  if (btArgs.m_parserFrame) {
    bt.append(
      make_darray(
        s_file, VarNR(btArgs.m_parserFrame->filename.get()),
        s_line, btArgs.m_parserFrame->lineNumber
      )
    );
  }

  int depth = 0;
  auto frm = BTFrame{};

  if (btArgs.m_fromWaitHandle) {
    frm = getARFromWH(btArgs.m_fromWaitHandle, visitedWHs);
    // No frames found, we are done.
    if (!frm) return bt;
  } else {
    VMRegAnchor _;
    // If there are no VM frames, we're done.
    if (!rds::header() || !vmfp()) return bt;

    frm.fp = vmfp();

    // Get the fp and pc of the top frame (possibly skipping one frame).

    if (!btArgs.m_skipInlined) {
      frm = initBTContextAt(ctx, vmJitReturnAddr(), frm);
    }

    if (btArgs.m_skipTop) {
      frm = getPrevActRec(ctx, frm, visitedWHs);
      // We skipped over the only VM frame, we're done.
      if (!frm) return bt;
    }

    if (btArgs.m_skipInlined && RuntimeOption::EvalJit) {
      while (frm && frm.fp->isInlined()) {
        frm = getPrevActRec(ctx, frm, visitedWHs);
      }
      if (!frm) return bt;
    }
  }

  // Handle the top frame.
  if (btArgs.m_withSelf) {
    // Builtins don't have a file and line number, so find the first user frame
    BTContext ctxCopy;
    auto curFrm = BTFrame {
      const_cast<ActRec*>(ctxCopy.clone(ctx, frm.fp)),
      frm.pc
    };
    while (curFrm && curFrm.fp->func()->isBuiltin()) {
      curFrm = getPrevActRec(ctxCopy, curFrm, visitedWHs);
    }
    if (curFrm) {
      auto const func = curFrm.fp->func();
      assertx(func);

      DArrayInit frame(btArgs.m_parserFrame ? 4 : 2);
      frame.set(s_file, Variant{const_cast<StringData*>(func->filename())});
      frame.set(s_line, func->unit()->getLineNumber(curFrm.pc));
      if (btArgs.m_parserFrame) {
        frame.set(s_function, s_include);
        frame.set(s_args,
                  make_varray(VarNR(btArgs.m_parserFrame->filename.get())));
      }
      bt.append(frame.toVariant());
      depth++;
    }
  }

  // Handle the subsequent VM frames.
  for (auto prev = getPrevActRec(ctx, frm, visitedWHs);
       frm && (btArgs.m_limit == 0 || depth < btArgs.m_limit);
       frm = prev, prev = getPrevActRec(ctx, frm, visitedWHs)) {
    auto const fp = frm.fp;

    // Do not capture frame for HPHP only functions.
    if (fp->func()->isNoInjection()) continue;

    DArrayInit frame(8);

    auto const curUnit = fp->func()->unit();

    // Builtins and generators don't have a file and line number.
    if (prev.fp && !prev.fp->func()->isBuiltin()) {
      auto const prevFunc = prev.fp->func();
      auto const prevFile = prevFunc->originalFilename()
        ? prevFunc->originalFilename()
        : prevFunc->unit()->filepath();

      assertx(prevFile != nullptr);
      frame.set(s_file, Variant{const_cast<StringData*>(prevFile)});
      frame.set(s_line, prevFunc->unit()->getLineNumber(prev.pc));
    }

    // Check for include.
    String funcname{const_cast<StringData*>(fp->func()->name())};
    if (fp->func()->isClosureBody()) {
      // Strip the file hash from the closure name.
      String fullName{const_cast<StringData*>(fp->func()->baseCls()->name())};
      funcname = fullName.substr(0, fullName.find(';'));
    }

    if (RuntimeOption::EnableArgsInBacktraces &&
        !fp->localsDecRefd() &&
        fp->func()->hasReifiedGenerics()) {
      // First local is always $0ReifiedGenerics which comes right after params
      auto const generics = frame_local(fp, fp->func()->numParams());
      if (type(generics) != KindOfUninit) {
        assertx(tvIsHAMSafeVArray(generics));
        auto const reified_generics = val(generics).parr;
        funcname += mangleReifiedGenericsName(reified_generics);
      }
    }

    frame.set(s_function, funcname);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context.
      auto ctx = arGetContextClass(fp);
      if (ctx != nullptr && !fp->func()->isClosureBody()) {
        String clsname{const_cast<StringData*>(ctx->name())};
        if (RuntimeOption::EnableArgsInBacktraces &&
            !fp->localsDecRefd() &&
            !fp->isInlined() &&
            ctx->hasReifiedGenerics() &&
            fp->hasThis()) {
          auto const reified_generics = getClsReifiedGenericsProp(ctx, fp);
          clsname += mangleReifiedGenericsName(reified_generics);
        }
        frame.set(s_class, clsname);
        if (!fp->localsDecRefd() &&
            !fp->isInlined() &&
            fp->hasThis() &&
            btArgs.m_withThis) {
          frame.set(s_object, Object(fp->getThis()));
        }
        frame.set(s_type, fp->func()->isStatic() ? s_double_colon : s_arrow);
      }
    }

    auto const withNames = btArgs.m_withArgNames;
    auto const withValues = btArgs.m_withArgValues;
    if ((!btArgs.m_withArgNames && !btArgs.m_withArgValues) ||
        !RuntimeOption::EnableArgsInBacktraces) {
      // do nothing
    } else if (funcname.same(s_include)) {
      auto filepath = const_cast<StringData*>(curUnit->filepath());
      frame.set(s_args, make_varray(filepath));
    } else if (fp->localsDecRefd()) {
      frame.set(s_args, empty_varray());
    } else {
      auto args = Array::CreateVArray();
      auto const nparams = fp->func()->numNonVariadicParams();

      for (int i = 0; i < nparams; i++) {
        Variant val =
          withValues ? Variant{variant_ref{frame_local(fp, i)}} : "";

        if (withNames) {
          auto const argname = fp->func()->localVarName(i);
          args.set(String(const_cast<StringData*>(argname)), val);
        } else {
          args.append(val);
        }
      }

      frame.set(s_args, args);
    }

    if (btArgs.m_withMetadata && !fp->localsDecRefd()) {
      auto local = fp->func()->lookupVarId(s_86metadata.get());
      if (local != kInvalidId) {
        auto const val = frame_local(fp, local);
        if (type(val) != KindOfUninit) {
          always_assert(tvIsPlausible(*val));
          frame.set(s_metadata, Variant{variant_ref{val}});
        }
      }
    }

    bt.append(frame.toVariant());
    depth++;
  }

  return bt;
}

void addBacktraceToStructLog(const Array& bt, StructuredLogEntry& cols) {
  std::set<std::string> strings;
  std::vector<folly::StringPiece> files;
  std::vector<folly::StringPiece> functions;
  std::vector<folly::StringPiece> lines;
  auto addString = [&] (std::string&& s) -> folly::StringPiece {
    return *strings.insert(std::move(s)).first;
  };
  auto addVariant = [&] (const Variant& v) -> folly::StringPiece {
    if (v.isString()) return v.asCStrRef().slice();
    return addString(v.toString().toCppString());
  };
  for (ArrayIter it(bt.get()); it; ++it) {
    Array frame = it.second().toArray();
    files.emplace_back(addVariant(frame[s_file]));
    if (frame.exists(s_class)) {
      functions.emplace_back(
        addString(folly::sformat("{}{}{}",
                                 frame[s_class].toString().data(),
                                 frame[s_type].toString().data(),
                                 frame[s_function].toString().data()
                                )
                 ));
    } else {
      functions.emplace_back(addVariant(frame[s_function]));
    }
    lines.emplace_back(addVariant(frame[s_line]));
  }
  cols.setVec("php_files", files);
  cols.setVec("php_functions", functions);
  cols.setVec("php_lines", lines);
}

///////////////////////////////////////////////////////////////////////////////

int64_t createBacktraceHash(bool consider_metadata) {
  // Settings constants before looping
  uint64_t hash = 0x9e3779b9;
  Unit* prev_unit = nullptr;

  walkStack([&] (ActRec* fp, Offset) {
    // Do not capture frame for HPHP only functions.
    if (fp->func()->isNoInjection()) return;

    auto const curFunc = fp->func();
    auto const curUnit = curFunc->unit();

    // Only do a filehash if the file changed. It is very common
    // to see sequences of calls within the same file
    // File paths are already hashed, and the hash bits are random enough
    // That allows us to do a faster combination of hashes using a known
    // implementation (boost::hash_combine)
    if (prev_unit != curUnit) {
      prev_unit = curUnit;
      auto filehash = curUnit->filepath()->hash();
      hash ^= filehash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    // Function names are already hashed, and the hash bits are random enough
    // That allows us to do a faster combination of hashes using a known
    // implementation (boost::hash_combine)
    auto funchash = curFunc->fullName()->hash();
    hash ^= funchash + 0x9e3779b9 + (hash << 6) + (hash >> 2);

    if (consider_metadata && !fp->localsDecRefd()) {
      tv_rval meta;
      auto local = fp->func()->lookupVarId(s_86metadata.get());
      if (local != kInvalidId) {
        auto const val = frame_local(fp, local);
        if (!isNullType(type(val))) meta = val;
      }

      if (meta) {
        hash ^= val(meta).num + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= static_cast<int>(type(meta))
          + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      }
    }
  });

  return hash;
}

void fillCompactBacktrace(CompactTraceData* trace, bool skipTop) {
  walkStack([trace] (ActRec* fp, Offset prevPc) {
    // Do not capture frame for HPHP only functions.
    if (fp->func()->isNoInjection()) return;
    trace->insert(fp, prevPc);
  }, skipTop);
}

req::ptr<CompactTrace> createCompactBacktrace(bool skipTop) {
  auto ret = req::make<CompactTrace>();
  fillCompactBacktrace(ret->get(), skipTop);
  return ret;
}

std::pair<const Func*, Offset> getCurrentFuncAndOffset() {
  VMRegAnchor _;
  folly::small_vector<c_WaitableWaitHandle*, 64> visitedWHs;
  BTContext ctx;

  auto frm = BTFrame { vmfp() };
  frm = initBTContextAt(ctx, vmJitReturnAddr(), frm);

  // Builtins don't have a file and line number, so find the first user frame
  while (frm && frm.fp->func()->isBuiltin()) {
    frm = getPrevActRec(ctx, frm, visitedWHs);
  }
  return std::make_pair(frm.fp ? frm.fp->func() : nullptr, frm.pc);
}

namespace {

struct CTKHasher final {
  uint64_t hash(const CompactTraceData::Ptr& k) const {
    if (!k) return 0;
    return k->hash();
  }
  bool equal(const CompactTraceData::Ptr& k1,
             const CompactTraceData::Ptr& k2) const;
};

struct CacheDeleter final {
  void operator()(ArrayData* ad) const {
    if (!ad->isUncounted()) return;
    Treadmill::enqueue([ad] {
      PackedArray::ReleaseUncounted(ad);
    });
  }
};

using CachedArray = std::shared_ptr<ArrayData>;
using Cache = ConcurrentScalableCache<CompactTraceData::Ptr,
                                      CachedArray,CTKHasher>;
Cache s_cache(1024);

bool CTKHasher::equal(const CompactTraceData::Ptr& k1,
                      const CompactTraceData::Ptr& k2) const {
  assertx(k1 && k2);
  if (k1->size() != k2->size() || k1->hash() != k2->hash()) {
    return false;
  }
  for (int i = 0; i < k1->frames().size(); ++i) {
    const auto& a = k1->frames()[i];
    const auto& b = k2->frames()[i];
    if (a.func != b.func || a.prevPcAndHasThis != b.prevPcAndHasThis) {
      return false;
    }
  }
  return true;
}

}

IMPLEMENT_RESOURCE_ALLOCATION(CompactTrace)

uint64_t CompactTraceData::hash() const {
  if (m_hash != 0) return m_hash;
  m_hash = 0xffffffff;
  for (auto frame : m_frames) {
    m_hash = hash_int64_pair(m_hash, frame.hash());
  }
  return m_hash;
}

void CompactTraceData::insert(const ActRec* fp, int32_t prevPc) {
  m_hash = 0;                           // invalidate

  m_frames.emplace_back(
    fp->func(),
    prevPc,
    !fp->localsDecRefd() && fp->func()->hasThisInBody()
  );
}

Array CompactTraceData::extract() const {
  VArrayInit aInit(m_frames.size());
  for (int idx = 0; idx < m_frames.size(); ++idx) {
    auto const prev = idx < m_frames.size() - 1 ? &m_frames[idx + 1] : nullptr;
    DArrayInit frame(6);
    if (prev && !prev->func->isBuiltin()) {
      auto const prevUnit = prev->func->unit();
      auto prevFile = prevUnit->filepath();
      if (prev->func->originalFilename()) {
        prevFile = prev->func->originalFilename();
      }

      auto const prevPc = prev->prevPc;
      frame.set(s_file, StrNR(prevFile).asString());
      frame.set(s_line, prevUnit->getLineNumber(prevPc));
    }

    auto const f = m_frames[idx].func;

    // Check for include.
    String funcname{const_cast<StringData*>(f->name())};
    if (f->isClosureBody()) {
      // Strip the file hash from the closure name.
      String fullName{const_cast<StringData*>(f->baseCls()->name())};
      funcname = fullName.substr(0, fullName.find(';'));
    }

    // Check for pseudomain.
    if (funcname.empty()) {
      if (!prev) continue;
      else funcname = s_include;
    }

    frame.set(s_function, funcname);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context.
      auto ctx = m_frames[idx].func->cls();
      if (ctx != nullptr && !f->isClosureBody()) {
        frame.set(s_class, Variant{const_cast<StringData*>(ctx->name())});
        if (m_frames[idx].hasThis) {
          frame.set(s_type, s_arrow);
        } else {
          frame.set(s_type, s_double_colon);
        }
      }
    } else {
      auto filepath = const_cast<StringData*>(f->unit()->filepath());
      frame.set(s_args, make_varray(filepath));
    }

    aInit.append(frame.toVariant());
  }

  return aInit.toArray();
}

Array CompactTrace::extract() const {
  if (size() <= 1) return Array::CreateVArray();

  Cache::ConstAccessor acc;
  if (s_cache.find(acc, m_backtrace)) {
    return Array(acc.get()->get());
  }

  auto arr = m_backtrace->extract();
  auto ins = CachedArray(
    arr.get()->empty()
      ? ArrayData::CreateVArray()
      : PackedArray::MakeUncounted(arr.get()),
    CacheDeleter()
  );
  if (!s_cache.insert(m_backtrace, ins)) {
    return arr;
  }
  return Array(ins.get());
}

///////////////////////////////////////////////////////////////////////////////

} // HPHP
