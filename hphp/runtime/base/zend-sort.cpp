/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 2014-2015 Etsy, Inc. (http://www.etsy.com)             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-array.h"
#include <string.h>

namespace HPHP {

#define COMPARE_AND_RETURN(o1, o2) do { \
  if (equal(o1, o2)) return 0; \
  else if (less(o1, o2)) return -1; \
  else return 1; \
} while (0)

static inline int zend_numeric_compare(const Variant& v1,
                                       const Variant& v2,
                                       UNUSED const void* unused) {
  auto v1_d = v1.toDouble();
  auto v2_d = v2.toDouble();
  COMPARE_AND_RETURN(v1_d, v2_d);
}

enum StringType {
  REGULAR_STRING,
  NATURAL_STRING,
  LOCALE_STRING
};

static inline int zend_string_compare_function(StringType type,
                                               const Variant& v1,
                                               const Variant& v2,
                                               bool case_insensitive) {
  int ret = 0;
  const auto &v1_str = v1.toString();
  const auto &v2_str = v2.toString();

  switch (type) {
  case REGULAR_STRING:
    if (case_insensitive) {
      ret = bstrcasecmp(v1_str.data(), v1_str.size(),
                        v2_str.data(), v2_str.size());
    } else {
      ret = string_strcmp(v1_str.data(), v1_str.size(),
                          v2_str.data(), v2_str.size());
    }
    break;

  case NATURAL_STRING:
    ret = string_natural_cmp(v1_str.data(), v1_str.size(),
                             v2_str.data(), v2_str.size(),
                             (int) case_insensitive);
    break;

  case LOCALE_STRING:
    ret = strcoll(v1_str.data(), v2_str.data());
    break;
  }

  return ret;
}

static inline int zend_locale_string_compare(const Variant& v1,
                                             const Variant& v2,
                                             UNUSED const void* unused) {
  bool case_insensitive = false;
  return zend_string_compare_function(LOCALE_STRING, v1, v2,
                                      case_insensitive);
}

static inline int zend_string_compare(const Variant& v1,
                                      const Variant& v2,
                                      UNUSED const void* unused) {
  bool case_insensitive = false;
  return zend_string_compare_function(REGULAR_STRING, v1, v2,
                                      case_insensitive);
}

static inline int zend_string_case_compare(const Variant& v1,
                                           const Variant& v2,
                                           UNUSED const void* unused) {
  bool case_insensitive = true;
  return zend_string_compare_function(REGULAR_STRING, v1, v2,
                                      case_insensitive);
}

static inline int zend_natural_string_compare(const Variant& v1,
                                              const Variant& v2,
                                              UNUSED const void* unused) {
  bool case_insensitive = false;
  return zend_string_compare_function(NATURAL_STRING, v1, v2,
                                      case_insensitive);
}

static inline int zend_natural_string_case_compare(const Variant& v1,
                                                   const Variant& v2,
                                                   UNUSED const void* unused) {
  bool case_insensitive = true;
  return zend_string_compare_function(NATURAL_STRING, v1, v2,
                                      case_insensitive);
}

static inline Variant convert_object_for_comparison(const Variant& obj) {
  if (obj.isObject()) return obj.toString();
  else return obj;
}

static inline int zend_regular_compare(const Variant& v1,
                                       const Variant& v2,
                                       UNUSED const void* unused) {
  if ((v1.isNull() || v1.isInteger() || v1.isDouble() || v1.isBoolean()) &&
      (v2.isNull() || v2.isInteger() || v2.isDouble() || v2.isBoolean())) {
    return zend_numeric_compare(v1, v2, nullptr);
  } else if (v1.isString() && v2.isString()) {
    return zend_string_compare(v1, v2, nullptr);
  } else if (v1.isNull() && v2.isString()) {
    return v2.toString().size() ? 0 : -1;
  } else if (v1.isString() && v2.isNull()) {
    return v1.toString().size() ? 0 : 1;
  } else if (v1.isNull() && v2.isObject()) {
    return -1;
  } else if (v1.isObject() && v2.isNull()) {
    return 1;
  } else if (v1.isArray() && v2.isArray()) {
    const auto &v1_arr = v1.toArray();
    const auto &v2_arr = v2.toArray();
    COMPARE_AND_RETURN(v1_arr, v2_arr);
  } else if (v1.isObject() && v2.isObject()) {
    const auto &v1_obj = v1.toObject();
    const auto &v2_obj = v2.toObject();
    COMPARE_AND_RETURN(v1_obj, v2_obj);
  } else {
    /* Out of options. Do what the zend-collator code does */
    const auto &v1_mod = convert_object_for_comparison(v1);
    const auto &v2_mod = convert_object_for_comparison(v2);
    COMPARE_AND_RETURN(v1_mod, v2_mod);
  }
}

#undef COMPARE_AND_RETURN

typedef struct {
  Array::PFUNC_CMP cmp_func;
  bool ascending;
} SortData;

static inline int zend_sort_wrapper(const Variant& v1,
                                    const Variant& v2,
                                    const void *data) {
  SortData *sort_data = (SortData *) data;
  int ret = sort_data->cmp_func(v1, v2, nullptr);
  return sort_data->ascending ? ret : (-ret);
}

static bool zend_sort_handler(bool renumber, Variant& array,
                              int sort_flags, bool ascending, bool byKey) {
  Array temp = array.toArray();
  SortData sort_data;

  sort_data.ascending = ascending;

  switch (sort_flags) {
  case SORT_NUMERIC:
    sort_data.cmp_func = zend_numeric_compare;
    break;
  case SORT_STRING_CASE:
    sort_data.cmp_func = zend_string_case_compare;
    break;
  case SORT_STRING:
    sort_data.cmp_func = zend_string_compare;
    break;
  case SORT_LOCALE_STRING:
    sort_data.cmp_func = zend_locale_string_compare;
    break;
  case SORT_NATURAL_CASE:
    sort_data.cmp_func = zend_natural_string_case_compare;
    break;
  case SORT_NATURAL:
    sort_data.cmp_func = zend_natural_string_compare;
    break;
  case SORT_REGULAR:
  default:
    sort_data.cmp_func = zend_regular_compare;
    break;
  }

  /* Sort specified array. */
  temp.sort(zend_sort_wrapper, byKey, renumber, &sort_data);

  array = temp;
  return true;
}

#define CREATE_ZEND_SORT_FUNCTION(NAME, RENUMBER, BYKEY) \
  bool zend_ ## NAME (Variant &array, int sort_flags, bool ascending) { \
    return zend_sort_handler(RENUMBER, array, sort_flags, ascending, BYKEY); \
  }

CREATE_ZEND_SORT_FUNCTION(sort, true, false);
CREATE_ZEND_SORT_FUNCTION(asort, false, false);
CREATE_ZEND_SORT_FUNCTION(ksort, false, true);

#undef CREATE_ZEND_SORT_FUNCTION

}
