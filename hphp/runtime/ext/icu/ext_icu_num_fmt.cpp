/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/icu/ext_icu_num_fmt.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/ext/ext_string.h"

namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////
// Internal resource data

const StaticString s_NumberFormatter("NumberFormatter");

#define UNUM_DECL(cns, val) \
  const int64_t q_NumberFormatter_ ## cns = val; \
  const StaticString s_NumberFormatter_ ## cns(#cns)

#define UNUM_ICU_DECL(cns) \
  const int64_t q_NumberFormatter_ ## cns = UNUM_ ## cns; \
  const StaticString s_NumberFormatter_ ## cns(#cns)

#define UNUM(cns) q_NumberFormatter_ ## cns

/* UNumberFormatStyle constants */
UNUM_ICU_DECL(PATTERN_DECIMAL);
UNUM_ICU_DECL(DECIMAL);
UNUM_ICU_DECL(CURRENCY);
UNUM_ICU_DECL(PERCENT);
UNUM_ICU_DECL(SCIENTIFIC);
UNUM_ICU_DECL(SPELLOUT);
UNUM_ICU_DECL(ORDINAL);
UNUM_ICU_DECL(DURATION);
UNUM_ICU_DECL(PATTERN_RULEBASED);
UNUM_ICU_DECL(IGNORE);
UNUM_DECL(DEFAULT_STYLE, UNUM_DEFAULT);

/* workaround for ICU bug */
#if U_ICU_VERSION_MAJOR_NUM == 3 && U_ICU_VERSION_MINOR_NUM < 8
#define UNUM_ROUND_HALFEVEN UNUM_FOUND_HALFEVEN
#endif

/* UNumberFormatRoundingMode */
UNUM_ICU_DECL(ROUND_CEILING);
UNUM_ICU_DECL(ROUND_FLOOR);
UNUM_ICU_DECL(ROUND_DOWN);
UNUM_ICU_DECL(ROUND_UP);
UNUM_ICU_DECL(ROUND_HALFEVEN);
UNUM_ICU_DECL(ROUND_HALFDOWN);
UNUM_ICU_DECL(ROUND_HALFUP);

/* UNumberFormatPadPosition */
UNUM_ICU_DECL(PAD_BEFORE_PREFIX);
UNUM_ICU_DECL(PAD_AFTER_PREFIX);
UNUM_ICU_DECL(PAD_BEFORE_SUFFIX);
UNUM_ICU_DECL(PAD_AFTER_SUFFIX);

/* UNumberFormatAttribute */
UNUM_ICU_DECL(PARSE_INT_ONLY);
UNUM_ICU_DECL(GROUPING_USED);
UNUM_ICU_DECL(DECIMAL_ALWAYS_SHOWN);
UNUM_ICU_DECL(MAX_INTEGER_DIGITS);
UNUM_ICU_DECL(MIN_INTEGER_DIGITS);
UNUM_ICU_DECL(INTEGER_DIGITS);
UNUM_ICU_DECL(MAX_FRACTION_DIGITS);
UNUM_ICU_DECL(MIN_FRACTION_DIGITS);
UNUM_ICU_DECL(FRACTION_DIGITS);
UNUM_ICU_DECL(MULTIPLIER);
UNUM_ICU_DECL(GROUPING_SIZE);
UNUM_ICU_DECL(ROUNDING_MODE);
UNUM_ICU_DECL(FORMAT_WIDTH);
UNUM_ICU_DECL(PADDING_POSITION);
UNUM_ICU_DECL(SECONDARY_GROUPING_SIZE);
UNUM_ICU_DECL(SIGNIFICANT_DIGITS_USED);
UNUM_ICU_DECL(MIN_SIGNIFICANT_DIGITS);
UNUM_ICU_DECL(MAX_SIGNIFICANT_DIGITS);
UNUM_ICU_DECL(LENIENT_PARSE);
UNUM_ICU_DECL(ROUNDING_INCREMENT);

