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

#include "hphp/runtime/vm/jit/inlining-decider.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/util/trace.h"

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

    FTRACE(1, "InliningDecider: refusing {}() <- {}{}\t<reason: {}>\n",
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
  if (callee->numIterators() != 0) {
    return refuse("callee has iterators");
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
  if (!callee || !RuntimeOption::EvalHHIREnableGenTimeInlining) {
    return false;
  }

  assert(!RuntimeOption::EvalJitEnableRenameFunction);
  if (callee->cls()) {
    if (!rds::isPersistentHandle(callee->cls()->classHandle())) {
      // if the callee's class is not persistent, its still ok
      // to use it if we're jitting into a method of a subclass
      auto ctx = callSK.func()->cls();
      if (!ctx || !ctx->classof(callee->cls())) {
        return false;
      }
    }
  } else {
    if (!rds::isPersistentHandle(callee->funcHandle())) {
      // if the callee isn't persistent, its still ok to
      // use it if its defined at the top level in the same
      // unit as the caller
      if (callee->unit() != callSK.unit() || !callee->top()) {
        return false;
      }
    }
  }

  // If inlining was disabled... don't inline.
  if (m_disabled) return false;

  // TODO(#3331014): We have this hack until more ARM codegen is working.
  if (arch() == Arch::ARM) return false;

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
  if (f->returnType() == KindOfDouble &&
      !Native::allowFCallBuiltinDoubles()) {
    return false;
  }

  if (auto const info = f->methInfo()) {
    if (info->attribute & (ClassInfo::NoFCallBuiltin |
                           ClassInfo::VariableArguments |
                           ClassInfo::RefVariableArguments)) {
      return false;
    }
    // Note: there is no need for a similar-to-the-above check for HNI
    // builtins---they'll just have a nullptr nativeFuncPtr (if they were
    // declared as needing an ActRec).
  }

  // For now, don't inline when we'd need to adjust ObjectData pointers.
  if (f->cls() && f->cls()->preClass()->builtinODOffset() != 0) {
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

/*
 * Opcodes that don't contribute to the inlining cost (i.e. produce no codegen).
 */
bool isFreeOp(Op op) {
  switch (op) {
  case Op::AssertRATL:
  case Op::AssertRATStk:
  case Op::BoxRNop:
  case Op::Nop:
  case Op::RGetCNop:
  case Op::UnboxRNop:
    return true;
  default:
    return false;
  }
}

/*
 * Compute estimated cost of inlining `region'.
 */
int computeCost(const RegionDesc& region) {
  int cost = 0;
  for (auto const& block : region.blocks()) {
    auto sk = block->start();

    for (auto i = 0, n = block->length(); i < n; ++i, sk.advance()) {
      auto op = sk.op();

      if (isFreeOp(op)) continue;

      cost += 1;

      // Add the size of immediate vectors to the cost.
      auto const pc = sk.pc();
      if (hasImmVector(op)) {
        cost += getImmVector(pc).size();
      }
    }
  }
  return cost;
}

///////////////////////////////////////////////////////////////////////////////
}

/*
 * Update context for start of inlining.
 */
void InliningDecider::accountForInlining(const Func* callee,
                                         const RegionDesc& region) {
  int cost = computeCost(region);
  m_costStack.push_back(cost);
  m_cost       += cost;
  m_callDepth  += 1;
  m_stackDepth += callee->maxStackCells();
}

bool InliningDecider::shouldInline(const Func* callee,
                                   const RegionDesc& region,
                                   uint32_t maxTotalCost) {
  auto sk = region.empty() ? SrcKey() : region.start();
  assertx(callee);
  assertx(sk.func() == callee);

  // Tracing return lambdas.
  auto refuse = [&] (const char* why) {
    FTRACE(1, "shouldInline: rejecting callee region: {}", show(region));
    return traceRefusal(m_topFunc, callee, why);
  };

  auto accept = [&, this] (const char* kind) {
    FTRACE(1, "InliningDecider: inlining {}() <- {}()\t<reason: {}>\n",
           m_topFunc->fullName()->data(), callee->fullName()->data(), kind);
    return true;
  };

  // Check inlining depths.
  if (m_callDepth + 1 >= RuntimeOption::EvalHHIRInliningMaxDepth) {
    return refuse("inlining call depth limit exceeded");
  }
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

  int numRets = 0;
  int numExits = 0;

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
        if (++numRets > RuntimeOption::EvalHHIRInliningMaxReturns) {
          return refuse("region has too many returns");
        }
        continue;
      }

      // We can't inline FCallArray.  XXX: Why?
      if (op == Op::FCallArray) {
        return refuse("can't inline FCallArray");
      }
    }

    if (region.isExit(block->id())) {
      if (++numExits > RuntimeOption::EvalHHIRInliningMaxBindJmps + numRets) {
        return refuse("region has too many non return exits");
      }
    }
  }

  // Refuse if the cost exceeds our thresholds.
  // We measure the cost of inlining each callstack and stop when it exceeds a
  // certain threshold.  (Note that we do not measure the total cost of all the
  // inlined calls for a given caller---just the cost of each nested stack.)
  const int maxCost = maxTotalCost - m_cost;
  const int cost = computeCost(region);
  if (cost > maxCost) {
    return refuse("too expensive");
  }

  if (numRets == 0) {
    return refuse("region has no returns");
  }
  return accept("small region with single return");
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
                                   std::vector<Type>& argTypes,
                                   int32_t maxBCInstrs) {
  auto const numParams = callee->numParams();

  // Set up the RegionContext for the tracelet selector.
  RegionContext ctx;
  ctx.func = callee;
  ctx.bcOffset = callee->getEntryForNumArgs(numArgs);
  ctx.spOffset = FPInvOffset{safe_cast<int32_t>(callee->numSlotsInFrame())};
  ctx.resumed = false;

  for (uint32_t i = 0; i < numArgs; ++i) {
    auto type = argTypes[i];
    assertx((type <= TGen) || (type <= TCls));
    ctx.liveTypes.push_back({RegionDesc::Location::Local{i}, type});
  }

  for (unsigned i = numArgs; i < numParams; ++i) {
    // These locals will be populated by DV init funclets but they'll start out
    // as Uninit.
    ctx.liveTypes.push_back({RegionDesc::Location::Local{i}, TUninit});
  }

  // Produce a tracelet for the callee.
  return selectTracelet(ctx, maxBCInstrs, false /* profiling */,
                        true /* inlining */);
}

