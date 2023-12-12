/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
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
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/base/intl-convert.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/zend/zend-strtod.h"

namespace HPHP {

#define UCHARS(len) ((len) / sizeof(UChar))
#define UBYTES(len) ((len) * sizeof(UChar))

static Variant collator_convert_string_to_number_if_possible(const Variant& str);

static double collator_u_strtod(const UChar *nptr, UChar **endptr) {
  const UChar *u = nptr, *nstart;
  UChar c = *u;
  int any = 0;

  while (u_isspace(c)) {
    c = *++u;
  }
  nstart = u;

  if (c == 0x2D /*'-'*/ || c == 0x2B /*'+'*/) {
    c = *++u;
  }

  while (c >= 0x30 /*'0'*/ && c <= 0x39 /*'9'*/) {
    any = 1;
    c = *++u;
  }

  if (c == 0x2E /*'.'*/) {
    c = *++u;
    while (c >= 0x30 /*'0'*/ && c <= 0x39 /*'9'*/) {
      any = 1;
      c = *++u;
    }
  }

  if ((c == 0x65 /*'e'*/ || c == 0x45 /*'E'*/) && any) {
    const UChar *e = u;
    int any_exp = 0;

    c = *++u;
    if (c == 0x2D /*'-'*/ || c == 0x2B /*'+'*/) {
      c = *++u;
    }

    while (c >= 0x30 /*'0'*/ && c <= 0x39 /*'9'*/) {
      any_exp = 1;
      c = *++u;
    }

    if (!any_exp) {
      u = e;
    }
  }

  if (any) {
    char buf[64], *numbuf, *bufpos;
    int length = u - nstart;
    double value;

    if (length < (int)sizeof(buf)) {
      numbuf = buf;
    } else {
      numbuf = (char *) req::malloc_noptrs(length + 1);
    }

    SCOPE_EXIT {
      if (numbuf != buf) {
        req::free(numbuf);
      }
    };

    bufpos = numbuf;

    while (nstart < u) {
      *bufpos++ = (char) *nstart++;
    }

    *bufpos = '\0';
    value = zend_strtod(numbuf, nullptr);

    if (endptr != nullptr) {
      *endptr = (UChar *)u;
    }

    return value;
  }

  if (endptr != nullptr) {
    *endptr = (UChar *)nptr;
  }

  return 0;
}

static long collator_u_strtol(const UChar *nptr, UChar **endptr,
                              int base) {
  const UChar *s = nptr;
  unsigned long acc;
  UChar c;
  unsigned long cutoff;
  int neg = 0, any, cutlim;

  if (s == nullptr) {
    errno = ERANGE;
    if (endptr != nullptr) {
      *endptr = nullptr;
    }
    return 0;
  }

  /*
   * Skip white space and pick up leading +/- sign if any.
   * If base is 0, allow 0x for hex and 0 for octal, else
   * assume decimal; if base is already 16, allow 0x.
   */
  do {
    c = *s++;
  } while (u_isspace(c));
  if (c == 0x2D /*'-'*/) {
    neg = 1;
    c = *s++;
  } else if (c == 0x2B /*'+'*/)
    c = *s++;
  if ((base == 0 || base == 16) &&
      (c == 0x30 /*'0'*/)
     && (*s == 0x78 /*'x'*/ || *s == 0x58 /*'X'*/)) {
    c = s[1];
    s += 2;
    base = 16;
  }
  if (base == 0)
    base = (c == 0x30 /*'0'*/) ? 8 : 10;

  /*
   * Compute the cutoff value between legal numbers and illegal
   * numbers.  That is the largest legal value, divided by the
   * base.  An input number that is greater than this value, if
   * followed by a legal input character, is too big.  One that
   * is equal to this value may be valid or not; the limit
   * between valid and invalid numbers is then based on the last
   * digit.  For instance, if the range for longs is
   * [-2147483648..2147483647] and the input base is 10,
   * cutoff will be set to 214748364 and cutlim to either
   * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
   * a value > 214748364, or equal but the next digit is > 7 (or 8),
   * the number is too big, and we will return a range error.
   *
   * Set any if any `digits' consumed; make it negative to indicate
   * overflow.
   */
  cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
  cutlim = cutoff % (unsigned long)base;
  cutoff /= (unsigned long)base;
  for (acc = 0, any = 0;; c = *s++) {
    if (c >= 0x30 /*'0'*/ && c <= 0x39 /*'9'*/)
      c -= 0x30 /*'0'*/;
    else if (c >= 0x41 /*'A'*/ && c <= 0x5A /*'Z'*/)
      c -= 0x41 /*'A'*/ - 10;
    else if (c >= 0x61 /*'a'*/ && c <= 0x7A /*'z'*/)
      c -= 0x61 /*'a'*/ - 10;
    else
      break;
    if (c >= base)
      break;

    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
      any = -1;
    else {
      any = 1;
      acc *= base;
      acc += c;
    }
  }
  if (any < 0) {
    acc = neg ? LONG_MIN : LONG_MAX;
    errno = ERANGE;
  } else if (neg)
    acc = -acc;
  if (endptr != nullptr)
    *endptr = (UChar *)(any ? s - 1 : nptr);
  return (acc);
}


static DataType collator_is_numeric(UChar *str, int length, int64_t *lval,
                                    double *dval, int allow_errors ) {
  int64_t local_lval;
  double local_dval;
  UChar *end_ptr_long, *end_ptr_double;
  int conv_base=10;

  if (!length) {
    return KindOfNull;
  }

  /* handle hex numbers */
  if (length>=2 && str[0]=='0' && (str[1]=='x' || str[1]=='X')) {
    conv_base=16;
  }

  errno=0;
  local_lval = collator_u_strtol(str, &end_ptr_long, conv_base);
  if (errno != ERANGE) {
    if (end_ptr_long == str+length) { /* integer string */
      if (lval) {
        *lval = local_lval;
      }
      return KindOfInt64;
    } else if (end_ptr_long == str &&
               *end_ptr_long != '\0' &&
               *str != '.' &&
               *str != '-') { /* ignore partial string matches */
      return KindOfNull;
    }
  } else {
    end_ptr_long = nullptr;
  }

  if (conv_base == 16) { /* hex string, under UNIX strtod() messes it up */
    /* TODO: keep compatibility with is_numeric_string() here? */
    return KindOfNull;
  }

  local_dval = collator_u_strtod(str, &end_ptr_double);
  if (local_dval == 0 && end_ptr_double == str) {
    end_ptr_double = nullptr;
  } else {
    if (end_ptr_double == str+length) { /* floating point string */
      if (!std::isfinite(local_dval)) {
        /* "inf","nan" and maybe other weird ones */
        return KindOfNull;
      }

      if (dval) {
        *dval = local_dval;
      }
      return KindOfDouble;
    }
  }

  if (!allow_errors) {
    return KindOfNull;
  }
  if (allow_errors == -1) {
    raise_notice("A non well formed numeric value encountered");
  }

  if (allow_errors) {
    if (end_ptr_double > end_ptr_long && dval) {
      *dval = local_dval;
      return KindOfDouble;
    } else if (end_ptr_long && lval) {
      *lval = local_lval;
      return KindOfInt64;
    }
  }
  return KindOfNull;
}

static String intl_convert_str_utf8_to_utf16(const String& utf8_str,
                                             UErrorCode * status) {
  UChar* ustr = nullptr;
  int ustr_len = 0;
  intl_convert_utf8_to_utf16(&ustr, &ustr_len,
                             utf8_str.data(), utf8_str.length(),
                             status);
  if (U_FAILURE(*status)) {
    return empty_string();
  }
  return String((char*)ustr, UBYTES(ustr_len), AttachString);
}

static String intl_convert_str_utf16_to_utf8(const String& utf16_str,
                                             UErrorCode * status) {
  char* str = nullptr;
  int str_len = 0;
  intl_convert_utf16_to_utf8(&str, &str_len,
                             (UChar*)(utf16_str.data()),
                             UCHARS(utf16_str.length()),
                             status);
  if (U_FAILURE(*status)) {
    return "";
  }
  return String(str, str_len, AttachString);
}

static Variant collator_convert_string_to_number(const Variant& str) {
  Variant num = collator_convert_string_to_number_if_possible(str);
  if (same(num, false)) {
    /* String wasn't converted => return zero. */
    return 0;
  }
  return num;
}

static Variant collator_convert_string_to_double(const Variant& str) {
  Variant num = collator_convert_string_to_number(str);
  return num.toDouble();
}

static Variant collator_convert_string_to_number_if_possible(const Variant& str) {
  int64_t lval     = 0;
  double dval    = 0;

  if (!str.isString()) return false;

  DataType ret = collator_is_numeric((UChar*)(str.toString().data()),
                                     UCHARS(str.toString().length()),
                                     &lval, &dval, 1);
  if (ret == KindOfInt64) return lval;
  if (ret == KindOfDouble) return dval;
  return false;
}

static Variant collator_convert_object_to_string(const Variant& obj) {
  if (!obj.isObject()) return obj;
  String str;
  try {
    str = obj.toString();
  } catch (Exception& ) {
    return obj;
  }
  UErrorCode status;
  String ustr = intl_convert_str_utf8_to_utf16(str, &status);
  if (U_FAILURE(status)) {
    raise_warning("Error casting object to string in "
                  "collator_convert_object_to_string()");
    return uninit_null();
  }
  return ustr;
}

static void collator_convert_array_from_utf16_to_utf8(Array &array,
                                                      UErrorCode * status) {
  for (ArrayIter iter(array); iter; ++iter) {
    auto const tv = iter.secondVal();
    /* Process string values only. */
    if (!isStringType(type(tv))) continue;
    String str = intl_convert_str_utf16_to_utf8(StrNR(val(tv).pstr), status);
    if (U_FAILURE(*status)) return;
    /* Update current value with the converted value. */
    Variant key = iter.first();
    array.set(key, str);
  }
}

static void collator_convert_array_from_utf8_to_utf16(Array &array,
                                                      UErrorCode * status) {
  for (ArrayIter iter(array); iter; ++iter) {
    auto const tv = iter.secondVal();
    /* Process string values only. */
    if (!isStringType(type(tv))) continue;
    String str = intl_convert_str_utf8_to_utf16(StrNR(val(tv).pstr), status);
    if (U_FAILURE(*status)) return;
    /* Update current value with the converted value. */
    Variant key = iter.first();
    array.set(key, str);
  }
}

static Variant collator_normalize_sort_argument(const Variant& arg) {
  if (!arg.isString()) return arg;

  Variant n_arg = collator_convert_string_to_number_if_possible(arg);
  if (same(n_arg, false)) {
    /* Conversion to number failed. */
    UErrorCode status;
    n_arg = intl_convert_str_utf16_to_utf8(arg.toString(), &status);
    if (U_FAILURE(status)) {
      raise_warning("Error converting utf16 to utf8 in "
                    "collator_normalize_sort_argument()");
    }
  }
  return n_arg;
}

static int collator_regular_compare_function(const Variant& v1, const Variant& v2,
                                             const void *data,
                                             bool ascending) {
  Variant str1 = collator_convert_object_to_string(v1);
  Variant str2 = collator_convert_object_to_string(v2);
  Variant num1;
  Variant num2;
  Variant norm1;
  Variant norm2;

  /* If both args are strings AND either of args is not numeric string
   * then use ICU-compare. Otherwise PHP-compare. */
  if (str1.isString() && str2.isString()) {
    num1 = collator_convert_string_to_number_if_possible(str1);
    if (!same(num1, false)) {
      num2 = collator_convert_string_to_number_if_possible(str2);
    }
    if (same(num1, false) || same(num2, false)) {
      assertx(data);
      int ret = ucol_strcoll((const UCollator *)data,
                             (UChar*)(str1.toString().data()),
                             UCHARS(str1.toString().length()),
                             (UChar*)(str2.toString().data()),
                             UCHARS(str2.toString().length()));
      return ascending ? ret : (-ret);
    }
  }

  /* num1 is set if str1 and str2 are strings. */
  if (!num1.isNull()) {
    if (same(num1, false)) {
      /* str1 is string but not numeric string just convert it to utf8. */
      UErrorCode status;
      norm1 = intl_convert_str_utf16_to_utf8(str1.toString(), &status);
      if (U_FAILURE(status)) {
        raise_warning("Error converting utf16 to utf8 in "
                      "collator_regular_compare_function()");
      }
      /* num2 is not set but str2 is string => do normalization. */
      norm2 = collator_normalize_sort_argument(str2);
    } else {
      /* str1 is numeric strings => passthru to PHP-compare. */
      norm1 = num1;
      norm2 = num2;
    }
  } else {
    /* str1 or str2 is not a string => do normalization. */
    norm1 = collator_normalize_sort_argument(str1);
    norm2 = collator_normalize_sort_argument(str2);
  }
  if (ascending) {
    if (less(norm1, norm2)) return -1;
    if (equal(norm1, norm2)) return 0;
    return 1;
  }
  if (less(norm1, norm2)) return 1;
  if (equal(norm1, norm2)) return 0;
  return -1;
}

static int collator_regular_compare_ascending(const Variant& v1, const Variant& v2,
                                              const void *data) {
  return collator_regular_compare_function(v1, v2, data, true);
}

static int collator_regular_compare_descending(const Variant& v1, const Variant& v2,
                                               const void *data) {
  return collator_regular_compare_function(v1, v2, data, false);
}

static int
collator_numeric_compare_function(const Variant& v1, const Variant& v2,
                                  const void* /*data*/, bool ascending) {
  Variant num1;
  Variant num2;

  if (v1.isString()) {
    num1 = collator_convert_string_to_double(v1);
  } else {
    num1 = v1.toDouble();
  }
  if (v2.isString()) {
    num2 = collator_convert_string_to_double(v2);
  } else {
    num2 = v2.toDouble();
  }
  if (ascending) {
    if (less(num1, num2)) return -1;
    if (equal(num1, num2)) return 0;
    return 1;
  }
  if (less(num1, num2)) return 1;
  if (equal(num1, num2)) return 0;
  return -1;
}

static int collator_numeric_compare_ascending(const Variant& v1, const Variant& v2,
                                              const void *data) {
  return collator_numeric_compare_function(v1, v2, data, true);
}

static int collator_numeric_compare_descending(const Variant& v1, const Variant& v2,
                                               const void *data) {
  return collator_numeric_compare_function(v1, v2, data, false);
}

static int collator_string_compare_function(const Variant& v1, const Variant& v2,
                                            const void *data,
                                            bool ascending) {
  assertx(data);
  String str1;
  if (v1.isString()) {
    str1 = v1.toString();
  } else {
    UErrorCode status;
    str1 = intl_convert_str_utf8_to_utf16(v1.toString(), &status);
    if (U_FAILURE(status)) {
      raise_warning("Error converting utf8 to utf16 in "
                    "collator_string_compare_function()");
    }
  }
  String str2;
  if (v2.isString()) {
    str2 = v2.toString();
  } else {
    UErrorCode status;
    str2 = intl_convert_str_utf8_to_utf16(v2.toString(), &status);
    if (U_FAILURE(status)) {
      raise_warning("Error converting utf8 to utf16 in "
                    "collator_string_compare_function()");
    }
  }

  int ret = ucol_strcoll((const UCollator *)data,
                         (UChar*)(str1.data()),
                         UCHARS(str1.length()),
                         (UChar*)(str2.data()),
                         UCHARS(str2.length()));
  return ascending ? ret : (-ret);
}

static int collator_string_compare_ascending(const Variant& v1, const Variant& v2,
                                            const void *data) {
  return collator_string_compare_function(v1, v2, data, true);
}

static int collator_string_compare_descending(const Variant& v1, const Variant& v2,
                                             const void *data) {
  return collator_string_compare_function(v1, v2, data, false);
}

static bool collator_sort_internal(bool renumber, Variant &array,
                                   int sort_flags, bool ascending, bool byKey,
                                   UCollator *coll, Intl::IntlError *errcode) {
  assertx(coll);
  errcode->clearError();
  Array temp = array.toArray();
  Array::PFUNC_CMP cmp_func;

  switch (sort_flags) {
  case COLLATOR_SORT_NUMERIC:
    cmp_func = ascending ? collator_numeric_compare_ascending
                         : collator_numeric_compare_descending;
    break;
  case COLLATOR_SORT_STRING:
    cmp_func = ascending ? collator_string_compare_ascending
                         : collator_string_compare_descending;
    break;
  case COLLATOR_SORT_REGULAR:
  default:
    cmp_func = ascending ? collator_regular_compare_ascending
                         : collator_regular_compare_descending;
    break;
  }

  /* Convert strings in the specified array from UTF-8 to UTF-16. */
  UErrorCode error = U_ZERO_ERROR;
  collator_convert_array_from_utf8_to_utf16(temp, &error);
  if (U_FAILURE(error)) {
    errcode->setError(error, "Error converting array from UTF-8 to UTF-16");
    return false;
  }

  /* Sort specified array. */
  temp.sort(cmp_func, byKey, renumber, coll);

  /* Convert strings in the specified array back to UTF-8. */
  errcode->clearError();
  error = U_ZERO_ERROR;
  collator_convert_array_from_utf16_to_utf8(temp, &error);
  if (U_FAILURE(error)) {
    errcode->setError(error, "Error converting array from UTF-16 to UTF-8");
    return false;
  }
  if (renumber) {
    array = temp.toVec();
  } else {
    array = temp.toDict();
  }
  return true;
}

bool collator_sort(Variant &array, int sort_flags, bool ascending,
                   UCollator *coll, Intl::IntlError *errcode) {
  assertx(coll);
  bool byKey = false;
  bool ret = collator_sort_internal(true, array, sort_flags, ascending, byKey,
                                    coll, errcode);
  return ret;
}

bool collator_asort(Variant &array, int sort_flags, bool ascending,
                    UCollator *coll, Intl::IntlError *errcode) {
  assertx(coll);
  bool byKey = false;
  bool ret = collator_sort_internal(false, array, sort_flags, ascending, byKey,
                                    coll, errcode);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
