/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/irgen.h"

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
    auto pc = reinterpret_cast<const Op*>(callSK.pc());
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

  auto pc = reinterpret_cast<const Op*>(callSK.pc());
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

/*
 * Check that the FPI region is suitable for inlining.
 *
 * We refuse to inline if the corresponding FPush is not found in the same
 * region as the FCall, or if other calls are made between the two.
 */
bool checkFPIRegion(SrcKey callSK, const Func* callee,
                    const RegionDesc& region) {
  assertx(callee);

  auto refuse = [&] (const char* why) {
    return traceRefusal(callSK.func(), callee, why);
  };

  // Check that the FPush instruction is in the same region, and that our FCall
  // is reachable from it.
  //
  // TODO(#4603302) Fix this once SrcKeys can appear multiple times in a region.
  auto fpi = callSK.func()->findFPI(callSK.offset());
  const SrcKey pushSK { callSK.func(),
                        fpi->m_fpushOff,
                        callSK.resumed() };
  int pushBlock = -1;

  auto const& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); ++i) {
    if (blocks[i]->contains(pushSK)) {
      pushBlock = i;
      break;
    }
  }
  if (pushBlock == -1) {
    return refuse("FPush* is not in the current region");
  }

  // Check that we have an acceptable FPush.
  switch (pushSK.op()) {
    case OpFPushClsMethodD:
      if (callee->mayHaveThis()) {
        return refuse("callee may have $this pointer");
      }
      // fallthrough
    case OpFPushFuncD:
    case OpFPushObjMethodD:
    case OpFPushCtorD:
    case OpFPushCtor:
      break;

    default:
      return refuse(folly::format("unsupported push op {}",
                                  opcodeToName(pushSK.op())).str().c_str());
  }

  // Calls invalidate all live SSATmps, so don't allow any in the FPI region.
  for (unsigned i = pushBlock; i < blocks.size(); ++i) {
    auto const& block = *blocks[i];

    auto iterSK = (i == pushBlock ? pushSK.advanced()
                                  : block.start());
    while (iterSK <= block.last()) {
      // We're all set once we've hit the to-be-inlined FCall.
      if (iterSK == callSK) return true;

      auto op = iterSK.op();

      if (isFCallStar(op) || op == Op::FCallBuiltin) {
        return refuse("FPI region contains another call");
      }
      iterSK.advance();
    }
  }

  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}

void InliningDecider::forbidInliningOf(const Func* callee) {
  hasCalledDisableInliningIntrinsic.store(true);
  SimpleLock locker(forbiddenInlineesLock);
  forbiddenInlinees.insert(callee->fullName());
}

bool InliningDecider::canInlineAt(SrcKey callSK, const Func* callee,
                                  const RegionDesc& region) const {
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

  if (!isCalleeInlinable(callSK, callee) ||
      !checkNumArgs(callSK, callee) ||
      !checkFPIRegion(callSK, callee, region)) {
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

///////////////////////////////////////////////////////////////////////////////
}

bool InliningDecider::shouldInline(const Func* callee,
                                   const RegionDesc& region) {
  auto sk = region.empty() ? SrcKey() : region.start();
  assertx(callee);
  assertx(sk.func() == callee);

  int cost = 0;

  // Tracing return lambdas.
  auto refuse = [&] (const char* why) {
    return traceRefusal(m_topFunc, callee, why);
  };

  auto accept = [&, this] (const char* kind) {
    FTRACE(1, "InliningDecider: inlining {}() <- {}()\t<reason: {}>\n",
           m_topFunc->fullName()->data(), callee->fullName()->data(), kind);

    // Update our context.
    m_costStack.push_back(cost);
    m_cost += cost;
    m_callDepth += 1;
    m_stackDepth += callee->maxStackCells();

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

  // We measure the cost of inlining each callstack and stop when it exceeds a
  // certain threshold.  (Note that we do not measure the total cost of all the
  // inlined calls for a given caller---just the cost of each nested stack.)
  const int maxCost = RuntimeOption::EvalHHIRInliningMaxCost - m_cost;

  // We only inline callee regions that have exactly one return.
  //
  // NOTE: Currently, the tracelet selector uses the first Ret in the child's
  // region to determine when to stop inlining.  However, the safety of this
  // behavior should not be considered guaranteed by InliningDecider; the
  // "right" way to decide when inlining ends is to inline all of `region'.
  int numRets = 0;

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
      if (isRet(op) || op == Op::NativeImpl) {
        if (++numRets > 1) {
          return refuse("region has too many returns");
        }
        continue;
      }

      // We can't inline FCallArray.  XXX: Why?
      if (op == Op::FCallArray) {
        return refuse("can't inline FCallArray");
      }

      // Assert opcodes don't contribute to the inlining cost.
      if (op == Op::AssertRATL || op == Op::AssertRATStk) continue;

      cost += 1;

      // Add the size of immediate vectors to the cost.
      auto const pc = reinterpret_cast<const Op*>(sk.pc());
      if (hasMVector(op)) {
        cost += getMVector(pc).size();
      } else if (hasImmVector(op)) {
        cost += getImmVector(pc).size();
      }

      // Refuse if the cost exceeds our thresholds.
      if (cost > maxCost) {
        return refuse("too expensive");
      }
    }
  }

  if (numRets != 1) {
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

///////////////////////////////////////////////////////////////////////////////
}}
