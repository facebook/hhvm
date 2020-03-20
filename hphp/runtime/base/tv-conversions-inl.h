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
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

// We want to avoid potential include cycle with func.h/class.h, so putting
// forward declarations here is more feasible and simpler.
const StringData* funcToStringHelper(const Func* func);
int64_t funcToInt64Helper(const Func* func);
const StringData* classToStringHelper(const Class* cls);
Array clsMethToVecHelper(const ClsMethDataRef clsMeth);
void raiseClsMethConvertWarningHelper(const char* toType);

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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:         return !cell.m_data.parr->empty();
    case KindOfObject:        return cell.m_data.pobj->toBoolean();
    case KindOfResource:      return cell.m_data.pres->data()->o_toBoolean();
    case KindOfRecord:        raise_convert_record_to_type("bool"); break;
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:       return true;
  }
  not_reached();
}

inline int64_t tvToInt(TypedValue cell) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:          return 0;
    case KindOfBoolean:       return cell.m_data.num;
    case KindOfInt64:         return cell.m_data.num;
    case KindOfDouble:        return double_to_int64(cell.m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:        return cell.m_data.pstr->toInt64(10);
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:         return cell.m_data.parr->empty() ? 0 : 1;
    case KindOfObject:        return cell.m_data.pobj->toInt64();
    case KindOfResource:      return cell.m_data.pres->data()->o_toInt64();
    case KindOfRecord:        raise_convert_record_to_type("int"); break;
    case KindOfFunc:
      return funcToInt64Helper(cell.m_data.pfunc);
    case KindOfClass:
      return classToStringHelper(cell.m_data.pclass)->toInt64();
    case KindOfClsMeth:
      raiseClsMethConvertWarningHelper("int");
      return 1;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

template <IntishCast IC>
inline TypedValue tvToKey(TypedValue cell, const ArrayData* ad) {
  assertx(tvIsPlausible(cell));

  auto coerceKey = [&] (const StringData* str) {
    int64_t n;
    if (ad->convertKey<IC>(str, n)) {
      return make_tv<KindOfInt64>(n);
    }
    return make_tv<KindOfString>(const_cast<StringData*>(str));
  };

  if (isStringType(cell.m_type)) {
    int64_t n;
    if (ad->convertKey<IC>(cell.m_data.pstr, n)) {
      return make_tv<KindOfInt64>(n);
    }
    return cell;
  } else if (isFuncType(cell.m_type)) {
    return coerceKey(funcToStringHelper(cell.m_data.pfunc));
  } else if (isClassType(cell.m_type)) {
    return coerceKey(classToStringHelper(cell.m_data.pclass));
  }

  if (LIKELY(isIntType(cell.m_type))) return cell;

  if (!ad->useWeakKeys()) {
    throwInvalidArrayKeyException(&cell, ad);
  }
  if (checkHACArrayKeyCast()) {
    raiseHackArrCompatImplicitArrayKey(&cell);
  }

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return make_tv<KindOfPersistentString>(staticEmptyString());

    case KindOfBoolean:
      return make_tv<KindOfInt64>(cell.m_data.num);

    case KindOfDouble:
      return make_tv<KindOfInt64>(double_to_int64(cell.m_data.dbl));

    case KindOfResource:
      return make_tv<KindOfInt64>(cell.m_data.pres->data()->o_toInt64());

    case KindOfDArray:
    case KindOfPersistentDArray:
    case KindOfVArray:
    case KindOfPersistentVArray:
    case KindOfClsMeth:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfObject:
    case KindOfRecord:
      raise_warning("Invalid operand type was used: Invalid type used as key");
      return make_tv<KindOfNull>();

    case KindOfInt64:
    case KindOfString:
    case KindOfPersistentString:
    case KindOfFunc:
    case KindOfClass:
      break;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

inline TypedNum stringToNumeric(const StringData* sd) {
  int64_t ival;
  double dval;
  auto const dt = sd->isNumericWithVal(ival, dval, true /* allow_errors */);
  return dt == KindOfInt64 ? make_tv<KindOfInt64>(ival) :
         dt == KindOfDouble ? make_tv<KindOfDouble>(dval) :
         make_tv<KindOfInt64>(0);
}

///////////////////////////////////////////////////////////////////////////////

}
