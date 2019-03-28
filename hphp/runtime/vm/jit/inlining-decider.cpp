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
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/util/arch.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

#include <folly/RWSpinLock.h>
#include <folly/Synchronized.h>
#include <cmath>
#include <vector>
#include <sstream>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(inlining);

namespace {
///////////////////////////////////////////////////////////////////////////////

std::string nameAndReason(int bcOff, std::string caller, std::string callee,
                          std::string why) {
  return folly::sformat("BC {}: {} -> {}: {}\n", bcOff, caller, callee, why);
}

bool traceRefusal(SrcKey callerSk, const Func* callee, std::string why,
                  Annotations* annotations) {
  // This is not under Trace::enabled so that we can collect the data in prod.
  const Func* caller = callerSk.func();
  int bcOff = callerSk.offset();
  auto calleeName = callee ? callee->fullName()->data() : "(unknown)";
  if (annotations && RuntimeOption::EvalDumpInlDecision > 0) {
    annotations->emplace_back("NoInline",
      nameAndReason(bcOff, caller->fullName()->data(), calleeName, why));
  }
  if (Trace::enabled) {
    assertx(caller);
    FTRACE(2, "InliningDecider: refusing {}() <- {}{}\t<reason: {}>\n",
           caller->fullName()->data(), calleeName, callee ? "()" : "", why);
  }
  if (caller->shouldSampleJit() || (callee && callee->shouldSampleJit())) {
    StructuredLogEntry inlLog;
    auto bcStr = [&] {
      std::ostringstream bcStrn;
      bcStrn << bcOff;
      return bcStrn.str();
    } ();
    inlLog.setStr("BC off", bcStr);
    inlLog.setStr("caller", caller->fullName()->data());
    inlLog.setStr("callee", calleeName);
    inlLog.setStr("reason", why);
    StructuredLog::log("hhvm_inline_refuse", inlLog);
  }
  return false;
}

std::atomic<uint64_t> s_baseProfCount{0};

///////////////////////////////////////////////////////////////////////////////
// canInlineAt() helpers.

const StaticString
  s_AlwaysInline("__ALWAYS_INLINE"),
  s_NeverInline("__NEVER_INLINE");

/*
 * Check if the funcd of `inst' has any characteristics which prevent inlining,
 * without peeking into its bytecode or regions.
 */
bool isCalleeInlinable(SrcKey callSK, const Func* callee,
                       Annotations* annotations) {
  assertx(callSK.op() == Op::FCall);
  auto refuse = [&] (const char* why) {
    return traceRefusal(callSK, callee, why, annotations);
  };

  if (!callee) {
    return refuse("callee not known");
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
    auto const numArgs = getImm(pc, 0).u_FCA.numArgs;
    auto const numParams = callee->numParams();
    if (numArgs >= numParams) {
      return refuse("callee has variadic capture with non-empty value");
    }
  }
  if (callee->isMagic()) {
    return refuse("magic callee");
  }
  if (callee->isGenerator()) {
    return refuse("callee is generator");
  }
  if (callee->maxStackCells() >= kStackCheckLeafPadding) {
    return refuse("function stack depth too deep");
  }
  if (callee->isMethod() && callee->cls() == Generator::getClass()) {
    return refuse("generator member function");
  }
  if (!RuntimeOption::EvalHHIRInliningIgnoreHints &&
      callee->userAttributes().count(s_NeverInline.get())) {
    return refuse("callee marked __NEVER_INLINE");
  }

  return true;
}

/*
 * Check that we don't have any missing or extra arguments.
 */
bool checkNumArgs(SrcKey callSK, const Func* callee, Annotations* annotations) {
  assertx(callSK.op() == Op::FCall);
  assertx(callee);

  auto refuse = [&] (const char* why) {
    return traceRefusal(callSK, callee, why, annotations);
  };

  auto pc = callSK.pc();
  auto const fca = getImm(pc, 0).u_FCA;
  auto const numParams = callee->numParams();

  if (fca.numArgs > numParams) {
    return refuse("callee called with too many arguments");
  }

  if (fca.hasUnpack()) {
    return refuse("callee called with variadic arguments");
  }

  if (fca.numRets != 1) {
    return refuse("callee with multiple returns");
  }

  if (fca.enforceReffiness()) {
    for (auto i = 0; i < fca.numArgs; ++i) {
      if (callee->byRef(i) != fca.byRef(i)) {
        return refuse("callee called with arguments of mismatched reffiness");
      }
    }
  }

  // It's okay if we passed fewer arguments than there are parameters as long
  // as the gap can be filled in by DV funclets.
  for (auto i = fca.numArgs; i < numParams; ++i) {
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

bool InliningDecider::canInlineAt(SrcKey callSK, const Func* callee,
                                  Annotations* annotations) const {
  assertx(callSK.op() == Op::FCall);

  if (m_disabled) {
    return traceRefusal(callSK, callee, "m_disabled", annotations);
  }
  if (!callee) {
    return traceRefusal(callSK, callee, "unknown callee", annotations);
  }
  if (!RuntimeOption::EvalHHIREnableGenTimeInlining) {
    return traceRefusal(callSK, callee, "disabled via runtime option",
                        annotations);
  }
  if (RuntimeOption::EvalJitEnableRenameFunction) {
    return traceRefusal(callSK, callee, "rename function is enabled",
                        annotations);
  }
  if (callee->attrs() & AttrInterceptable) {
    return traceRefusal(callSK, callee, "callee is interceptable", annotations);
  }
  if (callee->takesInOutParams()) {
    return traceRefusal(callSK, callee, "callee takes inout params",
                        annotations);
  }

  // TODO(#4238160): Inlining into pseudomain callsites is still buggy.
  if (callSK.func()->isPseudoMain()) {
    return traceRefusal(callSK, callee, "PseudoMain", annotations);
  }

  if (!isCalleeInlinable(callSK, callee, annotations) ||
      !checkNumArgs(callSK, callee, annotations)) {
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
    case Op::Array:
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
    case Op::VerifyParamTypeTS:
    case Op::VerifyRetTypeC:
    case Op::VerifyRetTypeTS:
    case Op::RetC:
    case Op::RetCSuspended:
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

  InlineRegionKey(InlineRegionKey&& irk) noexcept
    : entryKey(std::move(irk.entryKey))
    , ctxType(std::move(irk.ctxType))
  {
    for (auto ty : irk.argTypes) argTypes.push_back(ty);
    irk.argTypes.clear();
  }

  InlineRegionKey& operator=(const InlineRegionKey& irk) {
    entryKey = irk.entryKey;
    ctxType = irk.ctxType;
    argTypes.clear();
    for (auto ty : irk.argTypes) argTypes.push_back(ty);
    return *this;
  }

  InlineRegionKey& operator=(InlineRegionKey&& irk) noexcept {
    entryKey = irk.entryKey;
    ctxType = irk.ctxType;
    argTypes.clear();
    for (auto ty : irk.argTypes) argTypes.push_back(ty);
    irk.argTypes.clear();
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

using InlineCostCache = jit::fast_map<
  InlineRegionKey,
  unsigned,
  InlineRegionKey::Hash,
  InlineRegionKey::Eq
>;

Vcost computeTranslationCostSlow(SrcKey at, Op callerFPushOp,
                                 const RegionDesc& region,
                                 Annotations& annotations) {
  TransContext ctx {
    kInvalidTransID,
    TransKind::Optimize,
    TransFlags{},
    at,
    // We can pretend the stack is empty, but we at least need to account for
    // the locals, iters, and slots, etc.
    FPInvOffset{at.func()->numSlotsInFrame()},
    0,
    callerFPushOp
  };

  rqtrace::DisableTracing notrace;

  auto const unit = irGenInlineRegion(ctx, region, annotations);
  if (!unit) return {0, true};

  SCOPE_ASSERT_DETAIL("Inline-IRUnit") { return show(*unit); };
  return irlower::computeIRUnitCost(*unit);
}

folly::Synchronized<InlineCostCache, folly::RWSpinLock> s_inlCostCache;

int computeTranslationCost(SrcKey at, Op callerFPushOp,
                           const RegionDesc& region,
                           Annotations& annotations) {
  InlineRegionKey irk{region};
  SYNCHRONIZED_CONST(s_inlCostCache) {
    auto f = s_inlCostCache.find(irk);
    if (f != s_inlCostCache.end()) return f->second;
  }

  auto const info = computeTranslationCostSlow(at, callerFPushOp, region,
                                              annotations);
  auto cost = info.cost;

  // We normally store the computed cost into the cache.  However, if the region
  // is incomplete, and it's cost is still within the maximum allowed cost, and
  // we're still profiling that function, then we don't want to cache that
  // result yet.  The reason for this exception is that we may still gather
  // additional profiling information that will allow us to create a complete
  // region with acceptable cost.
  bool cacheResult = true;

  if (info.incomplete) {
    if (info.cost <= RuntimeOption::EvalHHIRInliningMaxVasmCostLimit) {
      auto const fid = region.entry()->func()->getFuncId();
      auto const profData = jit::profData();
      auto const profiling = profData && profData->profiling(fid);
      if (profiling) cacheResult = false;
    }

    // Set cost very high to prevent inlining of incomplete regions.
    cost = std::numeric_limits<int>::max();
  }

  if (cacheResult && !as_const(s_inlCostCache)->count(irk)) {
    s_inlCostCache->emplace(irk, cost);
  }
  FTRACE(3, "computeTranslationCost(at {}) = {}\n", showShort(at), cost);
  return cost;
}

uint64_t adjustedMaxVasmCost(const irgen::IRGS& env,
                             const RegionDesc& calleeRegion) {
  const auto baseVasmCost = RuntimeOption::EvalHHIRInliningVasmCostLimit;
  const auto baseProfCount = s_baseProfCount.load();
  if (baseProfCount == 0) return baseVasmCost;
  auto const callerProfCount = irgen::curProfCount(env);
  auto adjustedCost = baseVasmCost *
    std::pow((double)callerProfCount / baseProfCount,
             RuntimeOption::EvalHHIRInliningVasmCallerExp);
  auto const calleeProfCount = irgen::calleeProfCount(env, calleeRegion);
  if (calleeProfCount) {
    adjustedCost *= std::pow((double)callerProfCount / calleeProfCount,
                             RuntimeOption::EvalHHIRInliningVasmCalleeExp);
  }
  if (adjustedCost < RuntimeOption::EvalHHIRInliningMinVasmCostLimit) {
    adjustedCost = RuntimeOption::EvalHHIRInliningMinVasmCostLimit;
  }
  if (adjustedCost > RuntimeOption::EvalHHIRInliningMaxVasmCostLimit) {
    adjustedCost = RuntimeOption::EvalHHIRInliningMaxVasmCostLimit;
  }
  if (calleeProfCount) {
    FTRACE(3, "adjustedMaxVasmCost: adjustedCost ({}) = baseVasmCost ({}) * "
           "(callerProfCount ({}) / baseProfCount ({})) ^ {} * "
           "(callerProfCount ({}) / calleeProfCount ({})) ^ {}\n",
           adjustedCost, baseVasmCost,
           callerProfCount, baseProfCount,
           RuntimeOption::EvalHHIRInliningVasmCallerExp,
           callerProfCount, calleeProfCount,
           RuntimeOption::EvalHHIRInliningVasmCalleeExp);
  } else {
    FTRACE(3, "adjustedMaxVasmCost: adjustedCost ({}) = baseVasmCost ({}) * "
           "(callerProfCount ({}) / baseProfCount ({})) ^ {}\n",
           adjustedCost, baseVasmCost,
           callerProfCount, baseProfCount,
           RuntimeOption::EvalHHIRInliningVasmCallerExp);
  }
  return adjustedCost;
}

///////////////////////////////////////////////////////////////////////////////
}

/*
 * Update context for start of inlining.
 */
int InliningDecider::accountForInlining(SrcKey callerSk,
                                        Op callerFPushOp,
                                        const Func* callee,
                                        const RegionDesc& region,
                                        const irgen::IRGS& irgs,
                                        Annotations& annotations) {
  auto const alwaysInl =
    !RuntimeOption::EvalHHIRInliningIgnoreHints &&
    callee->userAttributes().count(s_AlwaysInline.get());

  // Functions marked as always inline don't contribute to overall cost
  int cost = alwaysInl
    ? 0
    : computeTranslationCost(callerSk, callerFPushOp, region, annotations);

  m_costStack.push_back(cost);
  m_cost       += cost;
  m_callDepth  += 1;
  m_stackDepth += callee->maxStackCells();

  return cost;
}

void InliningDecider::initWithCallee(const Func* callee) {
  m_costStack.push_back(0);
  m_callDepth  += 1;
  m_stackDepth += callee->maxStackCells();
}

bool InliningDecider::shouldInline(const irgen::IRGS& irgs,
                                   SrcKey callerSk,
                                   Op callerFPushOp,
                                   const Func* callee,
                                   const RegionDesc& region,
                                   uint32_t maxTotalCost,
                                   Annotations& annotations) {
  auto sk = region.empty() ? SrcKey() : region.start();
  assertx(callee);
  assertx(sk.func() == callee);

  auto annotationsPtr = mcgen::dumpTCAnnotation(irgs.context.kind) ?
                        &annotations : nullptr;
  // Tracing return lambdas.
  auto refuse = [&] (const std::string& why) {
    FTRACE(2, "shouldInline: rejecting callee region: {}", show(region));
    return traceRefusal(callerSk, callee, why, annotationsPtr);
  };

  auto accept = [&, this] (std::string why) {
    if (annotationsPtr && RuntimeOption::EvalDumpInlDecision >= 2) {
      auto str = nameAndReason(callerSk.offset(),
                               callerSk.func()->fullName()->data(),
                               callee->fullName()->data(),
                               why);
      annotationsPtr->emplace_back("DoInline", str);
    }

    FTRACE(2, "InliningDecider: inlining {}() <- {}()\t<reason: {}>\n",
           m_topFunc->fullName()->data(), callee->fullName()->data(), why);
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

  auto isAwaitish = [&] (Op opcode) {
    return opcode == OpAwait || opcode == OpAwaitAll;
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

      // RetM is currently not supported in the callee region.
      if (op == OpRetM) {
        return refuse("RetM is not supported");
      }

      // Detect that the region contains a return.
      if (isReturnish(op)) {
        hasRet = true;
      }

      // In optimized regions consider an await to be a returnish instruction,
      // if no returns appeared in the region then we likely suspend on all
      // calls to the callee.
      if (block->profTransID() != kInvalidTransID) {
        if (region.isExit(block->id()) && i + 1 == n && isAwaitish(op)) {
          hasRet = true;
        }
      }
    }
  }

  if (!hasRet) {
    return refuse(
      folly::sformat("region has no returns: callee BC instrs = {} : {}",
                     region.instrSize(), show(region)));
  }

  // Ignore cost computation for functions marked __ALWAYS_INLINE
  if (!RuntimeOption::EvalHHIRInliningIgnoreHints &&
      callee->userAttributes().count(s_AlwaysInline.get())) {
    return accept("callee marked as __ALWAYS_INLINE");
  }

  // Refuse if the cost exceeds our thresholds.
  // We measure the cost of inlining each callstack and stop when it exceeds a
  // certain threshold.  (Note that we do not measure the total cost of all the
  // inlined calls for a given caller---just the cost of each nested stack.)
  const int cost = computeTranslationCost(callerSk, callerFPushOp, region,
                                          annotations);
  if (cost <= RuntimeOption::EvalHHIRAlwaysInlineVasmCostLimit) {
    return accept(folly::sformat("cost={} within always-inline limit", cost));
  }

  const int maxCost = maxTotalCost - m_cost;
  const auto baseProfCount = s_baseProfCount.load();
  const auto callerProfCount = irgen::curProfCount(irgs);
  const auto calleeProfCount = irgen::calleeProfCount(irgs, region);
  if (cost > maxCost) {
    return refuse(folly::sformat(
      "too expensive: cost={} : maxTotalCost={} : maxCost={} : "
      "baseProfCount={} : callerProfCount={} : calleeProfCount={}", cost,
      maxTotalCost, maxCost, baseProfCount, callerProfCount, calleeProfCount));
  }

  return accept(folly::sformat("small region with return: cost={} : "
                               "maxTotalCost={} : maxCost={} : baseProfCount={}"
                               " : callerProfCount={} : calleeProfCount={}",
                               cost, maxTotalCost, maxCost, baseProfCount,
                               callerProfCount, calleeProfCount));
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
    ResumeMode::None,
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

RegionDescPtr selectCalleeCFG(SrcKey callerSk, const Func* callee,
                              const int numArgs,
                              Type ctxType, std::vector<Type>& argTypes,
                              int32_t maxBCInstrs,
                              Annotations* annotations) {
  auto const profData = jit::profData();
  if (!profData) {
    traceRefusal(callerSk, callee, "no profData", annotations);
    return nullptr;
  }

  if (!profData->profiling(callee->getFuncId())) {
    traceRefusal(callerSk, callee,
                 folly::sformat("no profiling data for callee FuncId: {}",
                                callee->getFuncId()),
                 annotations);
    return nullptr;
  }

  auto const dvID = findTransIDForCallee(profData, callee,
                                         numArgs, ctxType, argTypes);

  if (dvID == kInvalidTransID) {
    traceRefusal(callerSk, callee, "didn't find entry TransID for callee",
                 annotations);
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

  bool truncated = false;
  auto r = selectHotCFG(ctx, &truncated);
  if (truncated) {
    traceRefusal(callerSk, callee, "callee region truncated due to BC size",
                 annotations);
    return nullptr;
  }
  if (r) {
    r->setInlineContext(ctxType, argTypes);
  } else {
    traceRefusal(callerSk, callee, "failed selectHotCFG for callee",
                 annotations);
  }
  return r;
}
}

RegionDescPtr selectCalleeRegion(const SrcKey& sk,
                                 const Func* callee,
                                 const irgen::IRGS& irgs,
                                 InliningDecider& inl,
                                 int32_t maxBCInstrs,
                                 Annotations& annotations) {
  assertx(sk.op() == OpFCall);
  auto const numArgs = getImm(sk.pc(), 0).u_FCA.numArgs;
  auto const& fpiStack = irgs.irb->fs().fpiStack();
  assertx(!fpiStack.empty());
  auto const& fpiInfo = fpiStack.back();
  auto ctx = fpiInfo.ctxType;

  auto kind = irgs.context.kind;
  auto annotationsPtr = mcgen::dumpTCAnnotation(kind) ? &annotations : nullptr;

  if (ctx == TBottom) {
    traceRefusal(sk, callee, "ctx is TBottom", annotationsPtr);
    return nullptr;
  }
  if (callee->isClosureBody()) {
    if (!callee->cls()) {
      ctx = TNullptr;
    } else if (callee->mayHaveThis()) {
      ctx = TCtx;
    } else {
      ctx = TCctx;
    }
  } else {
    // Bail out if calling a static methods with an object ctx.
    if (ctx.maybe(TObj) &&
        (callee->isStaticInPrologue() ||
         (!sk.hasThis() && isFPushClsMethod(fpiInfo.fpushOpc)))) {
      traceRefusal(sk, callee, "calling static method with an object",
                   annotationsPtr);
      return nullptr;
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
      traceRefusal(sk, callee, folly::sformat("maybe boxed arg num: {}", i),
                   annotationsPtr);
      return nullptr;
    }
    argTypes.push_back(type);
  }

  const auto mode = RuntimeOption::EvalInlineRegionMode;

  if (mode == "cfg" || mode == "both") {
    if (profData()) {
      auto region = selectCalleeCFG(sk, callee, numArgs, ctx, argTypes,
                                    maxBCInstrs, annotationsPtr);
      if (region &&
          inl.shouldInline(irgs, sk, fpiInfo.fpushOpc, callee, *region,
                           adjustedMaxVasmCost(irgs, *region), annotations)) {
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

  if (region &&
      inl.shouldInline(irgs, sk, fpiInfo.fpushOpc, callee, *region,
                       adjustedMaxVasmCost(irgs, *region), annotations)) {
    return region;
  }

  return nullptr;
}

void setBaseInliningProfCount(uint64_t value) {
  s_baseProfCount.store(value);
  FTRACE(1, "setBaseInliningProfCount: {}\n", value);
}

///////////////////////////////////////////////////////////////////////////////
}}
