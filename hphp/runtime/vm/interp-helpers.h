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
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include <folly/Random.h>

namespace HPHP {

template<class TGetCtx>
void verifyParamType(const Func* func, int32_t id, tv_lval val,
                     TGetCtx getCtx) {
  assertx(id < func->numNonVariadicParams());
  assertx(func->numParams() == int(func->params().size()));
  const TypeConstraint& tc = func->params()[id].typeConstraint;
  if (tc.isCheckable()) {
    auto const ctx = tc.isThis() ? getCtx() : nullptr;
    tc.verifyParam(val, ctx, func, id);
  }
  if (func->hasParamsWithMultiUBs()) {
    auto const& ubs = func->paramUBs();
    auto const it = ubs.find(id);
    if (it != ubs.end()) {
      for (auto const& ub : it->second.m_constraints) {
        if (ub.isCheckable()) {
          auto const ctx = ub.isThis() ? getCtx() : nullptr;
          ub.verifyParam(val, ctx, func, id);
        }
      }
    }
  }
}

/*
 * RAII wrapper for popping/pushing generics from/to the VM stack.
 */
struct GenericsSaver {
  explicit GenericsSaver(bool hasGenerics) : m_generics(pop(hasGenerics)) {}
  ~GenericsSaver() { push(std::move(m_generics)); }

  static Array pop(bool hasGenerics) {
    if (LIKELY(!hasGenerics)) return Array();
    assertx(tvIsVec(vmStack().topC()));
    auto const generics = vmStack().topC()->m_data.parr;
    vmStack().discard();
    return Array::attach(generics);
  }
  static void push(Array&& generics) {
    if (LIKELY(generics.isNull())) return;
    vmStack().pushArrayLikeNoRc(generics.detach());
  }

private:
  Array m_generics;
};

/*
 * RAII wrapper for popping/pushing coeffects from/to the VM stack.
 */
struct CoeffectsSaver {
  explicit CoeffectsSaver(bool hasCoeffects) : m_coeffects(pop(hasCoeffects)) {}
  ~CoeffectsSaver() { push(m_coeffects); }

  static Optional<RuntimeCoeffects> pop(bool hasCoeffects) {
    if (LIKELY(!hasCoeffects)) return std::nullopt;
    assertx(tvIsInt(vmStack().topC()));
    auto const coeffects = vmStack().topC()->m_data.num;
    vmStack().discard();
    return RuntimeCoeffects::fromValue(coeffects);
  }
  static void push(Optional<RuntimeCoeffects> coeffects) {
    if (LIKELY(!coeffects)) return;
    vmStack().pushInt(coeffects->value());
  }

private:
  Optional<RuntimeCoeffects> m_coeffects;
};

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
  if (dynCallable && level < 2) return;

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
 * Check if the `callee` satisfies the coeffect constraints on the prologue
 * flags. Returns true if yes, otherwise raise a warning and return false or
 * raise an exception.
 */
inline bool calleeCoeffectChecks(const Func* callee,
                                 RuntimeCoeffects providedCoeffects,
                                 uint32_t numArgsInclUnpack,
                                 void* prologueCtx) {
  if (!CoeffectsConfig::enabled()) {
    if (callee->hasCoeffectsLocal()) {
      vmStack().pushInt(RuntimeCoeffects::none().value());
    }
    return true;
  }
  auto const requiredCoeffects = [&] {
    auto required = callee->requiredCoeffects();
    if (!callee->hasCoeffectRules()) return required;
    for (auto const& rule : callee->getCoeffectRules()) {
      required |= rule.emit(callee, numArgsInclUnpack, prologueCtx,
                            providedCoeffects);
    }
    if (callee->hasCoeffectsLocal()) vmStack().pushInt(required.value());
    return required;
  }();
  if (LIKELY(providedCoeffects.canCall(requiredCoeffects))) return true;
  raiseCoeffectsCallViolation(callee, providedCoeffects, requiredCoeffects);
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
    auto const ad = ArrayData::CreateVec();
    vmStack().pushArrayLikeNoRc(ad);
    return;
  }

