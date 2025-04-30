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
#include "hphp/runtime/base/enum-util.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/enum-cache.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

Optional<TypedValue> maybeEnumValue(const Class* cls, TypedValue cell) {
  assertx(isEnum(cls));
  if (UNLIKELY(!tvIsInt(cell) && !tvIsString(cell) &&
               !tvIsClass(cell) && !tvIsLazyClass(cell))) {
    return std::nullopt;
  }
  auto const values = EnumCache::getValuesBuiltin(cls);
  // Manually do int-like string conversion. This is to ensure the lookup
  // succeeds below (since the values array does int-like string key conversion
  // when created, even if its a dict).
  int64_t num;
  if (isCoercibleToInteger(cell, num, "maybeEnumValue")) {
    if (values->names.exists(num)) {
      return make_tv<KindOfInt64>(num);
    } else {
      return std::nullopt;
    }
  }
  auto const val = tvClassToString(cell);

  if (values->names.exists(val)) {
    return val;
  } else {
    return std::nullopt;
  };
}

bool isCoercibleToInteger(TypedValue cell, int64_t &num, const char* callsite) {
  if (!tvIsString(cell) || !val(cell).pstr->isStrictlyInteger(num)) {
    return false;
  }

  switch (Cfg::Eval::WarnOnImplicitCoercionOfEnumValue) {
    case 0:
      return true;
    case 1:
      raise_warning("Implicitly coercing string to int in callsite: %s", callsite);
      return true;
    default:
      return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

}
