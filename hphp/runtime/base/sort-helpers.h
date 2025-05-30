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

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/util/conv-10.h"
#include "hphp/util/safesort.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// For sorting VanillaVec and Vector
struct TVAccessor {
  typedef const TypedValue& ElmT;
  bool isInt(ElmT elm) const { return elm.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return isStringType(elm.m_type); }
  int64_t getInt(ElmT elm) const { return elm.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm); }
};

template<class Self, typename E>
struct AssocKeyAccessorImpl {
  typedef const E& ElmT;
  bool isInt(ElmT elm) const { return elm.hasIntKey(); }
  bool isStr(ElmT elm) const { return elm.hasStrKey(); }
  Variant getValue(ElmT elm) const {
    if (isInt(elm)) {
      return Self::getInt(elm);
    }
    assertx(isStr(elm));
    return Variant{Self::getStr(elm)};
  }
};

template<typename E> struct AssocKeyAccessor;

template<>
struct AssocKeyAccessor<VanillaDict::Elm> :
  AssocKeyAccessorImpl<AssocKeyAccessor<VanillaDict::Elm>, VanillaDict::Elm> {
  static int64_t getInt(ElmT elm) { return elm.ikey; }
  static StringData* getStr(ElmT elm) { return elm.skey; }
};

template<>
struct AssocKeyAccessor<VanillaKeyset::Elm> :
  AssocKeyAccessorImpl<AssocKeyAccessor<VanillaKeyset::Elm>, VanillaKeyset::Elm> {
  static int64_t getInt(ElmT elm) { return elm.intKey(); }
  static StringData* getStr(ElmT elm) { return elm.strKey(); }
};

template<typename E> struct AssocValAccessor {
  typedef const E& ElmT;
  bool isInt(ElmT elm) const { return elm.data.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return isStringType(elm.data.m_type); }
  int64_t getInt(ElmT elm) const { return elm.data.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm.data.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm.data); }
};


/**
 * These comparators below are used together with safesort(), and safesort
 * requires that comparators return true iff left is GREATER than right.
 */

template <typename AccessorT, int sort_flags, bool ascending>
struct IntElmCompare {
  using ElmT = typename AccessorT::ElmT;
  AccessorT acc;
  bool operator()(ElmT left, ElmT right) const {
    int64_t iLeft = acc.getInt(left);
    int64_t iRight = acc.getInt(right);
    if (sort_flags == SORT_REGULAR || sort_flags == SORT_NUMERIC) {
      return ascending ? (iLeft > iRight) : (iLeft < iRight);
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
      sLeft = sl.data();
      lenLeft = sl.size();
    }
    const StringData* sdRight = String::GetIntegerStringData(iRight);
    if (sdRight) {
      sRight = sdRight->data();
      lenRight = sdRight->size();
    } else {
      bufRight[20] = '\0';
      auto sl = conv_10(iRight, &bufRight[20]);
      sRight = sl.data();
      lenRight = sl.size();
    }
    if (sort_flags == SORT_STRING) {
      return ascending ?
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0) :
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0);
    }
    if (sort_flags == SORT_STRING_CASE) {
      return ascending ?
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0) :
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0);
    }
    if (sort_flags == SORT_LOCALE_STRING) {
      return ascending ? (strcoll(sLeft, sRight) > 0) :
                         (strcoll(sLeft, sRight) < 0);
    }
    if (sort_flags == SORT_NATURAL) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0);
    }
    if (sort_flags == SORT_NATURAL_CASE) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0);
    }
    assertx(false);
    return true;
  }
};

template <typename AccessorT, int sort_flags, bool ascending>
struct StrElmCompare {
  using ElmT = typename AccessorT::ElmT;
  AccessorT acc;
  bool operator()(ElmT left, ElmT right) const {
    StringData* sdLeft = acc.getStr(left);
    StringData* sdRight = acc.getStr(right);
    if (sort_flags == SORT_REGULAR) {
      return ascending ? (sdLeft->compare(sdRight) > 0) :
                         (sdLeft->compare(sdRight) < 0);
    }
    if (sort_flags == SORT_NUMERIC) {
      double dLeft = sdLeft->toDouble();
      double dRight = sdRight->toDouble();
      return ascending ? (dLeft > dRight) : (dLeft < dRight);
    }
    const char* sLeft = sdLeft->data();
    int lenLeft = sdLeft->size();
    const char* sRight = sdRight->data();
    int lenRight = sdRight->size();
    if (sort_flags == SORT_STRING) {
      return ascending ?
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0) :
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0);
    }
    if (sort_flags == SORT_STRING_CASE) {
      return ascending ?
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0) :
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0);
    }
    if (sort_flags == SORT_LOCALE_STRING) {
      return ascending ? (strcoll(sLeft, sRight) > 0) :
                         (strcoll(sLeft, sRight) < 0);
    }
    if (sort_flags == SORT_NATURAL) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0);
    }
    if (sort_flags == SORT_NATURAL_CASE) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0);
    }
    assertx(false);
    return true;
  }
};