/* UNumberFormatTextAttribute */
UNUM_ICU_DECL(POSITIVE_PREFIX);
UNUM_ICU_DECL(POSITIVE_SUFFIX);
UNUM_ICU_DECL(NEGATIVE_PREFIX);
UNUM_ICU_DECL(NEGATIVE_SUFFIX);
UNUM_ICU_DECL(PADDING_CHARACTER);
UNUM_ICU_DECL(CURRENCY_CODE);
UNUM_ICU_DECL(DEFAULT_RULESET);
UNUM_ICU_DECL(PUBLIC_RULESETS);

/* UNumberFormatSymbol */
UNUM_ICU_DECL(DECIMAL_SEPARATOR_SYMBOL);
UNUM_ICU_DECL(GROUPING_SEPARATOR_SYMBOL);
UNUM_ICU_DECL(PATTERN_SEPARATOR_SYMBOL);
UNUM_ICU_DECL(PERCENT_SYMBOL);
UNUM_ICU_DECL(ZERO_DIGIT_SYMBOL);
UNUM_ICU_DECL(DIGIT_SYMBOL);
UNUM_ICU_DECL(MINUS_SIGN_SYMBOL);
UNUM_ICU_DECL(PLUS_SIGN_SYMBOL);
UNUM_ICU_DECL(CURRENCY_SYMBOL);
UNUM_ICU_DECL(INTL_CURRENCY_SYMBOL);
UNUM_ICU_DECL(MONETARY_SEPARATOR_SYMBOL);
UNUM_ICU_DECL(EXPONENTIAL_SYMBOL);
UNUM_ICU_DECL(PERMILL_SYMBOL);
UNUM_ICU_DECL(PAD_ESCAPE_SYMBOL);
UNUM_ICU_DECL(INFINITY_SYMBOL);
UNUM_ICU_DECL(NAN_SYMBOL);
UNUM_ICU_DECL(SIGNIFICANT_DIGIT_SYMBOL);
UNUM_ICU_DECL(MONETARY_GROUPING_SEPARATOR_SYMBOL);

/* Format/Parse types */
UNUM_DECL(TYPE_DEFAULT, 0);
UNUM_DECL(TYPE_INT32, 1);
UNUM_DECL(TYPE_INT64, 2);
UNUM_DECL(TYPE_DOUBLE, 3);
UNUM_DECL(TYPE_CURRENCY, 4);

#undef UNUM_DECL
#undef UNUM_ICU_DECL

NumberFormatter::NumberFormatter(const String& locale,
                                 int64_t style,
                                 const String& pattern) {
  String pat;
  UErrorCode error = U_ZERO_ERROR;
  if (!pattern.empty()) {
    pat = u16(pattern, error);
    if (U_FAILURE(error)) {
      s_intl_error->set(error,
          "numfmt_create: error converting pattern to UTF-16");
      throwException("%s", s_intl_error->getErrorMessage().c_str());
    }
  }

  const String loc = locale.empty() ? GetDefaultLocale() : locale;

  error = U_ZERO_ERROR;
  m_formatter = unum_open((UNumberFormatStyle)style,
                          (UChar*)pat.c_str(), pat.size() / sizeof(UChar),
                          locale.c_str(),
                          nullptr, &error);
  if (U_FAILURE(error)) {
    s_intl_error->set(error,
        "numfmt_create: number formatter creation failed");
    throwException("%s", s_intl_error->getErrorMessage().c_str());
  }
}

NumberFormatter::NumberFormatter(const NumberFormatter *orig) {
  if (!orig || !orig->formatter()) {
    s_intl_error->set(U_ILLEGAL_ARGUMENT_ERROR,
                      "Cannot clone unconstructed NumberFormatter");
    throwException("%s", s_intl_error->getErrorMessage().c_str());
  }
  UErrorCode error = U_ZERO_ERROR;
  m_formatter = unum_clone(orig->formatter(), &error);
  if (U_FAILURE(error)) {
    s_intl_error->set(error, "numfmt_clone: number formatter clone failed");
    throwException("%s", s_intl_error->getErrorMessage().c_str());
  }
}

NumberFormatter *NumberFormatter::Get(Object obj) {
  return GetResData<NumberFormatter>(obj, s_NumberFormatter.get());
}

