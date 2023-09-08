/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/string/ext_string.h"

namespace HPHP::Intl {
//////////////////////////////////////////////////////////////////////////////
// Internal resource data

const StaticString s_NumberFormatter("NumberFormatter");

/* workaround for ICU bug */
#if U_ICU_VERSION_MAJOR_NUM == 3 && U_ICU_VERSION_MINOR_NUM < 8
#define UNUM_ROUND_HALFEVEN UNUM_FOUND_HALFEVEN
#endif

/* Format/Parse types */
static const int64_t k_UNUM_TYPE_DEFAULT = 0;
static const int64_t k_UNUM_TYPE_INT32 = 1;
static const int64_t k_UNUM_TYPE_INT64 = 2;
static const int64_t k_UNUM_TYPE_DOUBLE = 3;
static const int64_t k_UNUM_TYPE_CURRENCY = 4;

void NumberFormatter::setNumberFormatter(const String& locale,
                                         int64_t style,
                                         const String& pattern) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString pat(u16(pattern, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error,
        "numfmt_create: error converting pattern to UTF-16");
    throwException("%s", s_intl_error->getErrorMessage().c_str());
  }

  const String loc(localeOrDefault(locale));

  error = U_ZERO_ERROR;
  m_formatter = unum_open((UNumberFormatStyle)style,
                          pat.getBuffer(), pat.length(),
                          locale.c_str(),
                          nullptr, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error,
        "numfmt_create: number formatter creation failed");
    throwException("%s", s_intl_error->getErrorMessage().c_str());
  }
}

void NumberFormatter::setNumberFormatter(const NumberFormatter *orig) {
  if (!orig || !orig->formatter()) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "Cannot clone unconstructed NumberFormatter");
    throwException("%s", s_intl_error->getErrorMessage(false).c_str());
  }
  UErrorCode error = U_ZERO_ERROR;
  m_formatter = unum_clone(orig->formatter(), &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "numfmt_clone: "
                                  "number formatter clone failed");
    throwException("%s", s_intl_error->getErrorMessage().c_str());
  }
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
  Native::data<NumberFormatter>(this_)->
    setNumberFormatter(locale, style, pattern);
}

static String HHVM_METHOD(NumberFormatter, formatCurrency,
                          double value,
                          const String& currency) {
  NUMFMT_GET(obj, this_, String());
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uCurrency(u16(currency, error));
  NUMFMT_CHECK(obj, error, String());

  // By default UnicodeString isn't NULL terminated
  int32_t currencyBuffer_len = uCurrency.length();
  UChar *currencyBuffer = uCurrency.getBuffer(currencyBuffer_len + 1);
  SCOPE_EXIT{ uCurrency.releaseBuffer(currencyBuffer_len + 1); };
  currencyBuffer[currencyBuffer_len] = 0;

  error = U_ZERO_ERROR;
  uint32_t len =
    unum_formatDoubleCurrency(obj->formatter(), value,
                              currencyBuffer,
                              nullptr, 0,
                              nullptr, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return String();
  }
  icu::UnicodeString out;
  error = U_ZERO_ERROR;
  len = unum_formatDoubleCurrency(obj->formatter(), value,
                                  currencyBuffer,
                                  out.getBuffer(len + 1), len + 1,
                                  nullptr, &error);
  NUMFMT_CHECK(obj, error, String());
  out.releaseBuffer(len);
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, String());
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
  icu::UnicodeString out;
  len = unum_formatInt64(obj->formatter(), val,
                         out.getBuffer(len + 1), len + 1,
                         nullptr, &error);
  NUMFMT_CHECK(obj, error, false);
  out.releaseBuffer(len);
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
  icu::UnicodeString out;
  len = unum_formatDouble(obj->formatter(), val,
                          out.getBuffer(len + 1), len + 1,
                          nullptr, &error);
  NUMFMT_CHECK(obj, error, false);
  out.releaseBuffer(len);
  error = U_ZERO_ERROR;
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, false);
  return ret;
}

static Variant HHVM_METHOD(NumberFormatter, format, const Variant& value,
                          int64_t type) {
  NUMFMT_GET(obj, this_, false);
  Variant num(value); // De-const

  int64_t ival = 0;
  double dval = 0.0;
  DataType dt = num.toNumeric(ival, dval, true);
  if (dt == KindOfInt64) {
    num = ival;
  } else if (dt == KindOfDouble) {
    num = dval;
  } else {
    num = value.toInt64();
  }

  if (type == k_UNUM_TYPE_DEFAULT) {
    if (num.isInteger()) {
      type = k_UNUM_TYPE_INT64;
    } else if (num.isDouble()) {
      type = k_UNUM_TYPE_DOUBLE;
    }
  }

  if (type == k_UNUM_TYPE_DOUBLE) {
    return doFormat(obj, num.toDouble());
  } else {
    return doFormat(obj, num.toInt64());
  }
}