  auto const generics = vmStack().topC();
  assertx(tvIsVec(generics));
  checkFunReifiedGenericMismatch(callee, val(generics).parr);
}

/*
 * Check for too few or too many arguments and trim extra args.
 */
inline void calleeArgumentArityChecks(const Func* callee,
                                      uint32_t& numArgsInclUnpack) {
  if (numArgsInclUnpack < callee->numRequiredParams()) {
    throwMissingArgument(callee, numArgsInclUnpack);
  }

  if (numArgsInclUnpack > callee->numParams()) {
    assertx(!callee->hasVariadicCaptureParam());
    assertx(numArgsInclUnpack == callee->numNonVariadicParams() + 1);
    --numArgsInclUnpack;

    GenericsSaver gs{callee->hasReifiedGenerics()};

    assertx(tvIsVec(vmStack().topC()));
    auto const numUnpackArgs = vmStack().topC()->m_data.parr->size();
    vmStack().popC();

    if (numUnpackArgs != 0) {
      raiseTooManyArguments(callee, numArgsInclUnpack + numUnpackArgs);
    }
  }
}

inline void calleeArgumentTypeChecks(const Func* callee,
                                     uint32_t numArgsInclUnpack,
                                     void* prologueCtx) {
  // Builtins use a separate non-standard mechanism.
  if (callee->isCPPBuiltin()) return;

  auto const getCtx = [&] () -> const Class* {
    if (!callee->cls()) return nullptr;
    assertx(prologueCtx);
    auto const ctx = callee->isClosureBody()
      ? reinterpret_cast<c_Closure*>(prologueCtx)->getThisOrClass()
      : prologueCtx;
    return callee->isStatic()
      ? reinterpret_cast<Class*>(ctx)
      : reinterpret_cast<ObjectData*>(ctx)->getVMClass();
  };

  auto const numArgs =
    std::min(numArgsInclUnpack, callee->numNonVariadicParams());
  auto const firstArgIdx =
    numArgsInclUnpack - 1 + (callee->hasReifiedGenerics() ? 1 : 0);
  for (auto i = 0; i < numArgs; ++i) {
    verifyParamType(callee, i, vmStack().indC(firstArgIdx - i), getCtx);
  }
}

inline void initFuncInputs(const Func* callee, uint32_t numArgsInclUnpack) {
  assertx(numArgsInclUnpack <= callee->numParams());

  // All arguments already initialized. Extra arguments already popped
  // by calleeArgumentArityChecks().
  if (LIKELY(numArgsInclUnpack == callee->numParams())) return;

  CoeffectsSaver cs{callee->hasCoeffectsLocal()};
  GenericsSaver gs{callee->hasReifiedGenerics()};
  auto const numParams = callee->numNonVariadicParams();
  while (numArgsInclUnpack < numParams) {
    vmStack().pushUninit();
    ++numArgsInclUnpack;
  }

  if (callee->hasVariadicCaptureParam()) {
    auto const ad = ArrayData::CreateVec();
    vmStack().pushArrayLikeNoRc(ad);
    ++numArgsInclUnpack;
  }

  assertx(numArgsInclUnpack == callee->numParams());
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
   * All stack checks are inflated by stackCheckPadding() to ensure
   * there is space both for calling leaf functions /and/ for
   * re-entry.  (See kStackCheckReenterPadding and
   * RuntimeOption::EvalStackCheckLeafPadding.)
   */
  auto limit = f->maxStackCells() + stackCheckPadding() + extraCells;
  if (LIKELY(stack_in_bounds() && !stk.wouldOverflow(limit))) return;
  TRACE_MOD(Trace::gc, 1, "Maximum stack depth exceeded.\n");
  throw_stack_overflow();
}

}
