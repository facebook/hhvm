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

#ifndef incl_HPHP_SORT_FLAGS_H_
#define incl_HPHP_SORT_FLAGS_H_

#include <stdint.h>
#include "hphp/util/assertions.h"

namespace HPHP {

enum SortFlavor { IntegerSort, StringSort, GenericSort };

enum SortFlags {
  SORT_REGULAR = 0,
  SORT_NUMERIC = 1,
  SORT_STRING = 2,
  SORT_LOCALE_STRING = 5,
  SORT_NATURAL = 6,
  SORT_FLAG_CASE = 8,
  SORT_NATURAL_CASE = SORT_NATURAL | SORT_FLAG_CASE,
  SORT_STRING_CASE = SORT_STRING | SORT_FLAG_CASE
};

enum SortFunction {
  // Special encoding to save instructions:
  // 1. MSB is 1 (negative) if sorting with user-defined comparison function.
  // 2. Bit 0: 0 if ascending, 1 if descending. U*SORT considered as ascending.
  // This makes it possible to do things like (RSORT ^ ascending).
  // 3. Bit 1: 1 if SORT/RSORT/USORT, 0 otherwise.
  // 4. Bit 2: 1 if ASORT/ARSORT/UASORT, 0 otherwise.
  // 5. If Bit 1 and 2 are both zero. It is KSORT.
  _SORTFUNC_SIGNMASK = -16,             // MSB set, lower 4 bits are 0
  SORTFUNC_KSORT = 0,                   // make ksort easier to test
  SORTFUNC_KRSORT = SORTFUNC_KSORT + 1,
  SORTFUNC_UKSORT = SORTFUNC_KSORT | _SORTFUNC_SIGNMASK,

  SORTFUNC_SORT = 2,
  SORTFUNC_RSORT = SORTFUNC_SORT + 1,
  SORTFUNC_USORT = SORTFUNC_SORT | _SORTFUNC_SIGNMASK,

  SORTFUNC_ASORT = 4,
  SORTFUNC_ARSORT = SORTFUNC_ASORT + 1,
  SORTFUNC_UASORT = SORTFUNC_ASORT | _SORTFUNC_SIGNMASK
};

// Return true if the sorting has a user-defined comparison function
inline bool hasUserDefinedCmp(SortFunction s) {
  return s < 0;
}

// Return true if sort/rsort/usort
inline bool isSortFamily(SortFunction s) {
  return uint8_t(s) & SORTFUNC_SORT;
}

// Return true if asort/arsort/uasort
inline bool isASortFamily(SortFunction s) {
  return uint8_t(s) & SORTFUNC_ASORT;
}

// Return true if ksort/krsort/uksort
inline bool isKSortFamily(SortFunction s) {
  return (uint8_t(s) & 6) == 0;
}

// Return true if result can be represented as a PackedArray
inline bool supportedByPacked(SortFunction s) {
  // I put some static checking for encoding here.
  static_assert(SORTFUNC_USORT < 0, "");
  static_assert(SORTFUNC_UKSORT < 0, "");
  static_assert(SORTFUNC_UASORT < 0, "");
  static_assert(SORTFUNC_SORT > 0, "");
  static_assert(SORTFUNC_KSORT == 0, "");
  static_assert(SORTFUNC_ASORT > 0, "");
  static_assert(SORTFUNC_ASORT > 0, "");
  static_assert(SORTFUNC_RSORT == SORTFUNC_SORT + 1, "");
  static_assert(SORTFUNC_KRSORT == SORTFUNC_KSORT + 1, "");
  static_assert(SORTFUNC_ARSORT == SORTFUNC_ASORT + 1, "");

  if (s == SORTFUNC_KSORT) {
    return true;                        // ksort trivail for PackedArray
  }
  return isSortFamily(s);
}

inline SortFunction getSortFunction(SortFunction s, bool ascending = true) {
  assert(!hasUserDefinedCmp(s));
  // ascending: LSB == 0
  return static_cast<SortFunction>(s & ~static_cast<int>(ascending));
}

}

#endif // incl_HPHP_SORT_FLAGS_H_