Object NumberFormatter::wrap() {
  return WrapResData(s_NumberFormatter.get());
}

#define NUMFMT_GET(dest, src, def) \
  auto dest = NumberFormatter::Get(src); \
  if (!dest) { \
    return def; \
  }

#define NUMFMT_CHECK(ov, ec, fail) \
  if (U_FAILURE(ec)) { \
    ov->setError(ec); \
    return fail; \
  }

//////////////////////////////////////////////////////////////////////////////
// class NumberFormatter

static void HHVM_METHOD(NumberFormatter, __construct,
                        const String& locale,
                        int64_t style,
                        const String& pattern) {
  auto data = NEWOBJ(NumberFormatter)(locale, style, pattern);
  this_->o_set(s_resdata, Resource(data), s_NumberFormatter.get());
}

static void HHVM_METHOD(NumberFormatter, __clone) {
  auto data = NEWOBJ(NumberFormatter)(NumberFormatter::Get(this_));
  this_->o_set(s_resdata, Resource(data), s_NumberFormatter.get());
}

static String HHVM_METHOD(NumberFormatter, formatCurrency,
                          double value,
                          const String& currency) {
  NUMFMT_GET(obj, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  String uCurrency = u16(currency, error);
  NUMFMT_CHECK(obj, error, null_string);
  uint32_t len = unum_formatDoubleCurrency(obj->formatter(), value,
                                           (UChar*)uCurrency.c_str(),
                                           nullptr, 0,
                                           nullptr, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return null_string;
  }
  String out(sizeof(UChar) * (len + 1), ReserveString);
  error = U_ZERO_ERROR;
  len = unum_formatDoubleCurrency(obj->formatter(), value,
                                  (UChar*)uCurrency.c_str(),
                                  (UChar*)out->mutableData(), len + 1,
                                  nullptr, &error);
  NUMFMT_CHECK(obj, error, null_string);
  out.setSize(len * sizeof(UChar));
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, null_string);
  return ret;
}

static Variant doFormat(NumberFormatter *obj, int64_t val) {
  UErrorCode error = U_ZERO_ERROR;
  uint32_t len = unum_formatInt64(obj->formatter(), val,
                                  nullptr, 0, nullptr, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return false;
  }
  error = U_ZERO_ERROR;
  String out(sizeof(UChar) * (len + 1), ReserveString);
  len = unum_formatInt64(obj->formatter(), val,
                         (UChar*)out->mutableData(), len + 1,
                         nullptr, &error);
  NUMFMT_CHECK(obj, error, false);
  out.setSize(len * sizeof(UChar));
  error = U_ZERO_ERROR;
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, false);
  return ret;
}

static Variant doFormat(NumberFormatter *obj, double val) {
  UErrorCode error = U_ZERO_ERROR;
  uint32_t len = unum_formatDouble(obj->formatter(), val,
                                   nullptr, 0,
                                   nullptr, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return false;
  }
  error = U_ZERO_ERROR;
  String out(sizeof(UChar) * (len + 1), ReserveString);
  len = unum_formatDouble(obj->formatter(), val,
                          (UChar*)out->mutableData(), len + 1,
                          nullptr, &error);
  NUMFMT_CHECK(obj, error, false);
  out.setSize(len * sizeof(UChar));
  error = U_ZERO_ERROR;
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, false);
  return ret;
}

static Variant HHVM_METHOD(NumberFormatter, format, CVarRef value,
                          int64_t type) {
  NUMFMT_GET(obj, this_, false);
  Variant num(value); // De-const
  if (!num.isDouble() && !num.isInteger()) {
    int64_t ival = 0;
    double dval = 0.0;
    DataType dt = num.toNumeric(ival, dval, true);
    if (dt == KindOfInt64) {
      num = ival;
    } else if (dt == KindOfDouble) {
      num = dval;
    } else {
      obj->setError(U_ILLEGAL_ARGUMENT_ERROR);
      return false;
    }
  }

  if (type == UNUM(TYPE_DEFAULT)) {
    if (value.isInteger()) {
      type = UNUM(TYPE_INT64);
    } else if (value.isDouble()) {
      type = UNUM(TYPE_DOUBLE);
    }
  }

  if ((type == UNUM(TYPE_INT32)) ||
      (type == UNUM(TYPE_INT64))) {
    return doFormat(obj, num.toInt64());
  } else if (type == UNUM(TYPE_DOUBLE)) {
    return doFormat(obj, num.toDouble());
  } else {
    raise_warning("Unsupported format type %ld", (long)type);
    return false;
  }
}

