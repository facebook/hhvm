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

#include "hphp/runtime/vm/jit/inlining.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/region-selection.h"

#include "hphp/util/trace.h"

#include <string>

namespace HPHP { namespace JIT {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(region);

namespace {
///////////////////////////////////////////////////////////////////////////////

bool traceRefusal(const char* name, const Func* callee,
                  const char* why, const SrcKey& sk) {
#ifdef USE_TRACE
  const char* calleeName = callee ? callee->fullName()->data()
                                  : "(unknown)";
  FTRACE(1, "{}: refusing {} <reason: {}> [NI = {}]\n",
         name, calleeName, why, sk.showInst());

#endif
  return false;
}

/*
 * Check if the funcd of `inst' has any characteristics which prevent inlining,
 * without peeking into its bytecode or regions.
 */
bool isCalleeInlinable(const NormalizedInstruction& inst) {
  auto callee = inst.funcd;

  auto refuse = [&] (const char* why) {
    return traceRefusal("canInlineAt", callee, why, inst.source);
  };

  if (!callee) {
    return refuse("callee not known");
  }
  if (callee == inst.func()) {
    return refuse("call is recursive");
  }
  if (callee->hasVariadicCaptureParam()) {
    return refuse("callee has variadic capture");
  }
  if (callee->numIterators() != 0) {
    return refuse("callee has iterators");
  }
  if (callee->isMagic() || Func::isSpecial(callee->name())) {
    return refuse("special or magic callee");
  }
  if (callee->isResumable()) {
    return refuse("callee is resumable");
  }
  if (callee->maxStackCells() >= kStackCheckLeafPadding) {
    return refuse("function stack depth too deep");
  }
  if (callee->isMethod() && callee->cls() == c_Generator::classof()) {
    return refuse("generator member function");
  }
  return true;
}

/*
 * Check that we don't have any missing or extra arguments.
 */
bool checkNumArgs(const NormalizedInstruction& inst) {
  assert(inst.funcd);
  auto callee = inst.funcd;

  auto refuse = [&] (const char* why) {
    return traceRefusal("canInlineAt", callee, why, inst.source);
  };

  auto const numArgs = inst.imm[0].u_IVA;
  auto const numParams = callee->numParams();

  if (numArgs > numParams) {
    return refuse("callee called with too many arguments");
  }

  // It's okay if we passed fewer arguments than there are parameters as long
  // as the gap can be filled in by DV funclets.
  for (auto i = numArgs; i < numParams; ++i) {
    auto const& param = callee->params()[i];
    if (!param.hasDefaultValue()) {
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
bool checkFPIRegion(const NormalizedInstruction& inst,
                    const RegionDesc& region) {
  assert(inst.funcd);
  auto callee = inst.funcd;

  auto refuse = [&] (const char* why) {
    return traceRefusal("canInlineAt", callee, why, inst.source);
  };

  // Check that the FPush instruction is in the same region.
  auto fpi = inst.func()->findFPI(inst.offset());
  const SrcKey pushSK { inst.func(),
                        fpi->m_fpushOff,
                        inst.source.resumed() };
  int pushBlock = -1;

  for (unsigned i = 0; i < region.blocks.size(); ++i) {
    if (region.blocks[i]->contains(pushSK)) {
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
  for (unsigned i = pushBlock; i < region.blocks.size(); ++i) {
    auto const& block = *region.blocks[i];

    auto sk = (i == pushBlock ? pushSK.advanced()
                              : block.start());
    while (sk <= block.last()) {
      // We're all set once we've hit the to-be-inlined FCall.
      if (sk == inst.source) return true;

      auto op = sk.op();

      if (isFCallStar(op) || op == Op::FCallBuiltin) {
        return refuse("FPI region contains another call");
      }
      sk.advance();
    }
  }

  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}

bool canInlineAt(const NormalizedInstruction& inst,
                 const RegionDesc& region) {
  if (!RuntimeOption::RepoAuthoritative) return false;

  // We can only inline at normal FCalls.
  if (inst.op() != Op::FCall &&
      inst.op() != Op::FCallD) {
    return false;
  }

  // Don't inline from resumed functions.
  if (inst.source.resumed()) return false;

  // TODO(#4238160): Inlining into pseudomain callsites is still buggy.
  if (inst.func()->isPseudoMain()) return false;

  if (!isCalleeInlinable(inst) ||
      !checkNumArgs(inst) ||
      !checkFPIRegion(inst, region)) {
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