TransID findTransIDForCallee(const Func* callee, const int numArgs,
                             std::vector<Type>& argTypes) {
  using LTag = RegionDesc::Location::Tag;

  auto const profData = mcg->tx().profData();
  auto const idvec = profData->funcProfTransIDs(callee->getFuncId());

  auto const offset = callee->getEntryForNumArgs(numArgs);
  for (auto const id : idvec) {
    if (profData->transStartBcOff(id) != offset) continue;
    auto const region = profData->transRegion(id);

    auto const isvalid = [&] () {
      for (auto const& typeloc : region->entry()->typePreConditions()) {
        if (typeloc.location.tag() != LTag::Local) continue;
        auto const locId = typeloc.location.localId();

        if (locId < numArgs && !(argTypes[locId] <= typeloc.type)) {
          return false;
        }
      }
      return true;
    }();

    if (isvalid) return id;
  }
  return kInvalidTransID;
}

RegionDescPtr selectCalleeCFG(const Func* callee, const int numArgs,
                              std::vector<Type>& argTypes,
                              int32_t maxBCInstrs) {
  auto const profData = mcg->tx().profData();
  if (!profData || !profData->profiling(callee->getFuncId())) return nullptr;

  auto const dvID = findTransIDForCallee(callee, numArgs, argTypes);
  if (dvID == kInvalidTransID) {
    return nullptr;
  }

  TransIDSet selectedTIDs;
  TransCFG cfg(callee->getFuncId(), profData, mcg->tx().getSrcDB(),
               mcg->getJmpToTransIDMap(), true /* inlining */);

  HotTransContext ctx;
  ctx.tid = dvID;
  ctx.cfg = &cfg;
  ctx.profData = profData;
  ctx.maxBCInstrs = maxBCInstrs;
  ctx.inlining = true;
  ctx.inputTypes = &argTypes;
  return selectHotCFG(ctx, selectedTIDs, nullptr /* selectedVec */);
}
}

RegionDescPtr selectCalleeRegion(const SrcKey& sk,
                                 const Func* callee,
                                 const IRGS& irgs,
                                 InliningDecider& inl,
                                 int32_t maxBCInstrs) {
  auto const op = sk.pc();
  auto const numArgs = getImm(op, 0).u_IVA;

  std::vector<Type> argTypes;
  for (int i = numArgs - 1; i >= 0; --i) {
    // DataTypeGeneric is used because we're just passing the locals into the
    // callee.  It's up to the callee to constrain further if needed.
    auto type = irgen::publicTopType(irgs, BCSPOffset{i});

    // If we don't have sufficient type information to inline the region
    // return early
    if (!(type <= TGen) && !(type <= TCls)) return nullptr;
    argTypes.push_back(type);
  }

  const auto mode = RuntimeOption::EvalInlineRegionMode;
  if (mode == "tracelet" || mode == "both") {
    auto region = selectCalleeTracelet(callee, numArgs, argTypes, maxBCInstrs);
    auto const maxCost = RuntimeOption::EvalHHIRInliningMaxCost;
    if (region && inl.shouldInline(callee, *region, maxCost)) return region;
    if (mode == "tracelet") return nullptr;
  }

  if (RuntimeOption::EvalJitPGO && !mcg->tx().profData()->freed()) {
    auto region = selectCalleeCFG(callee, numArgs, argTypes, maxBCInstrs);
    auto const maxCost = RuntimeOption::EvalHHIRPGOInliningMaxCost;
    if (region && inl.shouldInline(callee, *region, maxCost)) return region;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}}
