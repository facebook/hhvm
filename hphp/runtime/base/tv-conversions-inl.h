/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/type-conversions.h" // toInt64(double)

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/string-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline bool cellToBool(Cell cell) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:          return false;
    case KindOfBoolean:       return cell.m_data.num;
    case KindOfInt64:         return cell.m_data.num != 0;
    case KindOfDouble:        return cell.m_data.dbl != 0;
    case KindOfPersistentString:
    case KindOfString:        return cell.m_data.pstr->toBoolean();
    case KindOfPersistentArray:
    case KindOfArray:         return !cell.m_data.parr->empty();
    case KindOfObject:        return cell.m_data.pobj->toBoolean();
    case KindOfResource:      return cell.m_data.pres->data()->o_toBoolean();
    case KindOfRef:
    case KindOfClass:         break;
  }
  not_reached();
}

inline int64_t cellToInt(Cell cell) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:          return 0;
    case KindOfBoolean:       return cell.m_data.num;
    case KindOfInt64:         return cell.m_data.num;
    case KindOfDouble:        return toInt64(cell.m_data.dbl);
    case KindOfPersistentString:
    case KindOfString:        return cell.m_data.pstr->toInt64(10);
    case KindOfPersistentArray:
    case KindOfArray:         return cell.m_data.parr->empty() ? 0 : 1;
    case KindOfObject:        return cell.m_data.pobj->toInt64();
    case KindOfResource:      return cell.m_data.pres->data()->o_toInt64();
    case KindOfRef:
    case KindOfClass:         break;
  }
  not_reached();
}

inline double cellToDouble(Cell cell) {
  Cell tmp;
  cellDup(cell, tmp);
  tvCastToDoubleInPlace(&tmp);
  return tmp.m_data.dbl;
}

inline TypedNum stringToNumeric(const StringData* sd) {
  int64_t ival;
  double dval;
  auto const dt = sd->isNumericWithVal(ival, dval, true /* allow_errors */);
  return dt == KindOfInt64 ? make_tv<KindOfInt64>(ival) :
         dt == KindOfDouble ? make_tv<KindOfDouble>(dval) :
         make_tv<KindOfInt64>(0);
}

//////////////////////////////////////////////////////////////////////

}