template <typename AccessorT, int sort_flags, bool ascending>
struct ElmCompare {
  using ElmT = typename AccessorT::ElmT;
  AccessorT acc;
  bool operator()(ElmT left, ElmT right) const {
    // Fast paths
    if (sort_flags == SORT_REGULAR) {
      if (acc.isStr(left)) {
        if (LIKELY(acc.isStr(right))) {
          StringData* sLeft = acc.getStr(left);
          StringData* sRight = acc.getStr(right);
          return ascending ? (sLeft->compare(sRight) > 0) :
                             (sLeft->compare(sRight) < 0);
        }
      } else if (acc.isInt(left)) {
        if (LIKELY(acc.isInt(right))) {
          int64_t iLeft = acc.getInt(left);
          int64_t iRight = acc.getInt(right);
          return ascending ? (iLeft > iRight) : (iLeft < iRight);
        }
      }
    }
    if (sort_flags == SORT_NUMERIC) {
      if (acc.isInt(left)) {
        if (LIKELY(acc.isInt(right))) {
          int64_t iLeft = acc.getInt(left);
          int64_t iRight = acc.getInt(right);
          return ascending ? (iLeft > iRight) : (iLeft < iRight);
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
                     (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0) :
                     (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0);
          }
          if (sort_flags == SORT_STRING_CASE) {
            return ascending ?
                     (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0) :
                     (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0);
          }
          if (sort_flags == SORT_LOCALE_STRING) {
            return ascending ? (strcoll(sLeft, sRight) > 0) :
                               (strcoll(sLeft, sRight) < 0);
          }
          if (sort_flags == SORT_NATURAL) {
            return ascending ?
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0) :
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0);
          }
          if (sort_flags == SORT_NATURAL_CASE) {
            return ascending ?
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0) :
              (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0);
          }
        }
      }
    }
    // Slow paths
    Variant vLeft = acc.getValue(left);
    Variant vRight = acc.getValue(right);
    if (sort_flags == SORT_REGULAR) {
      return ascending ? HPHP::more(vLeft, vRight) : HPHP::less(vLeft, vRight);
    }
    if (sort_flags == SORT_NUMERIC) {
      double dLeft = vLeft.toDouble();
      double dRight = vRight.toDouble();
      return ascending ? dLeft > dRight : dLeft < dRight;
    }
    String strLeft = vLeft.toString();
    String strRight = vRight.toString();
    const char* sLeft = strLeft.data();
    int lenLeft = strLeft.size();
    const char* sRight = strRight.data();
    int lenRight = strRight.size();
    if (sort_flags == SORT_STRING) {
      return ascending ?
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) > 0) :
               (string_strcmp(sLeft, lenLeft, sRight, lenRight) < 0);
    }
    if (sort_flags == SORT_STRING_CASE) {
      return ascending ?
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) > 0) :
               (bstrcasecmp(sLeft, lenLeft, sRight, lenRight) < 0);
    }
    if (sort_flags == SORT_LOCALE_STRING) {
      return ascending ?
               (strcoll(sLeft, sRight) > 0) :
               (strcoll(sLeft, sRight) < 0);
    }
    if (sort_flags == SORT_NATURAL) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) > 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 0) < 0);
    }
    if (sort_flags == SORT_NATURAL_CASE) {
      return ascending ?
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) > 0) :
               (string_natural_cmp(sLeft, lenLeft, sRight, lenRight, 1) < 0);
    }
    assertx(false);
    return true;
  }
};

template <typename AccessorT>
struct ElmUCompare {
  using ElmT = typename AccessorT::ElmT;
  AccessorT acc;
  const CallCtx* ctx;

  ElmUCompare() : warned(false) {}

  bool operator()(ElmT left, ElmT right) const {
    TypedValue args[2] = {
      *acc.getValue(left).asTypedValue(),
      *acc.getValue(right).asTypedValue()
    };
    auto ret = Variant::attach(
      g_context->invokeFuncFew(*ctx, 2, args, RuntimeCoeffects::fixme())
    );
    if (ctx->func->takesInOutParams()) {
      assertx(ret.isArray());
      ret = ret.asCArrRef()[0];
    }
    if (LIKELY(ret.isInteger())) {
      return ret.toInt64() > 0;
    }
    if (ret.isDouble()) {
      return ret.toDouble() > 0.0;
    }
    if (ret.isString()) {
      int64_t lval; double dval;
      auto dt = ret.getStringData()->isNumericWithVal(lval, dval, 0);

      if (isIntType(dt)) {
        return lval > 0;
      } else if (isDoubleType(dt)) {
        return dval > 0;
      }
    }
    if (ret.isBoolean()) {
      if (!warned) {
        raise_warning("Sort comparators should not return a boolean because "
                      "it may result in a different sort order than expected. "
                      "The comparator should return an integer (negative if "
                      "left is less than right, positive if left is greater "
                      "than right, zero if they are equal).");
        warned = true;
      }
    }
    // If the comparator returned something other than int, double, or a
    // numeric string, we fall back to casting it to an integer.
    return ret.toInt64() > 0;
  }
private:
  mutable bool warned;
};

///////////////////////////////////////////////////////////////////////////////
}
