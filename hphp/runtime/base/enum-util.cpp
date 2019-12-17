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

bool enumHasValue(const Class* cls, const TypedValue* cell) {
  assertx(isEnum(cls));
  auto const type = cell->m_type;
  if (UNLIKELY(type != KindOfInt64 && !isStringType(type))) {
    return false;
  }
  auto const values = EnumCache::getValuesBuiltin(cls);
  // Manually perform int-like key conversion even if names is a dict, for
  // backwards compatibility.
  int64_t num;
  if (isStringType(type) &&
      cell->m_data.pstr->isStrictlyInteger(num)) {
   return values->names.exists(num);
  }
  return values->names.exists(*cell);
}

///////////////////////////////////////////////////////////////////////////////

}
