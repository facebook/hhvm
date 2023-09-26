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
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/util/conv-10.h"
#include "hphp/util/trace.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include <folly/tracing/StaticTracepoint.h>
#include <folly/Random.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

/**
 * print_string will decRef the string
 */
void print_string(StringData* s) {
  g_context->write(s->data(), s->size());
  TRACE(2, "t-x64 output(str): (%p) %43s\n", s->data(),
        escapeStringForCPP(s->data(), s->size()).data());
  decRefStr(s);
}

void print_int(int64_t i) {
  char intbuf[21];
  auto const s = conv_10(i, intbuf + sizeof(intbuf));

  g_context->write(s.data(), s.size());
}

void print_boolean(bool val) {
  if (val) {
    g_context->write("1", 1);
  }
}

/**
 * concat_ss will will incRef the output string
 * and decref its first argument
 */
StringData* concat_ss(StringData* v1, StringData* v2) {
  if (v1->cowCheck()) {
    FOLLY_SDT(hhvm, hhvm_cow_concat, v1->size(), v2->size());
    StringData* ret = StringData::Make(v1, v2);
    // Because v1 was shared, we know this won't release the string.
    v1->decRefCount();
    return ret;
  }

  auto const rhs = v2->slice();
  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, rhs.size());
  auto const ret = v1->append(rhs);
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

/**
 * concat_is will incRef the output string
 */
StringData* concat_is(int64_t v1, StringData* v2) {
  char intbuf[21];
  // Convert the int to a string
  auto const s1 = conv_10(v1, intbuf + sizeof(intbuf));
  auto const s2 = v2->slice();
  return StringData::Make(s1, s2);
}

/**
 * concat_si will incRef the output string
 * and decref its first argument
 */
StringData* concat_si(StringData* v1, int64_t v2) {
  char intbuf[21];
  auto const s2 = conv_10(v2, intbuf + sizeof(intbuf));
  if (v1->cowCheck()) {
    auto const s1 = v1->slice();
    FOLLY_SDT(hhvm, hhvm_cow_concat, s1.size(), s2.size());
    auto const ret = StringData::Make(s1, s2);
    // Because v1 was shared, we know this won't release it.
    v1->decRefCount();
    return ret;
  }

  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, s2.size());
  auto const ret = v1->append(s2);
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

StringData* concat_s3(StringData* v1, StringData* v2, StringData* v3) {
  if (v1->cowCheck()) {
    auto s1 = v1->slice();
    auto s2 = v2->slice();
    auto s3 = v3->slice();
    FOLLY_SDT(hhvm, hhvm_cow_concat, s1.size(), s2.size() + s3.size());
    StringData* ret = StringData::Make(s1, s2, s3);
    // Because v1 was shared, we know this won't release it.
    v1->decRefCount();
    return ret;
  }

  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, v2->size() + v3->size());
  auto const ret = v1->append(v2->slice(), v3->slice());
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

StringData* concat_s4(StringData* v1, StringData* v2,
                      StringData* v3, StringData* v4) {
  if (v1->cowCheck()) {
    auto s1 = v1->slice();
    auto s2 = v2->slice();
    auto s3 = v3->slice();
    auto s4 = v4->slice();
    FOLLY_SDT(hhvm, hhvm_cow_concat, s1.size(),
              s2.size() + s3.size() + s4.size());
    StringData* ret = StringData::Make(s1, s2, s3, s4);
    // Because v1 was shared, we know this won't release it.
    v1->decRefCount();
    return ret;
  }

  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize,
            v2->size() + v3->size() + v4->size());
  auto const ret = v1->append(v2->slice(), v3->slice(), v4->slice());
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

void raiseWarning(const StringData* sd) {
  raise_warning("%s", sd->data());
}

void raiseNotice(const StringData* sd) {
  raise_notice("%s", sd->data());
}

void throwArrayIndexException(const ArrayData* ad, const int64_t index) {
  throwOOBArrayKeyException(index, ad);
}

void throwArrayKeyException(const ArrayData* ad, const StringData* key) {
  assertx(ad->isDictType());
  throwOOBArrayKeyException(key, ad);
}

void throwMustBeEnclosedInReadonlyException(const Class* cls, const StringData* propName) {
  throw_must_be_enclosed_in_readonly(cls->name()->data(), propName->data());
}

void throwMustBeReadonlyException(const Class* cls, const StringData* propName) {
  throw_must_be_readonly(cls->name()->data(), propName->data());
}

void throwMustBeMutableException(const Class* cls, const StringData* propName) {
  throw_must_be_mutable(cls->name()->data(), propName->data());
}

void throwOrWarnLocalMustBeValueTypeException(const StringData* locName) {
  throw_local_must_be_value_type(locName->data());
}

