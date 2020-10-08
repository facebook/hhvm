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

#pragma once

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/rx.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include <folly/Random.h>

namespace HPHP {

/*
 * RAII wrapper for popping/pushing generics from/to the VM stack.
 */
struct GenericsSaver {
  GenericsSaver(bool hasGenerics) : m_generics(pop(hasGenerics)) {}
  ~GenericsSaver() { push(std::move(m_generics)); }

  static Array pop(bool hasGenerics) {
    if (LIKELY(!hasGenerics)) return Array();
    assertx(tvIsHAMSafeVArray(vmStack().topC()));
    auto const generics = vmStack().topC()->m_data.parr;
    vmStack().discard();
    return Array::attach(generics);
  }
  static void push(Array&& generics) {
    if (LIKELY(generics.isNull())) return;
    if (RuntimeOption::EvalHackArrDVArrs) {
      vmStack().pushVecNoRc(generics.detach());
    } else {
      vmStack().pushArrayNoRc(generics.detach());
    }
  }

private:
  Array m_generics;
};

inline void callerInOutChecks(const Func* func, const FCallArgs& fca) {
  for (auto i = 0; i < fca.numArgs; ++i) {
    auto const inout = func->isInOut(i);
    if (inout != fca.isInOut(i)) {
      SystemLib::throwInvalidArgumentExceptionObject(
        formatParamInOutMismatch(func->fullName()->data(), i, inout));
    }
  }
}

inline void callerDynamicCallChecks(const Func* func,
                                    bool allowDynCallNoPointer = false) {
  auto dynCallable = func->isDynamicallyCallable();
  if (dynCallable) {
    if (allowDynCallNoPointer) return;
    if (!RO::EvalForbidDynamicCallsWithAttr) return;
  }
  auto level = func->isMethod()
    ? (func->isStatic()
        ? RO::EvalForbidDynamicCallsToClsMeth
        : RO::EvalForbidDynamicCallsToInstMeth)
    : RO::EvalForbidDynamicCallsToFunc;
  if (level <= 0) return;

  if (auto const rate = func->dynCallSampleRate()) {
    if (folly::Random::rand32(*rate) != 0) return;
    level = 1;
  }

  auto error_msg = dynCallable ?
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE :
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE;
  if (level >= 3 || (level >= 2 && !dynCallable)) {
    std::string msg;
    string_printf(msg, error_msg, func->fullName()->data());
    throw_invalid_operation_exception(makeStaticString(msg));
  }
  raise_notice(error_msg, func->fullName()->data());
}

inline void callerDynamicConstructChecks(const Class* cls) {
  auto level = RO::EvalForbidDynamicConstructs;
  if (level <= 0 || cls->isDynamicallyConstructible()) return;

  if (auto const rate = cls->dynConstructSampleRate()) {
    if (folly::Random::rand32(*rate) != 0) return;
    level = 1;
  }

  if (level >= 2) {
    std::string msg;
    string_printf(
      msg,
      Strings::CLASS_CONSTRUCTED_DYNAMICALLY,
      cls->name()->data()
    );
    throw_invalid_operation_exception(makeStaticString(msg));
  } else {
    raise_notice(
      Strings::CLASS_CONSTRUCTED_DYNAMICALLY,
      cls->name()->data()
    );
  }
}

inline void calleeDynamicCallChecks(const Func* func, bool dynamicCall,
                                    bool allowDynCallNoPointer = false) {
  if (!dynamicCall) return;
  if (func->isDynamicallyCallable() && allowDynCallNoPointer) return;

  auto error_msg = func->isDynamicallyCallable() ?
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE :
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE;

  if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin()) {
    raise_notice(
      error_msg,
      func->fullName()->data()
    );
  }
}

/*
 * Check if a call from `caller` to `callee` satisfies reactivity constraints.
 * Returns true if yes, otherwise raise a warning and return false or raise
 * an exception.
 */
inline bool callerRxChecks(const ActRec* caller, const Func* callee) {
  if (RuntimeOption::EvalPureEnforceCalls <= 0) return true;
  // Conditional reactivity is not tracked yet, so assume the caller has minimum
  // and the callee has maximum possible level of reactivity.
  auto const callerLevel = caller->rxMinLevel();
  if (!rxEnforceCallsInLevel(callerLevel)) return true;

  auto const minReqCalleeLevel = rxRequiredCalleeLevel(callerLevel);
  if (LIKELY(callee->rxLevel() >= minReqCalleeLevel)) return true;
  raiseRxCallViolation(caller, callee);
  return false;
}

/*
 * Check for presence, count and wildcard match of generics.
 */
