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

void raiseClsMethClsMethRelCompareWarning() {
  raise_notice("Comparing clsmeth with clsmeth relationally");
}

void throwInvalidClsMethToType(const char* ty) {
  SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
    "Cannot convert class method to {}", ty
  ));
}

} // namespace HPHP