static Variant HHVM_METHOD(NumberFormatter, getAttribute, int64_t attr) {
  NUMFMT_GET(obj, this_, false);
  switch (attr) {
    case UNUM(PARSE_INT_ONLY):
    case UNUM(GROUPING_USED):
    case UNUM(DECIMAL_ALWAYS_SHOWN):
    case UNUM(MAX_INTEGER_DIGITS):
    case UNUM(MIN_INTEGER_DIGITS):
    case UNUM(INTEGER_DIGITS):
    case UNUM(MAX_FRACTION_DIGITS):
    case UNUM(MIN_FRACTION_DIGITS):
    case UNUM(FRACTION_DIGITS):
    case UNUM(MULTIPLIER):
    case UNUM(GROUPING_SIZE):
    case UNUM(ROUNDING_MODE):
    case UNUM(FORMAT_WIDTH):
    case UNUM(PADDING_POSITION):
    case UNUM(SECONDARY_GROUPING_SIZE):
    case UNUM(SIGNIFICANT_DIGITS_USED):
    case UNUM(MIN_SIGNIFICANT_DIGITS):
    case UNUM(MAX_SIGNIFICANT_DIGITS):
    case UNUM(LENIENT_PARSE): {
      int64_t lval = unum_getAttribute(obj->formatter(),
                                       (UNumberFormatAttribute)attr);
      if (lval == -1) {
        obj->setError(U_UNSUPPORTED_ERROR);
        return false;
      }
      return lval;
    }
    case UNUM(ROUNDING_INCREMENT): {
      double dval = unum_getDoubleAttribute(obj->formatter(),
                                            (UNumberFormatAttribute)attr);
      if (dval == -1) {
        obj->setError(U_UNSUPPORTED_ERROR);
        return false;
      }
      return dval;
    }
    default:
      obj->setError(U_UNSUPPORTED_ERROR);
      return false;
  }
}

static int64_t HHVM_METHOD(NumberFormatter, getErrorCode) {
  NUMFMT_GET(data, this_, 0);
  return data->getErrorCode();
}

static String HHVM_METHOD(NumberFormatter, getErrorMessage) {
  NUMFMT_GET(obj, this_, null_string);
  return obj->getErrorMessage();
}

