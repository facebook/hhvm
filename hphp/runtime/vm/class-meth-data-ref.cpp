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
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {
namespace {
  template<typename T>
  T& val(T& obj) { return obj; }
  template<typename T>
  T& val(T* obj) { return *obj; }

  template<typename T>
  T* ptr(T& obj) { return &obj; }
  template<typename T>
  T* ptr(T* obj) { return obj; }
}

ClsMethData& ClsMethDataRef::operator*() {
  return val(m_data);
}

const ClsMethData& ClsMethDataRef::operator*() const {
  return val(m_data);
}

ClsMethData* ClsMethDataRef::operator->() {
  return ptr(m_data);
}

const ClsMethData* ClsMethDataRef::operator->() const {
  return ptr(m_data);
}

ClsMethDataRef ClsMethDataRef::create(Class* cls, Func* func) {
  return ClsMethDataRef(cls, func);
}

void checkClsMethFuncHelper(const Func* func) {
  assertx(func->isMethod());
  if (!func->isStaticInPrologue()) throw_missing_this(func);
  if (func->isAbstract()) {
    raise_error("Cannot call abstract method %s", func->fullName()->data());
  }
}

void raiseClsMethVecCompareWarningHelper() {
  if (!RuntimeOption::EvalRaiseClsMethComparisonWarning) return;
  raise_notice("Comparing clsmeth with vec");
}

void raiseClsMethNonClsMethRelCompareWarning() {
  raise_notice("Comparing clsmeth with non-clsmeth relationally");
}

void raiseClsMethClsMethRelCompareWarning() {
  raise_notice("Comparing clsmeth with clsmeth relationally");
}

void raiseClsMethToVecWarningHelper(const char* fn /* =nullptr */) {
  if (!RuntimeOption::EvalRaiseClsMethConversionWarning) return;
  const char* t = RuntimeOption::EvalHackArrDVArrs ? "vec" : "varray";
  if (!fn) raise_notice("Implicit clsmeth to %s conversion", t);
  else raise_notice("Implicit clsmeth to %s conversion for %s()", t, fn);
}

void raiseClsMethConvertWarningHelper(const char* toType) {
  if (!RuntimeOption::EvalRaiseClsMethConversionWarning) return;
  raise_notice("Implicit clsmeth to %s conversion", toType);
}

Array clsMethToVecHelper(ClsMethDataRef clsMeth) {
  assertx(RO::EvalIsCompatibleClsMethType);

  return make_varray(clsMeth->getClsStr(), clsMeth->getFuncStr());
}

void throwInvalidClsMethToType(const char* ty) {
  SystemLib::throwRuntimeExceptionObject(folly::sformat(
    "invalid conversion from clsmeth to {}", ty
  ));
}

} // namespace HPHP