inline void calleeGenericsChecks(const Func* callee, bool hasGenerics) {
  if (LIKELY(!callee->hasReifiedGenerics())) {
    if (UNLIKELY(hasGenerics)) vmStack().popC();
    return;
  }

  if (!hasGenerics) {
    if (!areAllGenericsSoft(callee->getReifiedGenericsInfo())) {
      throw_call_reified_func_without_generics(callee);
    }

    raise_warning_for_soft_reified(0, true, callee->fullName());

    // Push an empty array, as the remainder of the call setup assumes generics
    // are on the stack.
    ARRPROV_USE_RUNTIME_LOCATION();
    auto const ad = ArrayData::CreateVArray();
    if (RuntimeOption::EvalHackArrDVArrs) {
      vmStack().pushVecNoRc(ad);
    } else {
      vmStack().pushArrayNoRc(ad);
    }
    return;
  }

  auto const generics = vmStack().topC();
  assertx(tvIsHAMSafeVArray(generics));
  checkFunReifiedGenericMismatch(callee, val(generics).parr);
}

/*
 * Check for too few or too many arguments and trim extra args.
 */
inline void calleeArgumentArityChecks(const Func* callee,
                                      uint32_t numArgsInclUnpack) {
  if (numArgsInclUnpack < callee->numRequiredParams()) {
    throwMissingArgument(callee, numArgsInclUnpack);
  }

  if (numArgsInclUnpack > callee->numParams()) {
    assertx(!callee->hasVariadicCaptureParam());
    assertx(numArgsInclUnpack == callee->numNonVariadicParams() + 1);

    GenericsSaver gs{callee->hasReifiedGenerics()};

    assertx(tvIsHAMSafeVArray(vmStack().topC()));
    auto const numUnpackArgs = vmStack().topC()->m_data.parr->size();
    vmStack().popC();

    if (numUnpackArgs != 0) {
      raiseTooManyArguments(callee, numArgsInclUnpack + numUnpackArgs - 1);
    }
  }
}

inline void calleeImplicitContextChecks(const Func* callee) {
  if (!RO::EvalEnableImplicitContext ||
      !callee->hasNoContextAttr() ||
      *ImplicitContext::activeCtx == nullptr) {
    return;
  }
  throw_implicit_context_exception(folly::to<std::string>(
    "Function ", callee->fullName()->data(), " has implicit context "
    "but is marked with __NoContext"));
}

inline void initFuncInputs(const Func* callee, uint32_t numArgsInclUnpack) {
  assertx(numArgsInclUnpack <= callee->numNonVariadicParams() + 1);

  // All arguments already initialized. Extra arguments already popped
  // by calleeArgumentArityChecks().
  if (LIKELY(numArgsInclUnpack >= callee->numParams())) return;

  GenericsSaver gs{callee->hasReifiedGenerics()};
  auto const numParams = callee->numNonVariadicParams();
  while (numArgsInclUnpack < numParams) {
    vmStack().pushUninit();
    ++numArgsInclUnpack;
  }

  if (callee->hasVariadicCaptureParam()) {
    ARRPROV_USE_RUNTIME_LOCATION();
    auto const ad = ArrayData::CreateVArray();
    if (RuntimeOption::EvalHackArrDVArrs) {
      vmStack().pushVecNoRc(ad);
    } else {
      vmStack().pushArrayNoRc(ad);
    }
    ++numArgsInclUnpack;
  }

  assertx(numArgsInclUnpack == callee->numParams());
}

/*
 * This helper only does a stack overflow check for the native stack.
 * Both native and VM stack overflows are independently possible.
 */
inline void checkNativeStack() {
  // Check whether we're going out of bounds of our native stack.
  if (LIKELY(stack_in_bounds())) return;
  TRACE_MOD(Trace::gc, 1, "Maximum stack depth exceeded.\n");
  throw_stack_overflow();
}

/*
 * This helper does a stack overflow check on *both* the native stack
 * and the VM stack.
 *
 * In some cases for re-entry, we're checking for space other than
 * just the callee, and `extraCells' may need to be passed with a
 * non-zero value.  (We over-check in these situations, but it's fine.)
 */
ALWAYS_INLINE
void checkStack(Stack& stk, const Func* f, int32_t extraCells) {
  /*
   * Check whether func's maximum stack usage would overflow the stack.
   * Both native and VM stack overflows are independently possible.
   *
   * All stack checks are inflated by kStackCheckPadding to ensure
   * there is space both for calling leaf functions /and/ for
   * re-entry.  (See kStackCheckReenterPadding and
   * kStackCheckLeafPadding.)
   */
  auto limit = f->maxStackCells() + kStackCheckPadding + extraCells;
  if (LIKELY(stack_in_bounds() && !stk.wouldOverflow(limit))) return;
  TRACE_MOD(Trace::gc, 1, "Maximum stack depth exceeded.\n");
  throw_stack_overflow();
}

}