static String HHVM_METHOD(NumberFormatter, getLocale, int64_t type) {
  NUMFMT_GET(obj, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  const char *loc = unum_getLocaleByType(obj->formatter(),
                                         (ULocDataLocaleType)type, &error);
  NUMFMT_CHECK(obj, error, null_string);
  return String(loc, CopyString);
}

static String HHVM_METHOD(NumberFormatter, getPattern) {
  NUMFMT_GET(obj, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = unum_toPattern(obj->formatter(), 0, nullptr, 0, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return null_string;
  }
  String out(sizeof(UChar) * (len + 1), ReserveString);
  error = U_ZERO_ERROR;
  len = unum_toPattern(obj->formatter(), 0,
                       (UChar*)out->mutableData(), len + 1, &error);
  NUMFMT_CHECK(obj, error, null_string);
  out.setSize(len * sizeof(UChar));
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, null_string);
  return ret;
}

static String HHVM_METHOD(NumberFormatter, getSymbol, int64_t attr) {
  NUMFMT_GET(obj, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = unum_getSymbol(obj->formatter(),
                               (UNumberFormatSymbol)attr,
                               nullptr, 0, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return null_string;
  }
  String out(sizeof(UChar) * (len + 1), ReserveString);
  error = U_ZERO_ERROR;
  len = unum_getSymbol(obj->formatter(),
                       (UNumberFormatSymbol)attr,
                       (UChar*)out->mutableData(), len + 1, &error);
  NUMFMT_CHECK(obj, error, null_string);
  out.setSize(len * sizeof(UChar));
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, null_string);
  return ret;
}

static String HHVM_METHOD(NumberFormatter, getTextAttribute, int64_t attr) {
  NUMFMT_GET(obj, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = unum_getTextAttribute(obj->formatter(),
                                      (UNumberFormatTextAttribute)attr,
                                      nullptr, 0, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return null_string;
  }
  String out(sizeof(UChar) * (len + 1), ReserveString);
  error = U_ZERO_ERROR;
  len = unum_getTextAttribute(obj->formatter(),
                              (UNumberFormatTextAttribute)attr,
                              (UChar*)out->mutableData(), len + 1, &error);
  NUMFMT_CHECK(obj, error, null_string);
  out.setSize(len * sizeof(UChar));
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, null_string);
  return ret;
}

static Variant HHVM_METHOD(NumberFormatter, parseCurrency,
                           const String& value, VRefParam currency,
                           VRefParam position) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  String val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  int32_t pos = position.toInt64();
  UChar cur[5] = {0};
  error = U_ZERO_ERROR;
  double parsed = unum_parseDoubleCurrency(obj->formatter(),
                        (UChar*)val.c_str(), val.size() / sizeof(UChar),
                        &pos, cur, &error);
  NUMFMT_CHECK(obj, error, false);
  position = (int64_t)pos;
  error = U_ZERO_ERROR;
  currency = u8(cur, u_strlen(cur), error);
  NUMFMT_CHECK(obj, error, false);
  return parsed;
}

static Variant HHVM_METHOD(NumberFormatter, parse,
                           const String& value, int64_t type,
                           VRefParam position) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  String val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  Variant ret;
  int32_t pos = position.toInt64();
  error = U_ZERO_ERROR;
  switch (type) {
    case UNUM(TYPE_INT32):
      ret = unum_parse(obj->formatter(),
                       (UChar*)val.c_str(), val.size() / sizeof(UChar),
                       &pos, &error);
      break;
    case UNUM(TYPE_INT64):
      ret = unum_parseInt64(obj->formatter(),
                            (UChar*)val.c_str(), val.size() / sizeof(UChar),
                            &pos, &error);
      break;
    case UNUM(TYPE_DOUBLE):
      ret = unum_parseDouble(obj->formatter(),
                             (UChar*)val.c_str(), val.size() / sizeof(UChar),
                             &pos, &error);
      break;
    default:
      obj->setError(U_UNSUPPORTED_ERROR);
      return false;
  }
  NUMFMT_CHECK(obj, error, false);
  position = pos;
  return ret;
}

static bool HHVM_METHOD(NumberFormatter, setAttribute,
                        int64_t attr, CVarRef value) {
  NUMFMT_GET(obj, this_, false);
  switch (attr) {
    case UNUM(PARSE_INT_ONLY):
    case UNUM(GROUPING_USED):
    case UNUM(DECIMAL_ALWAYS_SHOWN):
    case UNUM(MAX_INTEGER_DIGITS):
    case UNUM(MIN_INTEGER_DIGITS):
    case UNUM(INTEGER_DIGITS):
    case UNUM(MAX_FRACTION_DIGITS):
    case UNUM(MIN_FRACTION_DIGITS):
    case UNUM(FRACTION_DIGITS):
    case UNUM(MULTIPLIER):
    case UNUM(GROUPING_SIZE):
    case UNUM(ROUNDING_MODE):
    case UNUM(FORMAT_WIDTH):
    case UNUM(PADDING_POSITION):
    case UNUM(SECONDARY_GROUPING_SIZE):
    case UNUM(SIGNIFICANT_DIGITS_USED):
    case UNUM(MIN_SIGNIFICANT_DIGITS):
    case UNUM(MAX_SIGNIFICANT_DIGITS):
    case UNUM(LENIENT_PARSE):
      unum_setAttribute(obj->formatter(),
                        (UNumberFormatAttribute)attr, value.toInt64());
      return true;
    case UNUM(ROUNDING_INCREMENT):
      unum_setDoubleAttribute(obj->formatter(),
                              (UNumberFormatAttribute)attr, value.toDouble());
      return true;
    default:
      obj->setError(U_UNSUPPORTED_ERROR);
      return false;
  }
}