void throwMustBeValueTypeException(const Class* cls, const StringData* propName) {
  throw_must_be_value_type(cls->name()->data(), propName->data());
}

std::string formatParamInOutMismatch(const char* fname, uint32_t index,
                                   bool funcByRef) {
  if (funcByRef) {
    return folly::sformat(
      "{}() expects parameter {} to be inout, but the call was "
      "not annotated with 'inout'", fname, index + 1
    );
  } else {
    return folly::sformat(
      "{}() does not expect parameter {} to be inout, but the call was "
      "annotated with 'inout'", fname, index + 1
    );
  }
}

void throwParamInOutMismatch(const Func* func, uint32_t index) {
  SystemLib::throwInvalidArgumentExceptionObject(formatParamInOutMismatch(
    func->fullName()->data(), index, func->isInOut(index)));
}

void checkInOutMismatch(const Func* func, uint32_t numArgs,
                        const uint8_t* inoutArgs) {
  assertx(numArgs == 0 || inoutArgs != nullptr);
  uint8_t tmp = 0;
  for (auto i = 0; i < numArgs; ++i) {
    tmp = (i % 8) == 0 ? *(inoutArgs++) : tmp >> 1;
    if (func->isInOut(i) != (tmp & 1)) throwParamInOutMismatch(func, i);
  }
}

std::string formatParamReadonlyMismatch(const char* fname, uint32_t index) {
  return folly::sformat(
    "{}() does not allow for parameter {} to be passed readonly, but the argument "
    "was annotated with 'readonly'", fname, index + 1
  );
}

std::string formatReturnReadonlyMismatch(const char* fname) {
  return folly::sformat(
    "{}() returns readonly, but the caller expects a mutable value",
    fname
  );
}

std::string formatReadonlyThisMismatch(const char* fname) {
  return folly::sformat(
    "{}() modifies the instance, but the caller has a readonly instance",
    fname
  );
}

void throwReadonlyMismatch(const Func* func, int32_t index) {
  auto const msg = [&] {
    if (index == kReadonlyReturnId) {
      return formatReturnReadonlyMismatch(func->fullName()->data());
    }
    if (index == kReadonlyThisId) {
      return formatReadonlyThisMismatch(func->fullName()->data());
    }
    return formatParamReadonlyMismatch(func->fullName()->data(), index);
  }();
  SystemLib::throwReadonlyViolationExceptionObject(msg);
}

void checkReadonlyMismatch(const Func* func, uint32_t numArgs,
                           const uint8_t* readonlyArgs) {
  assertx(numArgs == 0 || readonlyArgs != nullptr);
  uint8_t tmp = 0;
  for (auto i = 0; i < numArgs; ++i) {
    tmp = (i % 8) == 0 ? *(readonlyArgs++) : tmp >> 1;
    if ((tmp & 1) && !func->isReadonly(i)) throwReadonlyMismatch(func, i);
  }
}

void throwInvalidUnpackArgs() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only containers may be unpacked");
}

namespace {

std::string formatArgumentErrMsg(const Func* func, const char* amount,
                                 uint32_t expected, uint32_t got) {
  return folly::sformat(
    "{}() expects {} {} parameter{}, {} given",
    func->fullNameWithClosureName(),
    amount,
    expected,
    expected == 1 ? "" : "s",
    got
  );
}

}

void throwMissingArgument(const Func* func, int got) {
  auto const expected = func->numRequiredParams();
  assertx(got < expected);
  auto const amount = expected < func->numParams() ? "at least" : "exactly";
  auto const errMsg = formatArgumentErrMsg(func, amount, expected, got);
  SystemLib::throwRuntimeExceptionObject(Variant(errMsg));
}

void raiseTooManyArguments(const Func* func, int got) {
  assertx(!func->hasVariadicCaptureParam());

  if (!RuntimeOption::EvalWarnOnTooManyArguments && !func->isCPPBuiltin()) {
    return;
  }

  auto const total = func->numNonVariadicParams();
  assertx(got > total);
  auto const amount = func->numRequiredParams() < total ? "at most" : "exactly";
  auto const errMsg = formatArgumentErrMsg(func, amount, total, got);

  if (RuntimeOption::EvalWarnOnTooManyArguments > 1 || func->isCPPBuiltin()) {
    SystemLib::throwRuntimeExceptionObject(Variant(errMsg));
  } else {
    raise_warning(errMsg);
  }
}

void raiseTooManyArgumentsPrologue(const Func* func, ArrayData* unpackArgs) {
  SCOPE_EXIT { decRefArr(unpackArgs); };
  if (unpackArgs->empty()) return;
  auto const got = func->numNonVariadicParams() + unpackArgs->size();
  raiseTooManyArguments(func, got);
}

