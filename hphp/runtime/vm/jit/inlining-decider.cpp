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

#include "hphp/runtime/vm/jit/inlining-decider.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/util/arch.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

#include <folly/RWSpinLock.h>
#include <folly/Synchronized.h>
#include <vector>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(inlining);

namespace {
///////////////////////////////////////////////////////////////////////////////

bool traceRefusal(const Func* caller, const Func* callee, const char* why) {
  if (Trace::enabled) {
    UNUSED auto calleeName = callee ? callee->fullName()->data()
                                    : "(unknown)";
    assertx(caller);

    FTRACE(2, "InliningDecider: refusing {}() <- {}{}\t<reason: {}>\n",
           caller->fullName()->data(), calleeName, callee ? "()" : "", why);
  }
  return false;
}

std::atomic<bool> hasCalledDisableInliningIntrinsic;
hphp_hash_set<const StringData*,
              string_data_hash,
              string_data_isame> forbiddenInlinees;
SimpleMutex forbiddenInlineesLock;

bool inliningIsForbiddenFor(const Func* callee) {
  if (!hasCalledDisableInliningIntrinsic.load()) return false;
  SimpleLock locker(forbiddenInlineesLock);
  return forbiddenInlinees.find(callee->fullName()) != forbiddenInlinees.end();
}

///////////////////////////////////////////////////////////////////////////////
// canInlineAt() helpers.

/*
 * Check if the funcd of `inst' has any characteristics which prevent inlining,
 * without peeking into its bytecode or regions.
 */
bool isCalleeInlinable(SrcKey callSK, const Func* callee) {
  auto refuse = [&] (const char* why) {
    return traceRefusal(callSK.func(), callee, why);
  };

  if (!callee) {
    return refuse("callee not known");
  }
  if (inliningIsForbiddenFor(callee)) {
    return refuse("inlining disabled for callee");
  }
  if (callee == callSK.func()) {
    return refuse("call is recursive");
  }
  if (callee->hasVariadicCaptureParam()) {
    if (callee->attrs() & AttrMayUseVV) {
      return refuse("callee has variadic capture and MayUseVV");
    }
    // Refuse if the variadic parameter actually captures something.
    auto pc = callSK.pc();
    auto const numArgs = getImm(pc, 0).u_IVA;
    auto const numParams = callee->numParams();
    if (numArgs >= numParams) {
      return refuse("callee has variadic capture with non-empty value");
    }
  }
  if (callee->isMagic()) {
    return refuse("magic callee");
  }
  if (callee->isResumable()) {
    return refuse("callee is resumable");
  }
  if (callee->maxStackCells() >= kStackCheckLeafPadding) {
    return refuse("function stack depth too deep");
  }
  if (callee->isMethod() && callee->cls() == Generator::getClass()) {
    return refuse("generator member function");
  }
  return true;
}

/*
 * Check that we don't have any missing or extra arguments.
 */
bool checkNumArgs(SrcKey callSK, const Func* callee) {
  assertx(callee);

  auto refuse = [&] (const char* why) {
    return traceRefusal(callSK.func(), callee, why);
  };

  auto pc = callSK.pc();
  auto const numArgs = getImm(pc, 0).u_IVA;
  auto const numParams = callee->numParams();

  if (numArgs > numParams) {
    return refuse("callee called with too many arguments");
  }

  // It's okay if we passed fewer arguments than there are parameters as long
  // as the gap can be filled in by DV funclets.
  for (auto i = numArgs; i < numParams; ++i) {
    auto const& param = callee->params()[i];
    if (!param.hasDefaultValue() &&
        (i < numParams - 1 || !callee->hasVariadicCaptureParam())) {
      return refuse("callee called with too few arguments");
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}

void InliningDecider::forbidInliningOf(const Func* callee) {
  hasCalledDisableInliningIntrinsic.store(true);
  SimpleLock locker(forbiddenInlineesLock);
  forbiddenInlinees.insert(callee->fullName());
}

bool InliningDecider::canInlineAt(SrcKey callSK, const Func* callee) const {
  if (m_disabled ||
      !callee ||
      !RuntimeOption::EvalHHIREnableGenTimeInlining ||
      RuntimeOption::EvalJitEnableRenameFunction ||
      callee->attrs() & AttrInterceptable) {
    return false;
  }

  // We can only inline at normal FCalls.
  if (callSK.op() != Op::FCall &&
      callSK.op() != Op::FCallD) {
    return false;
  }

  // Don't inline from resumed functions.  The inlining mechanism doesn't have
  // support for these---it has no way to redefine stack pointers relative to
  // the frame pointer, because in a resumed function the frame pointer points
  // into the heap instead of into the eval stack.
  if (callSK.resumed()) return false;

  // TODO(#4238160): Inlining into pseudomain callsites is still buggy.
  if (callSK.func()->isPseudoMain()) return false;

  if (!isCalleeInlinable(callSK, callee) || !checkNumArgs(callSK, callee)) {
    return false;
  }

  return true;
}

namespace {
///////////////////////////////////////////////////////////////////////////////
// shouldInline() helpers.

/*
 * Check if a builtin is inlinable.
 */
bool isInlinableCPPBuiltin(const Func* f) {
  assertx(f->isCPPBuiltin());

  // The callee needs to be callable with FCallBuiltin, because NativeImpl
  // requires a frame.
  if (!RuntimeOption::EvalEnableCallBuiltin ||
      (f->attrs() & AttrNoFCallBuiltin) ||
      (f->numParams() > Native::maxFCallBuiltinArgs()) ||
      !f->nativeFuncPtr()) {
    return false;
  }

  // ARM currently can't handle floating point returns.
  if (f->hniReturnType() == KindOfDouble &&
      !Native::allowFCallBuiltinDoubles()) {
    return false;
  }

  return true;
}

/*
 * Conservative whitelist for HHBC opcodes we know are safe to inline, even if
 * the entire callee body required a AttrMayUseVV.
 *
 * This affects cases where we're able to eliminate control flow while inlining
 * due to the parameter types, and the AttrMayUseVV flag was due to something
 * happening in a block we won't inline.
 */
bool isInliningVVSafe(Op op) {
  switch (op) {
    case Op::Null:
    case Op::PopC:
    case Op::PopL:
    case Op::CGetL:
    case Op::SetL:
    case Op::IsTypeL:
    case Op::JmpNS:
    case Op::JmpNZ:
    case Op::JmpZ:
    case Op::AssertRATL:
    case Op::AssertRATStk:
    case Op::VerifyParamType:
    case Op::VerifyRetTypeC:
    case Op::RetC:
      return true;
    default:
      break;
  }
  return false;
}

struct InlineRegionKey {
  explicit InlineRegionKey(const RegionDesc& region)
    : entryKey(region.entry()->start())
    , ctxType(region.inlineCtxType())
  {
    for (auto const ty : region.inlineInputTypes()) {
      argTypes.push_back(ty);
    }
  }

  InlineRegionKey(const InlineRegionKey& irk)
    : entryKey(irk.entryKey)
    , ctxType(irk.ctxType)
  {
    for (auto ty : irk.argTypes) argTypes.push_back(ty);
  }

  InlineRegionKey& operator=(const InlineRegionKey& irk) {
    entryKey = irk.entryKey;
    ctxType = irk.ctxType;
    argTypes.clear();
    for (auto ty : irk.argTypes) argTypes.push_back(ty);
    return *this;
  }

  struct Eq {
    size_t operator()(const InlineRegionKey& k1,
                      const InlineRegionKey& k2) const {
      return
        k1.entryKey == k2.entryKey &&
        k1.ctxType == k2.ctxType &&
        k1.argTypes == k2.argTypes;
    }
  };

  struct Hash {
    size_t operator()(const InlineRegionKey& key) const {
      size_t h = 0;
      h = hash_combine(h, key.entryKey.toAtomicInt());
      h = hash_combine(h, key.ctxType.hash());
      for (auto const ty : key.argTypes) {
        h = hash_combine(h, ty.hash());
      }
      return h;
    }

  private:
    template<class T>
    static size_t hash_combine(size_t base, T other) {
      return folly::hash::hash_128_to_64(
          base, folly::hash::hash_combine(other));
    }
  };

  SrcKey entryKey;
  Type ctxType;
  TinyVector<Type, 4> argTypes;
};

using InlineCostCache = std::unordered_map<
  InlineRegionKey,
  unsigned,
  InlineRegionKey::Hash,
  InlineRegionKey::Eq
>;

using RegionKeySet = std::unordered_set<
  InlineRegionKey,
  InlineRegionKey::Hash,
  InlineRegionKey::Eq
>;

Vcost computeTranslationCostSlow(SrcKey at, Op callerFPushOp,
                                 const RegionDesc& region) {
  TransContext ctx {
    kInvalidTransID,
    TransKind::Optimize,
    TransFlags{},
    at,
    // We can pretend the stack is empty, but we at least need to account for
    // the locals, iters, and slots, etc.
    FPInvOffset{at.func()->numSlotsInFrame()},
    callerFPushOp
  };

  auto const unit = irGenInlineRegion(ctx, region);
  if (!unit) return {0, true};

  SCOPE_ASSERT_DETAIL("Inline-IRUnit") { return show(*unit); };
  return irlower::computeIRUnitCost(*unit);
}

folly::Synchronized<InlineCostCache, folly::RWSpinLock> s_inlCostCache;

int computeTranslationCost(SrcKey at, Op callerFPushOp,
                           const RegionDesc& region) {
  InlineRegionKey irk{region};
  SYNCHRONIZED_CONST(s_inlCostCache) {
    auto f = s_inlCostCache.find(irk);
    if (f != s_inlCostCache.end()) return f->second;
  }

  auto const info = computeTranslationCostSlow(at, callerFPushOp, region);
  auto cost = info.cost;

  // If the region wasn't complete, don't cache the result, unless we already
  // know it will be too expensive, or we've stopped profiling it
  auto const maxCost = RuntimeOption::EvalHHIRInliningMaxVasmCost;
  if (info.incomplete) {
    auto const fid = region.entry()->func()->getFuncId();
    auto const profData = jit::profData();
    auto const profiling = profData && profData->profiling(fid);
    cost = std::numeric_limits<int>::max();
    if (profiling && info.cost <= maxCost) return cost;
  }

  if (!s_inlCostCache.asConst()->count(irk)) {
    s_inlCostCache->emplace(irk, cost);
  }
  return cost;
}

///////////////////////////////////////////////////////////////////////////////
}

/*
 * Update context for start of inlining.
 */
void InliningDecider::accountForInlining(SrcKey callerSk,
                                         Op callerFPushOp,
                                         const Func* callee,
                                         const RegionDesc& region) {
  int cost = computeTranslationCost(callerSk, callerFPushOp, region);
  m_costStack.push_back(cost);
  m_cost       += cost;
  m_callDepth  += 1;
  m_stackDepth += callee->maxStackCells();
}

void InliningDecider::initWithCallee(const Func* callee) {
  m_costStack.push_back(0);
  m_callDepth  += 1;
  m_stackDepth += callee->maxStackCells();
}

bool InliningDecider::shouldInline(SrcKey callerSk,
                                   Op callerFPushOp,
                                   const Func* callee,
                                   const RegionDesc& region,
                                   uint32_t maxTotalCost) {
  auto sk = region.empty() ? SrcKey() : region.start();
  assertx(callee);
  assertx(sk.func() == callee);

  // Tracing return lambdas.
  auto refuse = [&] (const char* why) {
    FTRACE(2, "shouldInline: rejecting callee region: {}", show(region));
    return traceRefusal(m_topFunc, callee, why);
  };

  auto accept = [&, this] (const char* kind) {
    FTRACE(2, "InliningDecider: inlining {}() <- {}()\t<reason: {}>\n",
           m_topFunc->fullName()->data(), callee->fullName()->data(), kind);
    return true;
  };

  if (m_stackDepth + callee->maxStackCells() >= kStackCheckLeafPadding) {
    return refuse("inlining stack depth limit exceeded");
  }

  // Even if the func contains NativeImpl we may have broken the trace before
  // we hit it.
  auto containsNativeImpl = [&] {
    for (auto block : region.blocks()) {
      if (!block->empty() && block->last().op() == OpNativeImpl) return true;
    }
    return false;
  };

  // Try to inline CPP builtin functions.
  // The NativeImpl opcode may appear later in the function because of Asserts
  // generated in hhbbc
  if (callee->isCPPBuiltin() && containsNativeImpl()) {
    if (isInlinableCPPBuiltin(callee)) {
      return accept("inlinable CPP builtin");
    }
    return refuse("non-inlinable CPP builtin");
  }

  // If the function may use a VarEnv (which is stored in the ActRec) or may be
  // variadic, we restrict inlined callees to certain whitelisted instructions
  // which we know won't actually require these features.
  const bool needsCheckVVSafe = callee->attrs() & AttrMayUseVV;

  bool hasRet = false;

  // Iterate through the region, checking its suitability for inlining.
  for (auto const& block : region.blocks()) {
    sk = block->start();

    for (auto i = 0, n = block->length(); i < n; ++i, sk.advance()) {
      auto op = sk.op();

      // We don't allow inlined functions in the region.  The client is
      // expected to disable inlining for the region it gives us to peek.
      if (sk.func() != callee) {
        return refuse("got region with inlined calls");
      }

      // Restrict to VV-safe opcodes if necessary.
      if (needsCheckVVSafe && !isInliningVVSafe(op)) {
        return refuse(folly::format("{} may use dynamic environment",
                                    opcodeToName(op)).str().c_str());
      }

      // Count the returns.
      if (isReturnish(op)) {
        hasRet = true;
      }

      // We can't inline FCallArray.  XXX: Why?
      if (op == Op::FCallArray) {
        return refuse("can't inline FCallArray");
      }
    }
  }

  if (!hasRet) {
    return refuse("region has no returns");
  }

  // Refuse if the cost exceeds our thresholds.
  // We measure the cost of inlining each callstack and stop when it exceeds a
  // certain threshold.  (Note that we do not measure the total cost of all the
  // inlined calls for a given caller---just the cost of each nested stack.)
  const int maxCost = maxTotalCost - m_cost;
  const int cost = computeTranslationCost(callerSk, callerFPushOp, region);
  if (cost > maxCost) {
    return refuse("too expensive");
  }

  return accept("small region with return");
}

///////////////////////////////////////////////////////////////////////////////

void InliningDecider::registerEndInlining(const Func* callee) {
  auto cost = m_costStack.back();
  m_costStack.pop_back();

  m_cost -= cost;
  m_callDepth -= 1;
  m_stackDepth -= callee->maxStackCells();
}

namespace {
RegionDescPtr selectCalleeTracelet(const Func* callee,
                                   const int numArgs,
                                   Type ctxType,
                                   std::vector<Type>& argTypes,
                                   int32_t maxBCInstrs) {
  auto const numParams = callee->numParams();

  bool hasThis;
  if (ctxType <= TObj || !ctxType.maybe(TObj)) {
    hasThis = ctxType <= TObj;
  } else if (!callee->hasThisVaries()) {
    hasThis = callee->mayHaveThis();
  } else {
    return RegionDescPtr{};
  }

  // Set up the RegionContext for the tracelet selector.
  RegionContext ctx{
    callee, callee->getEntryForNumArgs(numArgs),
    FPInvOffset{safe_cast<int32_t>(callee->numSlotsInFrame())},
    false,
    hasThis
  };

  for (uint32_t i = 0; i < numArgs; ++i) {
    auto type = argTypes[i];
    assertx(type <= TGen);
    ctx.liveTypes.push_back({Location::Local{i}, type});
  }

  for (unsigned i = numArgs; i < numParams; ++i) {
    // These locals will be populated by DV init funclets but they'll start out
    // as Uninit.
    ctx.liveTypes.push_back({Location::Local{i}, TUninit});
  }

  // Produce a tracelet for the callee.
  auto r = selectTracelet(
    ctx,
    TransKind::Live,
    maxBCInstrs,
    true /* inlining */
  );
  if (r) {
    r->setInlineContext(ctxType, argTypes);
  }
  return r;
}

TransID findTransIDForCallee(const ProfData* profData,
                             const Func* callee, const int numArgs,
                             Type ctxType, std::vector<Type>& argTypes) {
  auto const idvec = profData->funcProfTransIDs(callee->getFuncId());

  auto const offset = callee->getEntryForNumArgs(numArgs);
  TransID ret = kInvalidTransID;
  bool hasThisVaries = callee->hasThisVaries() &&
    ctxType.maybe(TObj) && !(ctxType <= TObj);
  for (auto const id : idvec) {
    auto const rec = profData->transRec(id);
    if (rec->startBcOff() != offset) continue;
    auto const region = rec->region();

    auto const isvalid = [&] () {
      if (!hasThisVaries &&
          (rec->srcKey().hasThis() != ctxType.maybe(TObj))) {
        return false;
      }
      for (auto const& typeloc : region->entry()->typePreConditions()) {
        if (typeloc.location.tag() != LTag::Local) continue;
        auto const locId = typeloc.location.localId();

        if (locId < numArgs && !(argTypes[locId] <= typeloc.type)) {
          return false;
        }
      }
      return true;
    }();

    if (!isvalid) continue;
    if (!hasThisVaries) return id;
    // The function may be called with or without $this, if we've seen
    // both, give up.
    if (ret != kInvalidTransID) return kInvalidTransID;
    ret = id;
  }
  return ret;
}

RegionDescPtr selectCalleeCFG(const Func* callee, const int numArgs,
                              Type ctxType, std::vector<Type>& argTypes,
                              int32_t maxBCInstrs) {
  auto const profData = jit::profData();
  if (!profData || !profData->profiling(callee->getFuncId())) return nullptr;

  auto const dvID = findTransIDForCallee(profData, callee,
                                         numArgs, ctxType, argTypes);

  if (dvID == kInvalidTransID) {
    return nullptr;
  }

  TransCFG cfg(callee->getFuncId(), profData, true /* inlining */);

  HotTransContext ctx;
  ctx.tid = dvID;
  ctx.cfg = &cfg;
  ctx.profData = profData;
  ctx.maxBCInstrs = maxBCInstrs;
  ctx.inlining = true;
  ctx.inputTypes = &argTypes;

  auto r = selectHotCFG(ctx);
  if (r) {
    r->setInlineContext(ctxType, argTypes);
  }
  return r;
}
}

RegionDescPtr selectCalleeRegion(const SrcKey& sk,
                                 const Func* callee,
                                 const irgen::IRGS& irgs,
                                 InliningDecider& inl,
                                 int32_t maxBCInstrs) {
  auto const op = sk.pc();
  auto const numArgs = getImm(op, 0).u_IVA;

  auto const& fpiStack = irgs.irb->fs().fpiStack();
  assertx(!fpiStack.empty());
  auto const& fpiInfo = fpiStack.back();
  auto ctx = fpiInfo.ctxType;

  if (ctx == TBottom) return nullptr;
  if (callee->isClosureBody()) {
    if (!callee->cls()) {
      ctx = TNullptr;
    } else if (callee->mayHaveThis()) {
      ctx = TCtx;
    } else {
      ctx = TCctx;
    }
  }

  std::vector<Type> argTypes;
  for (int i = numArgs - 1; i >= 0; --i) {
    // DataTypeGeneric is used because we're just passing the locals into the
    // callee.  It's up to the callee to constrain further if needed.
    auto type = irgen::publicTopType(irgs, BCSPRelOffset{i});
    assertx(type <= TGen);

    // If we don't have sufficient type information to inline the region return
    // early
    if (type == TBottom) return nullptr;
    if (!(type <= TCell) && !(type <= TBoxedCell)) {
      return nullptr;
    }
    argTypes.push_back(type);
  }

  const auto mode = RuntimeOption::EvalInlineRegionMode;

  if (mode == "cfg" || mode == "both") {
    if (profData()) {
      auto region = selectCalleeCFG(callee, numArgs, ctx, argTypes,
                                    maxBCInstrs);
      auto const maxCost = RuntimeOption::EvalHHIRInliningMaxVasmCost;
      if (region && inl.shouldInline(sk, fpiInfo.fpushOpc,
                                     callee, *region, maxCost)) {
        return region;
      }
    }

    if (mode == "cfg") return nullptr;
  }

  auto region = selectCalleeTracelet(
    callee,
    numArgs,
    ctx,
    argTypes,
    maxBCInstrs
  );

  auto const maxCost = RuntimeOption::EvalHHIRInliningMaxVasmCost;
  if (region && inl.shouldInline(sk, fpiInfo.fpushOpc,
                                 callee, *region, maxCost)) {
    return region;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}}