static Variant HHVM_METHOD(NumberFormatter, getAttribute, int64_t attr) {
  NUMFMT_GET(obj, this_, false);
  switch (attr) {
    case UNUM_PARSE_INT_ONLY:
    case UNUM_GROUPING_USED:
    case UNUM_DECIMAL_ALWAYS_SHOWN:
    case UNUM_MAX_INTEGER_DIGITS:
    case UNUM_MIN_INTEGER_DIGITS:
    case UNUM_INTEGER_DIGITS:
    case UNUM_MAX_FRACTION_DIGITS:
    case UNUM_MIN_FRACTION_DIGITS:
    case UNUM_FRACTION_DIGITS:
    case UNUM_MULTIPLIER:
    case UNUM_GROUPING_SIZE:
    case UNUM_ROUNDING_MODE:
    case UNUM_FORMAT_WIDTH:
    case UNUM_PADDING_POSITION:
    case UNUM_SECONDARY_GROUPING_SIZE:
    case UNUM_SIGNIFICANT_DIGITS_USED:
    case UNUM_MIN_SIGNIFICANT_DIGITS:
    case UNUM_MAX_SIGNIFICANT_DIGITS:
    case UNUM_LENIENT_PARSE: {
      int64_t lval = unum_getAttribute(obj->formatter(),
                                       (UNumberFormatAttribute)attr);
      if (lval == -1) {
        obj->setError(U_UNSUPPORTED_ERROR);
        return false;
      }
      return lval;
    }
    case UNUM_ROUNDING_INCREMENT: {
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
  NUMFMT_GET(obj, this_, String());
  return obj->getErrorMessage();
}

static String HHVM_METHOD(NumberFormatter, getLocale, int64_t type) {
  NUMFMT_GET(obj, this_, String());
  UErrorCode error = U_ZERO_ERROR;
  const char *loc = unum_getLocaleByType(obj->formatter(),
                                         (ULocDataLocaleType)type, &error);
  NUMFMT_CHECK(obj, error, String());
  return String(loc, CopyString);
}

static String HHVM_METHOD(NumberFormatter, getPattern) {
  NUMFMT_GET(obj, this_, String());
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = unum_toPattern(obj->formatter(), 0, nullptr, 0, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return String();
  }
  icu::UnicodeString out;
  error = U_ZERO_ERROR;
  len = unum_toPattern(obj->formatter(), 0,
                       out.getBuffer(len + 1), len + 1, &error);
  NUMFMT_CHECK(obj, error, String());
  out.releaseBuffer(len);
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, String());
  return ret;
}

static String HHVM_METHOD(NumberFormatter, getSymbol, int64_t attr) {
  NUMFMT_GET(obj, this_, String());
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = unum_getSymbol(obj->formatter(),
                               (UNumberFormatSymbol)attr,
                               nullptr, 0, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return String();
  }
  icu::UnicodeString out;
  error = U_ZERO_ERROR;
  len = unum_getSymbol(obj->formatter(), (UNumberFormatSymbol)attr,
                       out.getBuffer(len + 1), len + 1, &error);
  NUMFMT_CHECK(obj, error, String());
  out.releaseBuffer(len);
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, String());
  return ret;
}

static String HHVM_METHOD(NumberFormatter, getTextAttribute, int64_t attr) {
  NUMFMT_GET(obj, this_, String());
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = unum_getTextAttribute(obj->formatter(),
                                      (UNumberFormatTextAttribute)attr,
                                      nullptr, 0, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    obj->setError(error);
    return String();
  }
  icu::UnicodeString out;
  error = U_ZERO_ERROR;
  len = unum_getTextAttribute(obj->formatter(),
                              (UNumberFormatTextAttribute)attr,
                              out.getBuffer(len + 1), len + 1, &error);
  NUMFMT_CHECK(obj, error, String());
  out.releaseBuffer(len);
  String ret(u8(out, error));
  NUMFMT_CHECK(obj, error, String());
  return ret;
}

