/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/util/string-vsnprintf.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include <unicode/uloc.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_REQUEST_LOCAL(IntlGlobalError, s_intl_error);

namespace Intl {

void IntlError::setError(UErrorCode code, const char *format, ...) {
  m_errorCode = code;
  if (format) {
    va_list args;
    va_start(args, format);
    string_vsnprintf(m_errorMessage, format, args);
    va_end(args);
  } else {
    m_errorMessage.clear();
  }

  if (this != s_intl_error.get()) {
    s_intl_error->m_errorCode = m_errorCode;
    s_intl_error->m_errorMessage = m_errorMessage;
  }
}

void IntlError::clearError(bool clearGlobalError /*= true */) {
  m_errorCode = U_ZERO_ERROR;
  m_errorMessage.clear();

  if (clearGlobalError && (this != s_intl_error.get())) {
    s_intl_error->m_errorCode = U_ZERO_ERROR;
    s_intl_error->m_errorMessage.clear();
  }
}

/////////////////////////////////////////////////////////////////////////////
// INI Setting

static __thread std::string* s_defaultLocale;

void IntlExtension::bindIniSettings() {
  // TODO: t5226715 We shouldn't need to check s_defaultLocale here,
  // but right now this is called for every request.
  if (s_defaultLocale) return;
  s_defaultLocale = new std::string;
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   "intl.default_locale", "",
                   s_defaultLocale);
}

void IntlExtension::threadShutdown() {
  delete s_defaultLocale;
  s_defaultLocale = nullptr;
}

const String GetDefaultLocale() {
  assert(s_defaultLocale);
  if (s_defaultLocale->empty()) {
    return String(uloc_getDefault(), CopyString);
  }
  return *s_defaultLocale;
}

