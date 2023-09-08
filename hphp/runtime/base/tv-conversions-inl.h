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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-conv-notice.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

// We want to avoid potential include cycle with func.h/class.h, so putting
// forward declarations here is more feasible and simpler.
const StringData* classToStringHelper(const Class* cls);
[[noreturn]] void invalidFuncConversion(const char*);

///////////////////////////////////////////////////////////////////////////////

inline bool tvToBool(TypedValue cell) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:          return false;
    case KindOfBoolean:       return cell.m_data.num;
    case KindOfInt64:         return cell.m_data.num != 0;
    case KindOfDouble:        return cell.m_data.dbl != 0;
    case KindOfPersistentString:
    case KindOfString:        return cell.m_data.pstr->toBoolean();
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:        return !cell.m_data.parr->empty();
    case KindOfObject:        return cell.m_data.pobj->toBoolean();
    case KindOfResource:      return cell.m_data.pres->data()->o_toBoolean();
    case KindOfEnumClassLabel:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfLazyClass:     return true;
  }
  not_reached();
}

inline int64_t tvToInt(TypedValue cell) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return 0;
    case KindOfBoolean:
    case KindOfInt64:
      return cell.m_data.num;
    case KindOfDouble:
      return double_to_int64(cell.m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:
      return cell.m_data.pstr->toInt64(10);
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentVec:
    case KindOfVec:
      case KindOfPersistentKeyset:
    case KindOfKeyset:
      return cell.m_data.parr->empty() ? 0 : 1;
    case KindOfObject:
      return cell.m_data.pobj->toInt64();
    case KindOfResource:
      return cell.m_data.pres->data()->o_toInt64();
    case KindOfRFunc:         raise_convert_rfunc_to_type("int"); break;
    case KindOfFunc:
      invalidFuncConversion("int");
    case KindOfClass:
      return classToStringHelper(cell.m_data.pclass)->toInt64();
    case KindOfLazyClass:
      return lazyClassToStringHelper(cell.m_data.plazyclass)->toInt64();
    case KindOfClsMeth:
      throwInvalidClsMethToType("int");
    case KindOfRClsMeth:      raise_convert_rcls_meth_to_type("int"); break;
    case KindOfEnumClassLabel:
      raise_convert_ecl_to_type("int");
  }
  not_reached();
}
///////////////////////////////////////////////////////////////////////////////

template <IntishCast IC>
inline TypedValue tvToKey(TypedValue cell, const ArrayData* ad) {
  assertx(tvIsPlausible(cell));

  if (isStringType(cell.m_type)) {
    int64_t n;
    if (IC == IntishCast::Cast && ArrayData::IntishCastKey(cell.m_data.pstr, n)) {
      return make_tv<KindOfInt64>(n);
    }
    return cell;
  }

  if (LIKELY(isIntType(cell.m_type))) return cell;

  throwInvalidArrayKeyException(&cell, ad);
}

///////////////////////////////////////////////////////////////////////////////

inline TypedValue tvClassToString(TypedValue key) {
  if (isClassType(type(key))) {
    auto const keyStr = classToStringHelper(val(key).pclass);
    return make_tv<KindOfPersistentString>(keyStr);
  }
  if (isLazyClassType(type(key))) {
    auto const keyStr = lazyClassToStringHelper(val(key).plazyclass);
    return make_tv<KindOfPersistentString>(keyStr);
  }
  return key;
}

///////////////////////////////////////////////////////////////////////////////

}
