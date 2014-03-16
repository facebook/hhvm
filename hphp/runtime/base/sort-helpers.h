/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SORT_HELPERS_H_
#define incl_HPHP_SORT_HELPERS_H_

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/sort-flags.h"

#include "hphp/util/safesort.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <typename AccessorT, int sort_flags, bool ascending>
struct IntElmCompare {
  typedef typename AccessorT::ElmT ElmT;
  AccessorT acc;
  bool operator()(ElmT left, ElmT right) const {
    int64_t iLeft = acc.getInt(left);
    int64_t iRight = acc.getInt(right);
    if (sort_flags == SORT_REGULAR || sort_flags == SORT_NUMERIC) {
      return ascending ? (iLeft < iRight) : (iLeft > iRight);
    }
    char bufLeft[21];
    char bufRight[21];
    const char* sLeft;
    const char* sRight;
    int lenLeft;
    int lenRight;
    // Take advantage of precomputed StringDatas if they are available
    const StringData* sdLeft = String::GetIntegerStringData(iLeft);
    if (sdLeft) {
      sLeft = sdLeft->data();
      lenLeft = sdLeft->size();
    } else {
      bufLeft[20] = '\0';
      auto sl = conv_10(iLeft, &bufLeft[20]);
      sLeft = sl.ptr;
      lenLeft = sl.len;
    }
    const StringData* sdRight = String::GetIntegerStringData(iRight);
    if (sdRight) {
      sRight = sdRight->data();
      lenRight = sdRight->size();
    } else {
      bufRight[20] = '\0';
      auto sl = conv_10(iRight, &bufRight[20]);
      sRight = sl.ptr;
      lenRight = sl.len;
    }
    if (sort_flags == SORT_STRING) {
      return ascending ?
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0) :
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0);
    }
    if (sort_flags == SORT_STRING_CASE) {
      return ascending ?
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0) :
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0);
    }
    if (sort_flags == SORT_LOCALE_STRING) {
      return ascending ? (strcoll(sLeft, sRight) < 0) :
                         (strcoll(sLeft, sRight) > 0);
    }
    if (sort_flags == SORT_NATURAL) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0);
    }
    if (sort_flags == SORT_NATURAL_CASE) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0);
    }
    assert(false);
    return true;
  }
};

template <typename AccessorT, int sort_flags, bool ascending>
struct StrElmCompare {
  typedef typename AccessorT::ElmT ElmT;
  AccessorT acc;
  bool operator()(ElmT left, ElmT right) const {
    StringData* sdLeft = acc.getStr(left);
    StringData* sdRight = acc.getStr(right);
    if (sort_flags == SORT_REGULAR) {
      return ascending ? (sdLeft->compare(sdRight) < 0) :
                         (sdLeft->compare(sdRight) > 0);
    }
    if (sort_flags == SORT_NUMERIC) {
      double dLeft = sdLeft->toDouble();
      double dRight = sdRight->toDouble();
      return ascending ? (dLeft < dRight) : (dLeft > dRight);
    }
    const char* sLeft = sdLeft->data();
    int lenLeft = sdLeft->size();
    const char* sRight = sdRight->data();
    int lenRight = sdRight->size();
    if (sort_flags == SORT_STRING) {
      return ascending ?
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0) :
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0);
    }
    if (sort_flags == SORT_STRING_CASE) {
      return ascending ?
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0) :
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0);
    }
    if (sort_flags == SORT_LOCALE_STRING) {
      return ascending ? (strcoll(sLeft, sRight) < 0) :
                         (strcoll(sLeft, sRight) > 0);
    }
    if (sort_flags == SORT_NATURAL) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0);
    }
    if (sort_flags == SORT_NATURAL_CASE) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0);
    }
    assert(false);
    return true;
  }
};