bool SetDefaultLocale(const String& locale) {
  assert(s_defaultLocale);
  *s_defaultLocale = locale.toCppString();
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// Common extension init

const StaticString
#ifdef U_ICU_DATA_VERSION
  s_INTL_ICU_DATA_VERSION("INTL_ICU_DATA_VERSION"),
  s_U_ICU_DATA_VERSION(U_ICU_DATA_VERSION),
#endif
  s_INTL_ICU_VERSION("INTL_ICU_VERSION"),
  s_U_ICU_VERSION(U_ICU_VERSION);

void IntlExtension::bindConstants() {
#ifdef U_ICU_DATA_VERSION
  Native::registerConstant<KindOfString>(s_INTL_ICU_DATA_VERSION.get(),
                                         s_U_ICU_DATA_VERSION.get());
#endif
  Native::registerConstant<KindOfString>(s_INTL_ICU_VERSION.get(),
                                         s_U_ICU_VERSION.get());

  // UErrorCode constants
#define UERRCODE(cns) \
  Native::registerConstant<KindOfInt64>(makeStaticString(#cns), cns)
  UERRCODE(U_AMBIGUOUS_ALIAS_WARNING);
  UERRCODE(U_BAD_VARIABLE_DEFINITION);
  UERRCODE(U_BRK_ASSIGN_ERROR);
  UERRCODE(U_BRK_ERROR_LIMIT);
  UERRCODE(U_BRK_ERROR_START);
  UERRCODE(U_BRK_HEX_DIGITS_EXPECTED);
  UERRCODE(U_BRK_INIT_ERROR);
  UERRCODE(U_BRK_INTERNAL_ERROR);
  UERRCODE(U_BRK_MALFORMED_RULE_TAG);
  UERRCODE(U_BRK_MISMATCHED_PAREN);
  UERRCODE(U_BRK_NEW_LINE_IN_QUOTED_STRING);
  UERRCODE(U_BRK_RULE_EMPTY_SET);
  UERRCODE(U_BRK_RULE_SYNTAX);
  UERRCODE(U_BRK_SEMICOLON_EXPECTED);
  UERRCODE(U_BRK_UNCLOSED_SET);
  UERRCODE(U_BRK_UNDEFINED_VARIABLE);
  UERRCODE(U_BRK_UNRECOGNIZED_OPTION);
  UERRCODE(U_BRK_VARIABLE_REDFINITION);
  UERRCODE(U_BUFFER_OVERFLOW_ERROR);
  UERRCODE(U_CE_NOT_FOUND_ERROR);
  UERRCODE(U_COLLATOR_VERSION_MISMATCH);
  UERRCODE(U_DIFFERENT_UCA_VERSION);
  UERRCODE(U_ENUM_OUT_OF_SYNC_ERROR);
  UERRCODE(U_ERROR_LIMIT);
  UERRCODE(U_ERROR_WARNING_LIMIT);
  UERRCODE(U_ERROR_WARNING_START);
  UERRCODE(U_FILE_ACCESS_ERROR);
  UERRCODE(U_FMT_PARSE_ERROR_LIMIT);
  UERRCODE(U_FMT_PARSE_ERROR_START);
  UERRCODE(U_ILLEGAL_ARGUMENT_ERROR);
  UERRCODE(U_ILLEGAL_CHARACTER);
  UERRCODE(U_ILLEGAL_CHAR_FOUND);
  UERRCODE(U_ILLEGAL_CHAR_IN_SEGMENT);
  UERRCODE(U_ILLEGAL_ESCAPE_SEQUENCE);
  UERRCODE(U_ILLEGAL_PAD_POSITION);
  UERRCODE(U_INDEX_OUTOFBOUNDS_ERROR);
  UERRCODE(U_INTERNAL_PROGRAM_ERROR);
  UERRCODE(U_INTERNAL_TRANSLITERATOR_ERROR);
  UERRCODE(U_INVALID_CHAR_FOUND);
  UERRCODE(U_INVALID_FORMAT_ERROR);
  UERRCODE(U_INVALID_FUNCTION);
  UERRCODE(U_INVALID_ID);
  UERRCODE(U_INVALID_PROPERTY_PATTERN);
  UERRCODE(U_INVALID_RBT_SYNTAX);
  UERRCODE(U_INVALID_STATE_ERROR);
  UERRCODE(U_INVALID_TABLE_FILE);
  UERRCODE(U_INVALID_TABLE_FORMAT);
  UERRCODE(U_INVARIANT_CONVERSION_ERROR);
  UERRCODE(U_MALFORMED_EXPONENTIAL_PATTERN);
  UERRCODE(U_MALFORMED_PRAGMA);
  UERRCODE(U_MALFORMED_RULE);
  UERRCODE(U_MALFORMED_SET);
  UERRCODE(U_MALFORMED_SYMBOL_REFERENCE);
  UERRCODE(U_MALFORMED_UNICODE_ESCAPE);
  UERRCODE(U_MALFORMED_VARIABLE_DEFINITION);
  UERRCODE(U_MALFORMED_VARIABLE_REFERENCE);
  UERRCODE(U_MEMORY_ALLOCATION_ERROR);
  UERRCODE(U_MESSAGE_PARSE_ERROR);
  UERRCODE(U_MISMATCHED_SEGMENT_DELIMITERS);
  UERRCODE(U_MISPLACED_ANCHOR_START);
  UERRCODE(U_MISPLACED_COMPOUND_FILTER);
  UERRCODE(U_MISPLACED_CURSOR_OFFSET);
  UERRCODE(U_MISPLACED_QUANTIFIER);
  UERRCODE(U_MISSING_OPERATOR);
  UERRCODE(U_MISSING_RESOURCE_ERROR);
  UERRCODE(U_MISSING_SEGMENT_CLOSE);
  UERRCODE(U_MULTIPLE_ANTE_CONTEXTS);
  UERRCODE(U_MULTIPLE_COMPOUND_FILTERS);
  UERRCODE(U_MULTIPLE_CURSORS);
  UERRCODE(U_MULTIPLE_EXPONENTIAL_SYMBOLS);
  UERRCODE(U_MULTIPLE_PAD_SPECIFIERS);
  UERRCODE(U_MULTIPLE_PERCENT_SYMBOLS);
  UERRCODE(U_MULTIPLE_PERMILL_SYMBOLS);
  UERRCODE(U_MULTIPLE_POST_CONTEXTS);
  UERRCODE(U_NO_SPACE_AVAILABLE);
  UERRCODE(U_NO_WRITE_PERMISSION);
  UERRCODE(U_PARSE_ERROR);
  UERRCODE(U_PARSE_ERROR_LIMIT);
  UERRCODE(U_PARSE_ERROR_START);
  UERRCODE(U_PATTERN_SYNTAX_ERROR);
  UERRCODE(U_PRIMARY_TOO_LONG_ERROR);
  UERRCODE(U_REGEX_BAD_ESCAPE_SEQUENCE);
  UERRCODE(U_REGEX_BAD_INTERVAL);
  UERRCODE(U_REGEX_ERROR_LIMIT);
  UERRCODE(U_REGEX_ERROR_START);
  UERRCODE(U_REGEX_INTERNAL_ERROR);
  UERRCODE(U_REGEX_INVALID_BACK_REF);
  UERRCODE(U_REGEX_INVALID_FLAG);
  UERRCODE(U_REGEX_INVALID_STATE);
  UERRCODE(U_REGEX_LOOK_BEHIND_LIMIT);
  UERRCODE(U_REGEX_MAX_LT_MIN);
  UERRCODE(U_REGEX_MISMATCHED_PAREN);
  UERRCODE(U_REGEX_NUMBER_TOO_BIG);
  UERRCODE(U_REGEX_PROPERTY_SYNTAX);
  UERRCODE(U_REGEX_RULE_SYNTAX);
  UERRCODE(U_REGEX_SET_CONTAINS_STRING);
  UERRCODE(U_REGEX_UNIMPLEMENTED);
  UERRCODE(U_RESOURCE_TYPE_MISMATCH);
  UERRCODE(U_RULE_MASK_ERROR);
  UERRCODE(U_SAFECLONE_ALLOCATED_WARNING);
  UERRCODE(U_SORT_KEY_TOO_SHORT_WARNING);
  UERRCODE(U_STANDARD_ERROR_LIMIT);
  UERRCODE(U_STATE_OLD_WARNING);
  UERRCODE(U_STATE_TOO_OLD_ERROR);
  UERRCODE(U_STRING_NOT_TERMINATED_WARNING);
  UERRCODE(U_TOO_MANY_ALIASES_ERROR);
  UERRCODE(U_TRAILING_BACKSLASH);
  UERRCODE(U_TRUNCATED_CHAR_FOUND);
  UERRCODE(U_UNCLOSED_SEGMENT);
  UERRCODE(U_UNDEFINED_SEGMENT_REFERENCE);
  UERRCODE(U_UNDEFINED_VARIABLE);
  UERRCODE(U_UNEXPECTED_TOKEN);
  UERRCODE(U_UNMATCHED_BRACES);
  UERRCODE(U_UNQUOTED_SPECIAL);
  UERRCODE(U_UNSUPPORTED_ATTRIBUTE);
  UERRCODE(U_UNSUPPORTED_ERROR);
  UERRCODE(U_UNSUPPORTED_ESCAPE_SEQUENCE);
  UERRCODE(U_UNSUPPORTED_PROPERTY);
  UERRCODE(U_UNTERMINATED_QUOTE);
  UERRCODE(U_USELESS_COLLATOR_ERROR);
  UERRCODE(U_USING_DEFAULT_WARNING);
  UERRCODE(U_USING_FALLBACK_WARNING);
  UERRCODE(U_VARIABLE_RANGE_EXHAUSTED);
  UERRCODE(U_VARIABLE_RANGE_OVERLAP);
  UERRCODE(U_ZERO_ERROR);

  // Legacy constants
#ifndef U_STRINGPREP_PROHIBITED_ERROR
# define U_STRINGPREP_PROHIBITED_ERROR 66560
#endif
#ifndef U_STRINGPREP_UNASSIGNED_ERROR
# define U_STRINGPREP_UNASSIGNED_ERROR 66561
#endif
#ifndef U_STRINGPREP_CHECK_BIDI_ERROR
# define U_STRINGPREP_CHECK_BIDI_ERROR 66562
#endif
  UERRCODE(U_STRINGPREP_PROHIBITED_ERROR);
  UERRCODE(U_STRINGPREP_UNASSIGNED_ERROR);
  UERRCODE(U_STRINGPREP_CHECK_BIDI_ERROR);
#undef UERRCODE
}

/////////////////////////////////////////////////////////////////////////////
// UTF8<->UTF16 string encoding conversion

icu::UnicodeString u16(const char *u8, int32_t u8_len, UErrorCode &error,
                       UChar32 subst /* =0 */) {
  error = U_ZERO_ERROR;
  if (u8_len == 0) {
    return icu::UnicodeString();
  }
  int32_t outlen;
  if (subst) {
    u_strFromUTF8WithSub(nullptr, 0, &outlen, u8, u8_len,
                         subst, nullptr, &error);
  } else {
    u_strFromUTF8(nullptr, 0, &outlen, u8, u8_len, &error);
  }
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return icu::UnicodeString();
  }
  icu::UnicodeString ret;
  auto out = ret.getBuffer(outlen + 1);
  error = U_ZERO_ERROR;
  if (subst) {
    u_strFromUTF8WithSub(out, outlen + 1, &outlen, u8, u8_len,
                         subst, nullptr, &error);
  } else {
    u_strFromUTF8(out, outlen + 1, &outlen, u8, u8_len, &error);
  }
  ret.releaseBuffer(outlen);
  if (U_FAILURE(error)) {
    return icu::UnicodeString();
  }
  return ret;
}

String u8(const UChar *u16, int32_t u16_len, UErrorCode &error) {
  error = U_ZERO_ERROR;
  if (u16_len == 0) {
    return empty_string();
  }
  int32_t outlen;
  u_strToUTF8(nullptr, 0, &outlen, u16, u16_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return String();
  }
  String ret(outlen + 1, ReserveString);
  char *out = ret.get()->mutableData();
  error = U_ZERO_ERROR;
  u_strToUTF8(out, outlen + 1, &outlen, u16, u16_len, &error);
  if (U_FAILURE(error)) {
    return String();
  }
  ret.setSize(outlen);
  return ret;
}

double VariantToMilliseconds(const Variant& arg) {
  if (arg.isNumeric(true)) {
    return U_MILLIS_PER_SECOND * arg.toDouble();
  }
  if (arg.isObject() &&
      arg.toObject()->instanceof(SystemLib::s_DateTimeInterfaceClass)) {
    return U_MILLIS_PER_SECOND *
           (double) DateTimeData::getTimestamp(arg.toObject());
  }
  // TODO: Handle object IntlCalendar
  return NAN;
}

} // namespace Intl

Intl::IntlExtension s_intl_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
