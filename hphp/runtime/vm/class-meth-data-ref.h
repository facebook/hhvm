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

  ClsMethDataRef(Class* cls, Func* func) : data(ClsMethData::make(cls, func)) {}

  static ClsMethDataRef create(Class* cls, Func* func) {
    return ClsMethDataRef(cls, func);
  }

  static void Release(ClsMethDataRef clsMeth) noexcept {
    clsMeth->release();
  }

  ClsMethData& operator*() {
    return *data;
  }

  const ClsMethData& operator*() const {
    return *data;
  }

  ClsMethData* operator->() {
    return data;
  }

  const ClsMethData* operator->() const {
    return data;
  }

  bool operator==(ClsMethDataRef o) const {
    return (data->getCls() == o->getCls()) && (data->getFunc() == o->getFunc());
  }

  const ClsMethData* get() const {
    return operator->();
  }

private:
  ClsMethData* data;
};

ALWAYS_INLINE void decRefClsMeth(ClsMethDataRef clsMeth) {
  clsMeth->decRefAndRelease();
}

void raiseClsMethToVecWarningHelper(const char* fn = nullptr);

void raiseClsMethConvertWarningHelper(const char* toType);

Array clsMethToVecHelper(ClsMethDataRef clsMeth);

} // namespace HPHP
