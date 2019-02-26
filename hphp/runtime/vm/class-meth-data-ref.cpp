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
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

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
  return make_varray(clsMeth->getCls(), clsMeth->getFunc());
}

} // namespace HPHP