//////////////////////////////////////////////////////////////////////

RDS_LOCAL_NO_CHECK(uint64_t, rl_num_coeffect_violations){0};

void raiseCoeffectsCallViolation(const Func* callee,
                                 RuntimeCoeffects provided,
                                 RuntimeCoeffects required) {
  assertx(CoeffectsConfig::enabled());
  auto const callerName = [&] {
    VMRegAnchor _;
    if (!vmfp()) {
      // VM is entering to the first frame
      return String{makeStaticString("[vm-entry]")};
    }
    String result;
    walkStack([&] (const BTFrame& frm) {
      assertx(frm);
      auto const func = frm.func();
      assertx(func);
      if (func->hasCoeffectRules() &&
          func->getCoeffectRules().size() == 1 &&
          func->getCoeffectRules()[0].isCaller()) {
        return false; // keep going
      }
      result = func->fullNameWithClosureName();
      return true;
    });
    assertx(!result.isNull());
    return result;
  };

  auto const errMsg = [&] {
    return folly::sformat(
      "Call to {}() requires [{}] coeffects but {}() provided [{}]",
      callee->fullNameWithClosureName(),
      required.toString(),
      callerName(),
      provided.toString()
    );
  };

  FTRACE_MOD(Trace::coeffects, 1, "{}\n {:016b} -> {:016b}\n",
             errMsg(), provided.value(), required.value());

  assertx(!provided.canCall(required));
  if (provided.canCallWithWarning(required)) {
    if (*rl_num_coeffect_violations >= RO::EvalCoeffectViolationWarningMax) {
      return;
    }
    (*rl_num_coeffect_violations)++;
    raise_warning(errMsg());
  } else {
    SystemLib::throwCoeffectViolationExceptionObject(errMsg());
  }
}

void raiseCoeffectsFunParamTypeViolation(TypedValue tv,
                                         int32_t paramIdx) {
  auto const errMsg =
    folly::sformat("Coeffect rule requires parameter at position {} to be a "
                   "closure object, function/method pointer or null but "
                   "{} given",
                   paramIdx + 1, describe_actual_type(&tv));
  if (CoeffectsConfig::throws()) {
    SystemLib::throwInvalidArgumentExceptionObject(errMsg);
  }
  raise_warning(errMsg);
}

void raiseCoeffectsFunParamCoeffectRulesViolation(const Func* f) {
  assertx(f);
  auto const errMsg =
    folly::sformat("Function/method pointer to {}() contains polymorphic "
                   "coeffects but is used as a coeffect rule",
                   f->fullNameWithClosureName());
  if (CoeffectsConfig::throws()) {
    SystemLib::throwInvalidArgumentExceptionObject(errMsg);
  }
  raise_warning(errMsg);
}

namespace {

void moduleBoundaryViolationImpl(
  std::string symbol,
  const StringData* symbolModule,
  const StringData* fromModule,
  bool soft
) {
  // Internal symbols must always have a module
  assertx(symbolModule != nullptr);
  assertx(fromModule != nullptr);
  assertx(symbolModule != fromModule);
  assertx(!symbol.empty());
  // Don't throw error in debugger when bypassCheck is on
  if (!g_context.isNull() && g_context->debuggerSettings.bypassCheck) return;
  auto const errMsg = folly::sformat(
    "Accessing {}internal {} in module {} from {} is not allowed",
    soft ? "soft " : "",
    symbol,
    symbolModule,
    Module::isDefault(fromModule)
      ? "the default module"
      : folly::sformat("module {}", fromModule)
  );
  if (!soft) SystemLib::throwModuleBoundaryViolationExceptionObject(errMsg);
  raise_warning(errMsg);
}

} // namespace

void raiseModulePropertyViolation(
  const Class* cls,
  const StringData* prop,
  const StringData* callerModule,
  bool is_static = false
) {
  assertx(cls);
  assertx(prop);
  auto const attrs = [&] {
    if (is_static) {
      auto const propSlot = cls->lookupSProp(prop);
      return cls->staticProperties()[propSlot].attrs;
    }
    auto const propSlot = cls->lookupDeclProp(prop);
    return cls->declProperties()[propSlot].attrs;
  }();
  assertx(attrs & AttrInternal);
  auto const soft = attrs & AttrInternalSoft;

  auto const msg = is_static ? "static property {}::${}" : "property {}::${}";
  return moduleBoundaryViolationImpl(
    folly::sformat(msg, cls->name(), prop->data()),
    cls->moduleName(),
    callerModule,
    soft
  );
}

