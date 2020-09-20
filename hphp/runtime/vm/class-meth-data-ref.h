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

#include "hphp/runtime/vm/class-meth-data.h"

namespace HPHP {

struct Array;
struct Class;
struct Func;

struct ClsMethDataRef {
  ClsMethDataRef() = default;

  static ClsMethDataRef create(Class* cls, Func* func);

  static void Release(ClsMethDataRef clsMeth) noexcept {
    clsMeth->release();
  }

  ClsMethData& operator*();
  const ClsMethData& operator*() const;
  ClsMethData* operator->();
  const ClsMethData* operator->() const;

  bool operator==(ClsMethDataRef o) const {
    auto const thiz = get();
    return (thiz->getCls() == o->getCls()) &&
           (thiz->getFunc() == o->getFunc());
  }

  const ClsMethData* get() const {
    return operator->();
  }

private:
  ClsMethDataRef(Class* cls, Func* func)
    : m_data(ClsMethData::make(cls, func))
    {}

  ClsMethData::cls_meth_t m_data;
};

ALWAYS_INLINE void decRefClsMeth(ClsMethDataRef clsMeth) {
  clsMeth->decRefAndRelease();
}

ALWAYS_INLINE void incRefClsMeth(ClsMethDataRef clsMeth) {
  clsMeth->incRef();
}

ALWAYS_INLINE bool checkCountClsMeth(ClsMethDataRef clsMeth) {
  return clsMeth->checkRefCount();
}

ALWAYS_INLINE bool isRefCountedClsMeth(ClsMethDataRef clsMeth) {
  return clsMeth->isRefCountedType();
}

void checkClsMethFuncHelper(const Func* f);

void raiseClsMethVecCompareWarningHelper();
void raiseClsMethNonClsMethRelCompareWarning();
void raiseClsMethToVecWarningHelper(const char* fn = nullptr);
void raiseClsMethConvertWarningHelper(const char* toType);

Array clsMethToVecHelper(ClsMethDataRef clsMeth);

} // namespace HPHP