static bool HHVM_METHOD(NumberFormatter, setPattern, const String& pattern) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  String pat(u16(pattern, error));
  NUMFMT_CHECK(obj, error, false);
  error = U_ZERO_ERROR;
  unum_applyPattern(obj->formatter(), 0,
                    (UChar*)pat.c_str(), pat.size() / sizeof(UChar),
                    nullptr, &error);
  NUMFMT_CHECK(obj, error, false);
  return true;
}

static bool HHVM_METHOD(NumberFormatter, setSymbol,
                        int64_t attr, const String& value) {
  NUMFMT_GET(obj, this_, false);
  if (attr >= UNUM_FORMAT_SYMBOL_COUNT || attr < 0) {
    obj->setError(U_ILLEGAL_ARGUMENT_ERROR,
                  "numfmt_set_symbol: invalid symbol value");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  String val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  error = U_ZERO_ERROR;
  unum_setSymbol(obj->formatter(), (UNumberFormatSymbol)attr,
                 (UChar*)val.c_str(), val.size() / sizeof(UChar), &error);
  NUMFMT_CHECK(obj, error, false);
  return true;
}

static bool HHVM_METHOD(NumberFormatter, setTextAttribute,
                        int64_t attr, const String& value) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  String val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  unum_setTextAttribute(obj->formatter(), (UNumberFormatTextAttribute)attr,
                        (UChar*)val.c_str(), val.size() / sizeof(UChar),
                        &error);
  NUMFMT_CHECK(obj, error, false);
  return true;
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initNumberFormatter() {
  HHVM_ME(NumberFormatter, __construct);
  HHVM_ME(NumberFormatter, __clone);
  HHVM_ME(NumberFormatter, formatCurrency);
  HHVM_ME(NumberFormatter, format);
  HHVM_ME(NumberFormatter, getAttribute);
  HHVM_ME(NumberFormatter, getErrorCode);
  HHVM_ME(NumberFormatter, getErrorMessage);
  HHVM_ME(NumberFormatter, getLocale);
  HHVM_ME(NumberFormatter, getPattern);
  HHVM_ME(NumberFormatter, getSymbol);
  HHVM_ME(NumberFormatter, getTextAttribute);
  HHVM_ME(NumberFormatter, parseCurrency);
  HHVM_ME(NumberFormatter, parse);
  HHVM_ME(NumberFormatter, setAttribute);
  HHVM_ME(NumberFormatter, setPattern);
  HHVM_ME(NumberFormatter, setSymbol);
  HHVM_ME(NumberFormatter, setTextAttribute);

#define NUMFMT_CONST(n) \
  Native::registerClassConstant<KindOfInt64>(s_NumberFormatter.get(), \
                                             s_NumberFormatter_ ## n .get(), \
                                             q_NumberFormatter_ ## n)

  NUMFMT_CONST(PATTERN_DECIMAL);
  NUMFMT_CONST(DECIMAL);
  NUMFMT_CONST(CURRENCY);
  NUMFMT_CONST(PERCENT);
  NUMFMT_CONST(SCIENTIFIC);
  NUMFMT_CONST(SPELLOUT);
  NUMFMT_CONST(ORDINAL);
  NUMFMT_CONST(DURATION);
  NUMFMT_CONST(PATTERN_RULEBASED);
  NUMFMT_CONST(IGNORE);
  NUMFMT_CONST(DEFAULT_STYLE);
  NUMFMT_CONST(ROUND_CEILING);
  NUMFMT_CONST(ROUND_FLOOR);
  NUMFMT_CONST(ROUND_DOWN);
  NUMFMT_CONST(ROUND_UP);
  NUMFMT_CONST(ROUND_HALFEVEN);
  NUMFMT_CONST(ROUND_HALFDOWN);
  NUMFMT_CONST(ROUND_HALFUP);
  NUMFMT_CONST(PAD_BEFORE_PREFIX);
  NUMFMT_CONST(PAD_AFTER_PREFIX);
  NUMFMT_CONST(PAD_BEFORE_SUFFIX);
  NUMFMT_CONST(PAD_AFTER_SUFFIX);
  NUMFMT_CONST(PARSE_INT_ONLY);
  NUMFMT_CONST(GROUPING_USED);
  NUMFMT_CONST(DECIMAL_ALWAYS_SHOWN);
  NUMFMT_CONST(MAX_INTEGER_DIGITS);
  NUMFMT_CONST(MIN_INTEGER_DIGITS);
  NUMFMT_CONST(INTEGER_DIGITS);
  NUMFMT_CONST(MAX_FRACTION_DIGITS);
  NUMFMT_CONST(MIN_FRACTION_DIGITS);
  NUMFMT_CONST(FRACTION_DIGITS);
  NUMFMT_CONST(MULTIPLIER);
  NUMFMT_CONST(GROUPING_SIZE);
  NUMFMT_CONST(ROUNDING_MODE);
  NUMFMT_CONST(ROUNDING_INCREMENT);
  NUMFMT_CONST(FORMAT_WIDTH);
  NUMFMT_CONST(PADDING_POSITION);
  NUMFMT_CONST(SECONDARY_GROUPING_SIZE);
  NUMFMT_CONST(SIGNIFICANT_DIGITS_USED);
  NUMFMT_CONST(MIN_SIGNIFICANT_DIGITS);
  NUMFMT_CONST(MAX_SIGNIFICANT_DIGITS);
  NUMFMT_CONST(LENIENT_PARSE);
  NUMFMT_CONST(POSITIVE_PREFIX);
  NUMFMT_CONST(POSITIVE_SUFFIX);
  NUMFMT_CONST(NEGATIVE_PREFIX);
  NUMFMT_CONST(NEGATIVE_SUFFIX);
  NUMFMT_CONST(PADDING_CHARACTER);
  NUMFMT_CONST(CURRENCY_CODE);
  NUMFMT_CONST(DEFAULT_RULESET);
  NUMFMT_CONST(PUBLIC_RULESETS);
  NUMFMT_CONST(DECIMAL_SEPARATOR_SYMBOL);
  NUMFMT_CONST(GROUPING_SEPARATOR_SYMBOL);
  NUMFMT_CONST(PATTERN_SEPARATOR_SYMBOL);
  NUMFMT_CONST(PERCENT_SYMBOL);
  NUMFMT_CONST(ZERO_DIGIT_SYMBOL);
  NUMFMT_CONST(DIGIT_SYMBOL);
  NUMFMT_CONST(MINUS_SIGN_SYMBOL);
  NUMFMT_CONST(PLUS_SIGN_SYMBOL);
  NUMFMT_CONST(CURRENCY_SYMBOL);
  NUMFMT_CONST(INTL_CURRENCY_SYMBOL);
  NUMFMT_CONST(MONETARY_SEPARATOR_SYMBOL);
  NUMFMT_CONST(EXPONENTIAL_SYMBOL);
  NUMFMT_CONST(PERMILL_SYMBOL);
  NUMFMT_CONST(PAD_ESCAPE_SYMBOL);
  NUMFMT_CONST(INFINITY_SYMBOL);
  NUMFMT_CONST(NAN_SYMBOL);
  NUMFMT_CONST(SIGNIFICANT_DIGIT_SYMBOL);
  NUMFMT_CONST(MONETARY_GROUPING_SEPARATOR_SYMBOL);
  NUMFMT_CONST(TYPE_DEFAULT);
  NUMFMT_CONST(TYPE_INT32);
  NUMFMT_CONST(TYPE_INT64);
  NUMFMT_CONST(TYPE_DOUBLE);
  NUMFMT_CONST(TYPE_CURRENCY);
#undef NUMFMT_CONST

  loadSystemlib("icu_num_fmt");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