static Variant HHVM_METHOD(NumberFormatter, parseCurrency,
                           const String& value, String& currency,
                           Variant& position) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  int32_t pos = position.toInt64();
  UChar cur[5] = {0};
  error = U_ZERO_ERROR;
  double parsed = unum_parseDoubleCurrency(obj->formatter(),
                        val.getBuffer(), val.length(),
                        &pos, cur, &error);
  NUMFMT_CHECK(obj, error, false);
  position = (int64_t)pos;
  error = U_ZERO_ERROR;
  currency = u8(cur, u_strlen(cur), error);
  NUMFMT_CHECK(obj, error, false);
  return parsed;
}

static Variant HHVM_METHOD(NumberFormatter, parseWithPosition,
                           const String& value, int64_t type,
                           Variant& position) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  Variant ret;
  int32_t pos = position.toInt64();
  error = U_ZERO_ERROR;
  switch (type) {
    case k_UNUM_TYPE_INT32:
      ret = unum_parse(obj->formatter(), val.getBuffer(), val.length(),
                       &pos, &error);
      break;
    case k_UNUM_TYPE_INT64:
      ret = unum_parseInt64(obj->formatter(), val.getBuffer(), val.length(),
                            &pos, &error);
      break;
    case k_UNUM_TYPE_DOUBLE:
      ret = unum_parseDouble(obj->formatter(), val.getBuffer(), val.length(),
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
                        int64_t attr, const Variant& value) {
  NUMFMT_GET(obj, this_, false);
  switch (attr) {
    case UNUM_PARSE_INT_ONLY:
    case UNUM_GROUPING_USED:
    case UNUM_DECIMAL_ALWAYS_SHOWN:
    case UNUM_MAX_INTEGER_DIGITS:
    case UNUM_MIN_INTEGER_DIGITS:
    case UNUM_INTEGER_DIGITS:
    case UNUM_MAX_FRACTION_DIGITS:
    case UNUM_MIN_FRACTION_DIGITS:
    case UNUM_FRACTION_DIGITS:
    case UNUM_MULTIPLIER:
    case UNUM_GROUPING_SIZE:
    case UNUM_ROUNDING_MODE:
    case UNUM_FORMAT_WIDTH:
    case UNUM_PADDING_POSITION:
    case UNUM_SECONDARY_GROUPING_SIZE:
    case UNUM_SIGNIFICANT_DIGITS_USED:
    case UNUM_MIN_SIGNIFICANT_DIGITS:
    case UNUM_MAX_SIGNIFICANT_DIGITS:
    case UNUM_LENIENT_PARSE:
      unum_setAttribute(obj->formatter(),
                        (UNumberFormatAttribute)attr, value.toInt64());
      return true;
    case UNUM_ROUNDING_INCREMENT:
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
  icu::UnicodeString pat(u16(pattern, error));
  NUMFMT_CHECK(obj, error, false);
  error = U_ZERO_ERROR;
  unum_applyPattern(obj->formatter(), 0, pat.getBuffer(), pat.length(),
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
  icu::UnicodeString val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  error = U_ZERO_ERROR;
  unum_setSymbol(obj->formatter(), (UNumberFormatSymbol)attr,
                 val.getBuffer(), val.length(), &error);
  NUMFMT_CHECK(obj, error, false);
  return true;
}

static bool HHVM_METHOD(NumberFormatter, setTextAttribute,
                        int64_t attr, const String& value) {
  NUMFMT_GET(obj, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString val(u16(value, error));
  NUMFMT_CHECK(obj, error, false);
  unum_setTextAttribute(obj->formatter(), (UNumberFormatTextAttribute)attr,
                        val.getBuffer(), val.length(), &error);
  NUMFMT_CHECK(obj, error, false);
  return true;
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initNumberFormatter() {
  HHVM_ME(NumberFormatter, __construct);
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
  HHVM_ME(NumberFormatter, parseWithPosition);
  HHVM_ME(NumberFormatter, setAttribute);
  HHVM_ME(NumberFormatter, setPattern);
  HHVM_ME(NumberFormatter, setSymbol);
  HHVM_ME(NumberFormatter, setTextAttribute);

  /* UNumberFormatStyle constants */
  HHVM_RCC_INT(NumberFormatter, PATTERN_DECIMAL, UNUM_PATTERN_DECIMAL);
  HHVM_RCC_INT(NumberFormatter, DECIMAL, UNUM_DECIMAL);
  HHVM_RCC_INT(NumberFormatter, CURRENCY, UNUM_CURRENCY);
  HHVM_RCC_INT(NumberFormatter, PERCENT, UNUM_PERCENT);
  HHVM_RCC_INT(NumberFormatter, SCIENTIFIC, UNUM_SCIENTIFIC);
  HHVM_RCC_INT(NumberFormatter, SPELLOUT, UNUM_SPELLOUT);
  HHVM_RCC_INT(NumberFormatter, ORDINAL, UNUM_ORDINAL);
  HHVM_RCC_INT(NumberFormatter, DURATION, UNUM_DURATION);
  HHVM_RCC_INT(NumberFormatter, PATTERN_RULEBASED, UNUM_PATTERN_RULEBASED);
  HHVM_RCC_INT(NumberFormatter, IGNORE, UNUM_IGNORE);
  HHVM_RCC_INT(NumberFormatter, DEFAULT_STYLE, UNUM_DEFAULT);

  /* UNumberFormatRoundingMode */
  HHVM_RCC_INT(NumberFormatter, ROUND_CEILING, UNUM_ROUND_CEILING);
  HHVM_RCC_INT(NumberFormatter, ROUND_FLOOR, UNUM_ROUND_FLOOR);
  HHVM_RCC_INT(NumberFormatter, ROUND_DOWN, UNUM_ROUND_DOWN);
  HHVM_RCC_INT(NumberFormatter, ROUND_UP, UNUM_ROUND_UP);
  HHVM_RCC_INT(NumberFormatter, ROUND_HALFEVEN, UNUM_ROUND_HALFEVEN);
  HHVM_RCC_INT(NumberFormatter, ROUND_HALFDOWN, UNUM_ROUND_HALFDOWN);
  HHVM_RCC_INT(NumberFormatter, ROUND_HALFUP, UNUM_ROUND_HALFUP);

  /* UNumberFormatPadPosition */
  HHVM_RCC_INT(NumberFormatter, PAD_BEFORE_PREFIX, UNUM_PAD_BEFORE_PREFIX);
  HHVM_RCC_INT(NumberFormatter, PAD_AFTER_PREFIX, UNUM_PAD_AFTER_PREFIX);
  HHVM_RCC_INT(NumberFormatter, PAD_BEFORE_SUFFIX, UNUM_PAD_BEFORE_SUFFIX);
  HHVM_RCC_INT(NumberFormatter, PAD_AFTER_SUFFIX, UNUM_PAD_AFTER_SUFFIX);

  /* UNumberFormatAttribute */
  HHVM_RCC_INT(NumberFormatter, PARSE_INT_ONLY, UNUM_PARSE_INT_ONLY);
  HHVM_RCC_INT(NumberFormatter, GROUPING_USED, UNUM_GROUPING_USED);
  HHVM_RCC_INT(NumberFormatter, DECIMAL_ALWAYS_SHOWN,
               UNUM_DECIMAL_ALWAYS_SHOWN);
  HHVM_RCC_INT(NumberFormatter, MAX_INTEGER_DIGITS, UNUM_MAX_INTEGER_DIGITS);
  HHVM_RCC_INT(NumberFormatter, MIN_INTEGER_DIGITS, UNUM_MIN_INTEGER_DIGITS);
  HHVM_RCC_INT(NumberFormatter, INTEGER_DIGITS, UNUM_INTEGER_DIGITS);
  HHVM_RCC_INT(NumberFormatter, MAX_FRACTION_DIGITS, UNUM_MAX_FRACTION_DIGITS);
  HHVM_RCC_INT(NumberFormatter, MIN_FRACTION_DIGITS, UNUM_MIN_FRACTION_DIGITS);
  HHVM_RCC_INT(NumberFormatter, FRACTION_DIGITS, UNUM_FRACTION_DIGITS);
  HHVM_RCC_INT(NumberFormatter, MULTIPLIER, UNUM_MULTIPLIER);
  HHVM_RCC_INT(NumberFormatter, GROUPING_SIZE, UNUM_GROUPING_SIZE);
  HHVM_RCC_INT(NumberFormatter, ROUNDING_MODE, UNUM_ROUNDING_MODE);
  HHVM_RCC_INT(NumberFormatter, FORMAT_WIDTH, UNUM_FORMAT_WIDTH);
  HHVM_RCC_INT(NumberFormatter, PADDING_POSITION, UNUM_PADDING_POSITION);
  HHVM_RCC_INT(NumberFormatter, SECONDARY_GROUPING_SIZE,
               UNUM_SECONDARY_GROUPING_SIZE);
  HHVM_RCC_INT(NumberFormatter, SIGNIFICANT_DIGITS_USED,
               UNUM_SIGNIFICANT_DIGITS_USED);
  HHVM_RCC_INT(NumberFormatter, MIN_SIGNIFICANT_DIGITS,
               UNUM_MIN_SIGNIFICANT_DIGITS);
  HHVM_RCC_INT(NumberFormatter, MAX_SIGNIFICANT_DIGITS
               , UNUM_MAX_SIGNIFICANT_DIGITS);
  HHVM_RCC_INT(NumberFormatter, LENIENT_PARSE, UNUM_LENIENT_PARSE);
  HHVM_RCC_INT(NumberFormatter, ROUNDING_INCREMENT, UNUM_ROUNDING_INCREMENT);

  /* UNumberFormatTextAttribute */
  HHVM_RCC_INT(NumberFormatter, POSITIVE_PREFIX, UNUM_POSITIVE_PREFIX);
  HHVM_RCC_INT(NumberFormatter, POSITIVE_SUFFIX, UNUM_POSITIVE_SUFFIX);
  HHVM_RCC_INT(NumberFormatter, NEGATIVE_PREFIX, UNUM_NEGATIVE_PREFIX);
  HHVM_RCC_INT(NumberFormatter, NEGATIVE_SUFFIX, UNUM_NEGATIVE_SUFFIX);
  HHVM_RCC_INT(NumberFormatter, PADDING_CHARACTER, UNUM_PADDING_CHARACTER);
  HHVM_RCC_INT(NumberFormatter, CURRENCY_CODE, UNUM_CURRENCY_CODE);
  HHVM_RCC_INT(NumberFormatter, DEFAULT_RULESET, UNUM_DEFAULT_RULESET);
  HHVM_RCC_INT(NumberFormatter, PUBLIC_RULESETS, UNUM_PUBLIC_RULESETS);

  /* UNumberFormatSymbol */
  HHVM_RCC_INT(NumberFormatter, DECIMAL_SEPARATOR_SYMBOL,
               UNUM_DECIMAL_SEPARATOR_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, GROUPING_SEPARATOR_SYMBOL,
               UNUM_GROUPING_SEPARATOR_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, PATTERN_SEPARATOR_SYMBOL,
               UNUM_PATTERN_SEPARATOR_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, PERCENT_SYMBOL, UNUM_PERCENT_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, ZERO_DIGIT_SYMBOL, UNUM_ZERO_DIGIT_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, DIGIT_SYMBOL, UNUM_DIGIT_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, MINUS_SIGN_SYMBOL, UNUM_MINUS_SIGN_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, PLUS_SIGN_SYMBOL, UNUM_PLUS_SIGN_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, CURRENCY_SYMBOL, UNUM_CURRENCY_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, INTL_CURRENCY_SYMBOL,
               UNUM_INTL_CURRENCY_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, MONETARY_SEPARATOR_SYMBOL,
               UNUM_MONETARY_SEPARATOR_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, EXPONENTIAL_SYMBOL, UNUM_EXPONENTIAL_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, PERMILL_SYMBOL, UNUM_PERMILL_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, PAD_ESCAPE_SYMBOL, UNUM_PAD_ESCAPE_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, INFINITY_SYMBOL, UNUM_INFINITY_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, NAN_SYMBOL, UNUM_NAN_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, SIGNIFICANT_DIGIT_SYMBOL,
               UNUM_SIGNIFICANT_DIGIT_SYMBOL);
  HHVM_RCC_INT(NumberFormatter, MONETARY_GROUPING_SEPARATOR_SYMBOL,
               UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL);

  /* Format/Parse types */
  HHVM_RCC_INT(NumberFormatter, TYPE_DEFAULT, k_UNUM_TYPE_DEFAULT);
  HHVM_RCC_INT(NumberFormatter, TYPE_INT32, k_UNUM_TYPE_INT32);
  HHVM_RCC_INT(NumberFormatter, TYPE_INT64, k_UNUM_TYPE_INT64);
  HHVM_RCC_INT(NumberFormatter, TYPE_DOUBLE, k_UNUM_TYPE_DOUBLE);
  HHVM_RCC_INT(NumberFormatter, TYPE_CURRENCY, k_UNUM_TYPE_CURRENCY);

  Native::registerNativeDataInfo<NumberFormatter>();
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