template <typename AccessorT, int sort_flags, bool ascending>
struct ElmCompare {
  typedef typename AccessorT::ElmT ElmT;
  AccessorT acc;
  bool operator()(ElmT left, ElmT right) const {
    // Fast paths
    if (sort_flags == SORT_REGULAR) {
      if (acc.isStr(left)) {
        if (LIKELY(acc.isStr(right))) {
          StringData* sLeft = acc.getStr(left);
          StringData* sRight = acc.getStr(right);
          return ascending ? (sLeft->compare(sRight) < 0) :
                             (sLeft->compare(sRight) > 0);
        }
      } else if (acc.isInt(left)) {
        if (LIKELY(acc.isInt(right))) {
          int64_t iLeft = acc.getInt(left);
          int64_t iRight = acc.getInt(right);
          return ascending ? (iLeft < iRight) : (iLeft > iRight);
        }
      }
    }
    if (sort_flags == SORT_NUMERIC) {
      if (acc.isInt(left)) {
        if (LIKELY(acc.isInt(right))) {
          int64_t iLeft = acc.getInt(left);
          int64_t iRight = acc.getInt(right);
          return ascending ? (iLeft < iRight) : (iLeft > iRight);
        }
      }
    }
    if (sort_flags == SORT_STRING || sort_flags == SORT_LOCALE_STRING ||
        sort_flags == SORT_NATURAL || sort_flags == SORT_NATURAL_CASE) {
      if (acc.isStr(left)) {
        if (LIKELY(acc.isStr(right))) {
          StringData* sdLeft = acc.getStr(left);
          StringData* sdRight = acc.getStr(right);
          const char* sLeft = sdLeft->data();
          int lenLeft = sdLeft->size();
          const char* sRight = sdRight->data();
          int lenRight = sdRight->size();
          if (sort_flags == SORT_STRING) {
            return ascending ?
                     (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0) :
                     (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0);
          }
          if (sort_flags == SORT_STRING_CASE) {
            return ascending ?
                     (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0) :
                     (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0);
          }
          if (sort_flags == SORT_LOCALE_STRING) {
            return ascending ? (strcoll(sLeft, sRight) < 0) :
                               (strcoll(sLeft, sRight) > 0);
          }
          if (sort_flags == SORT_NATURAL) {
            return ascending ?
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0) :
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0);
          }
          if (sort_flags == SORT_NATURAL_CASE) {
            return ascending ?
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0) :
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0);
          }
        }
      }
    }
    // Slow paths
    Variant vLeft = acc.getValue(left);
    Variant vRight = acc.getValue(right);
    if (sort_flags == SORT_REGULAR) {
      return ascending ? HPHP::less(vLeft, vRight) : HPHP::more(vLeft, vRight);
    }
    if (sort_flags == SORT_NUMERIC) {
      double dLeft = vLeft.toDouble();
      double dRight = vRight.toDouble();
      return ascending ? dLeft < dRight : dLeft > dRight;
    }
    String strLeft = vLeft.toString();
    String strRight = vRight.toString();
    const char* sLeft = strLeft.data();
    int lenLeft = strLeft.size();
    const char* sRight = strRight.data();
    int lenRight = strRight.size();
    if (sort_flags == SORT_STRING) {
      return ascending ?
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0) :
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0);
    }
    if (sort_flags == SORT_STRING_CASE) {
      return ascending ?
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0) :
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0);
    }
    if (sort_flags == SORT_LOCALE_STRING) {
      return ascending ?
               (strcoll(sLeft, sRight) < 0) :
               (strcoll(sLeft, sRight) > 0);
    }
    if (sort_flags == SORT_NATURAL) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0);
    }
    if (sort_flags == SORT_NATURAL_CASE) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0);
    }
    assert(false);
    return true;
  }
};

template <typename AccessorT>
struct ElmUCompare {
  typedef typename AccessorT::ElmT ElmT;
  AccessorT acc;
  const CallCtx* ctx;

  // only warn with HH syntax enabled
  ElmUCompare() : warned(!RuntimeOption::EnableHipHopSyntax) {}

  bool operator()(ElmT left, ElmT right) const {
    Variant ret;
    {
      TypedValue args[2] = {
        *acc.getValue(left).asCell(),
        *acc.getValue(right).asCell()
      };
      g_context->invokeFuncFew(ret.asTypedValue(), *ctx, 2, args);
    }
    if (ret.isInteger()) {
      return ret.toInt64() < 0;
    }
    if (ret.isDouble()) {
      return ret.toDouble() < 0.0;
    }
    if (ret.isString()) {
      int64_t lval; double dval;
      switch (ret.getStringData()->isNumericWithVal(lval, dval, 0)) {
        case KindOfInt64: return lval < 0;
        case KindOfDouble: return dval < 0;
        default: /* fall through */ break;
      }
    }
    if (!warned) {
      raise_warning("Sort comparators should not return a boolean since it "
                    "may result in a different sort order than expected. The "
                    "comparator should return an integer or double (negative "
                    "if left is less than right, positive if left is greater "
                    "than right, zero otherwise).");
      warned = true;
    }
    if (ret.isBoolean()) {
      // Match the behavior of Zend PHP for comparators that return
      // boolean values
      bool b = ret.toBoolean();
      if (b) {
        return false;
      }
      Variant ret2;
      TypedValue args[2] = {
        *acc.getValue(right).asCell(),
        *acc.getValue(left).asCell()
      };
      g_context->invokeFuncFew(ret2.asTypedValue(), *ctx, 2, args);
      if (ret2.isBoolean()) {
        return ret2.toBoolean();
      }
      // We have a wild comparator that returns boolean and non-boolean
      // values; give up and fall through to the logic below
    }
    return ret.toInt64() < 0;
  }
private:
  mutable bool warned;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SORT_HELPERS_H_