void raiseModuleBoundaryViolation(const Class* ctx,
                                  const Func* callee,
                                  const StringData* callerModule) {
  assertx(callee);
  assertx(IMPLIES(callee->isMethod(), ctx));
  assertx(callee->isInternal());
  return moduleBoundaryViolationImpl(
    ctx && callee->isMethod()
        ? folly::sformat("method {}::{}", ctx->name(), callee->name())
        : folly::sformat("function {}", callee->name()),
    callee->moduleName(),
    callerModule,
    callee->attrs() & AttrInternalSoft
  );
}

void raiseModuleBoundaryViolation(const Class* cls,
                                  const StringData* callerModule) {
  assertx(cls);
  assertx(cls->isInternal());
  auto const symbolType =
    isEnum(cls) ? "enum" : (isEnumClass(cls) ? "enum class" : "class");
  return moduleBoundaryViolationImpl(
    folly::sformat("{} {}", symbolType, cls->name()),
    cls->moduleName(),
    callerModule,
    cls->attrs() & AttrInternalSoft
  );
}

void raiseDeploymentBoundaryViolation(const Func* callee) {
  if (!RO::EvalEnforceDeployment) return;
  auto const& packageInfo = g_context->getPackageInfo();
  assertx(callee && !callee->isMethod());
  auto const soft = packageInfo.moduleInASoftPackage(callee->moduleName());
  auto const calleeName = folly::sformat("function {}", callee->name());
  auto const errMsg = folly::sformat(
    "Calling {} outside the active deployment is not allowed",
    calleeName);
  if (!soft) SystemLib::throwDeploymentBoundaryViolationExceptionObject(errMsg);
  if (auto const rate = RO::EvalDeploymentViolationWarningSampleRate) {
    if (folly::Random::rand32(rate) != 0) return;
    raise_warning(errMsg);
  }
}

void raiseDeploymentBoundaryViolation(const Class* cls) {
  if (!RO::EvalEnforceDeployment) return;
  auto const& packageInfo = g_context->getPackageInfo();
  auto const soft = packageInfo.moduleInASoftPackage(cls->moduleName());
  auto const symbolType =
    isEnum(cls) ? "enum" : (isEnumClass(cls) ? "enum class" : "class");
  auto const clsName = folly::sformat("{}", cls->name());
  auto const errMsg = folly::sformat(
    "Accessing {} {} outside the active deployment is not allowed",
    symbolType,
    clsName);
  if (!soft) SystemLib::throwDeploymentBoundaryViolationExceptionObject(errMsg);
  if (auto const rate = RO::EvalDeploymentViolationWarningSampleRate) {
    if (folly::Random::rand32(rate) != 0) return;
    raise_warning(errMsg);
  }
}

void raiseImplicitContextStateInvalid(const Func* func,
                                      ImplicitContext::State state) {
  auto const msg = folly::sformat(
    "{} is a [defaults] memoized function, "
    "but it is called with {} state implicit context",
    func->fullName(),
    ImplicitContext::stateToString(state)
  );
  if (!ImplicitContext::isStateSoft(state)) {
    SystemLib::throwInvalidOperationExceptionObject(msg);
  }
  raise_warning(msg);
}

void raiseImplicitContextSoftInaccessibleStateInvalid(const Func* func,
                                                      ImplicitContext::State state) {
  auto const msg = folly::sformat(
    "{} is a soft IC inaccessible memoized function, "
    "but it is called with {} state implicit context",
    func->fullName(),
    ImplicitContext::stateToString(state)
  );
  if (!ImplicitContext::isStateSoft(state)) {
    SystemLib::throwInvalidOperationExceptionObject(msg);
  }
  raise_warning(msg);
}

void raiseImplicitContextStateInvalidDispatch(const Func* func) {
  auto const obj = *ImplicitContext::activeCtx;
  assertx(obj);
  auto const context = Native::data<ImplicitContext>(obj);

  if (func->isNoICMemoize()) {
    raiseImplicitContextStateInvalid(func, context->m_state);
    return;
  }

  assertx(func->isSoftMakeICInaccessibleMemoize());
  raiseImplicitContextSoftInaccessibleStateInvalid(func, context->m_state);
}

//////////////////////////////////////////////////////////////////////

int64_t zero_error_level() {
  auto& id = RequestInfo::s_requestInfo.getNoCheck()->m_reqInjectionData;
  auto level = id.getErrorReportingLevel();
  id.setErrorReportingLevel(0);
  return level;
}

void restore_error_level(int64_t oldLevel) {
  auto& id = RequestInfo::s_requestInfo.getNoCheck()->m_reqInjectionData;
  if (id.getErrorReportingLevel() == 0) {
    id.setErrorReportingLevel(oldLevel);
  }
}

//////////////////////////////////////////////////////////////////////

}
