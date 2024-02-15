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
#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/util/rds-local.h"
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

[[noreturn]]
void IntlError::throwException(const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  SystemLib::throwExceptionObject(buffer);
}

/////////////////////////////////////////////////////////////////////////////
// INI Setting

static RDS_LOCAL(std::string, s_defaultLocale);

void IntlExtension::bindIniSettings() {
  s_defaultLocale.getCheck();
  IniSetting::Bind(this, IniSetting::Mode::Request,
                   "intl.default_locale", "",
                   s_defaultLocale.get()
                 );
}

const String GetDefaultLocale() {
  if (s_defaultLocale->empty()) {
    return String(uloc_getDefault(), CopyString);
  }
  return *(s_defaultLocale.get());
}

bool SetDefaultLocale(const String& locale) {
  *s_defaultLocale = locale.toCppString();
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// Common extension init

void IntlExtension::registerNativeConstants() {
#ifdef U_ICU_DATA_VERSION
  HHVM_RC_STR(INTL_ICU_DATA_VERSION, U_ICU_DATA_VERSION);
#endif
  HHVM_RC_STR(INTL_ICU_VERSION, U_ICU_VERSION);

  // UErrorCode constants
  HHVM_RC_INT_SAME(U_AMBIGUOUS_ALIAS_WARNING);
  HHVM_RC_INT_SAME(U_BAD_VARIABLE_DEFINITION);
  HHVM_RC_INT_SAME(U_BRK_ASSIGN_ERROR);
  HHVM_RC_INT_SAME(U_BRK_ERROR_LIMIT);
  HHVM_RC_INT_SAME(U_BRK_ERROR_START);
  HHVM_RC_INT_SAME(U_BRK_HEX_DIGITS_EXPECTED);
  HHVM_RC_INT_SAME(U_BRK_INIT_ERROR);
  HHVM_RC_INT_SAME(U_BRK_INTERNAL_ERROR);
  HHVM_RC_INT_SAME(U_BRK_MALFORMED_RULE_TAG);
  HHVM_RC_INT_SAME(U_BRK_MISMATCHED_PAREN);
  HHVM_RC_INT_SAME(U_BRK_NEW_LINE_IN_QUOTED_STRING);
  HHVM_RC_INT_SAME(U_BRK_RULE_EMPTY_SET);
  HHVM_RC_INT_SAME(U_BRK_RULE_SYNTAX);
  HHVM_RC_INT_SAME(U_BRK_SEMICOLON_EXPECTED);
  HHVM_RC_INT_SAME(U_BRK_UNCLOSED_SET);
  HHVM_RC_INT_SAME(U_BRK_UNDEFINED_VARIABLE);
  HHVM_RC_INT_SAME(U_BRK_UNRECOGNIZED_OPTION);
  HHVM_RC_INT_SAME(U_BRK_VARIABLE_REDFINITION);
  HHVM_RC_INT_SAME(U_BUFFER_OVERFLOW_ERROR);
  HHVM_RC_INT_SAME(U_CE_NOT_FOUND_ERROR);
  HHVM_RC_INT_SAME(U_COLLATOR_VERSION_MISMATCH);
  HHVM_RC_INT_SAME(U_DIFFERENT_UCA_VERSION);
  HHVM_RC_INT_SAME(U_ENUM_OUT_OF_SYNC_ERROR);
  HHVM_RC_INT_SAME(U_ERROR_LIMIT);
  HHVM_RC_INT_SAME(U_ERROR_WARNING_LIMIT);
  HHVM_RC_INT_SAME(U_ERROR_WARNING_START);
  HHVM_RC_INT_SAME(U_FILE_ACCESS_ERROR);
  HHVM_RC_INT_SAME(U_FMT_PARSE_ERROR_LIMIT);
  HHVM_RC_INT_SAME(U_FMT_PARSE_ERROR_START);
  HHVM_RC_INT_SAME(U_ILLEGAL_ARGUMENT_ERROR);
  HHVM_RC_INT_SAME(U_ILLEGAL_CHARACTER);
  HHVM_RC_INT_SAME(U_ILLEGAL_CHAR_FOUND);
  HHVM_RC_INT_SAME(U_ILLEGAL_CHAR_IN_SEGMENT);
  HHVM_RC_INT_SAME(U_ILLEGAL_ESCAPE_SEQUENCE);
  HHVM_RC_INT_SAME(U_ILLEGAL_PAD_POSITION);
  HHVM_RC_INT_SAME(U_INDEX_OUTOFBOUNDS_ERROR);
  HHVM_RC_INT_SAME(U_INTERNAL_PROGRAM_ERROR);
  HHVM_RC_INT_SAME(U_INTERNAL_TRANSLITERATOR_ERROR);
  HHVM_RC_INT_SAME(U_INVALID_CHAR_FOUND);
  HHVM_RC_INT_SAME(U_INVALID_FORMAT_ERROR);
  HHVM_RC_INT_SAME(U_INVALID_FUNCTION);
  HHVM_RC_INT_SAME(U_INVALID_ID);
  HHVM_RC_INT_SAME(U_INVALID_PROPERTY_PATTERN);
  HHVM_RC_INT_SAME(U_INVALID_RBT_SYNTAX);
  HHVM_RC_INT_SAME(U_INVALID_STATE_ERROR);
  HHVM_RC_INT_SAME(U_INVALID_TABLE_FILE);
  HHVM_RC_INT_SAME(U_INVALID_TABLE_FORMAT);
  HHVM_RC_INT_SAME(U_INVARIANT_CONVERSION_ERROR);
  HHVM_RC_INT_SAME(U_MALFORMED_EXPONENTIAL_PATTERN);
  HHVM_RC_INT_SAME(U_MALFORMED_PRAGMA);
  HHVM_RC_INT_SAME(U_MALFORMED_RULE);
  HHVM_RC_INT_SAME(U_MALFORMED_SET);
  HHVM_RC_INT_SAME(U_MALFORMED_SYMBOL_REFERENCE);
  HHVM_RC_INT_SAME(U_MALFORMED_UNICODE_ESCAPE);
  HHVM_RC_INT_SAME(U_MALFORMED_VARIABLE_DEFINITION);
  HHVM_RC_INT_SAME(U_MALFORMED_VARIABLE_REFERENCE);
  HHVM_RC_INT_SAME(U_MEMORY_ALLOCATION_ERROR);
  HHVM_RC_INT_SAME(U_MESSAGE_PARSE_ERROR);
  HHVM_RC_INT_SAME(U_MISMATCHED_SEGMENT_DELIMITERS);
  HHVM_RC_INT_SAME(U_MISPLACED_ANCHOR_START);
  HHVM_RC_INT_SAME(U_MISPLACED_COMPOUND_FILTER);
  HHVM_RC_INT_SAME(U_MISPLACED_CURSOR_OFFSET);
  HHVM_RC_INT_SAME(U_MISPLACED_QUANTIFIER);
  HHVM_RC_INT_SAME(U_MISSING_OPERATOR);
  HHVM_RC_INT_SAME(U_MISSING_RESOURCE_ERROR);
  HHVM_RC_INT_SAME(U_MISSING_SEGMENT_CLOSE);
  HHVM_RC_INT_SAME(U_MULTIPLE_ANTE_CONTEXTS);
  HHVM_RC_INT_SAME(U_MULTIPLE_COMPOUND_FILTERS);
  HHVM_RC_INT_SAME(U_MULTIPLE_CURSORS);
  HHVM_RC_INT_SAME(U_MULTIPLE_DECIMAL_SEPARATORS);
  HHVM_RC_INT_SAME(U_MULTIPLE_EXPONENTIAL_SYMBOLS);
  HHVM_RC_INT_SAME(U_MULTIPLE_PAD_SPECIFIERS);
  HHVM_RC_INT_SAME(U_MULTIPLE_PERCENT_SYMBOLS);
  HHVM_RC_INT_SAME(U_MULTIPLE_PERMILL_SYMBOLS);
  HHVM_RC_INT_SAME(U_MULTIPLE_POST_CONTEXTS);
  HHVM_RC_INT_SAME(U_NO_SPACE_AVAILABLE);
  HHVM_RC_INT_SAME(U_NO_WRITE_PERMISSION);
  HHVM_RC_INT_SAME(U_PARSE_ERROR);
  HHVM_RC_INT_SAME(U_PARSE_ERROR_LIMIT);
  HHVM_RC_INT_SAME(U_PARSE_ERROR_START);
  HHVM_RC_INT_SAME(U_PATTERN_SYNTAX_ERROR);
  HHVM_RC_INT_SAME(U_PRIMARY_TOO_LONG_ERROR);
  HHVM_RC_INT_SAME(U_REGEX_BAD_ESCAPE_SEQUENCE);
  HHVM_RC_INT_SAME(U_REGEX_BAD_INTERVAL);
  HHVM_RC_INT_SAME(U_REGEX_ERROR_LIMIT);
  HHVM_RC_INT_SAME(U_REGEX_ERROR_START);
  HHVM_RC_INT_SAME(U_REGEX_INTERNAL_ERROR);
  HHVM_RC_INT_SAME(U_REGEX_INVALID_BACK_REF);
  HHVM_RC_INT_SAME(U_REGEX_INVALID_FLAG);
  HHVM_RC_INT_SAME(U_REGEX_INVALID_STATE);
  HHVM_RC_INT_SAME(U_REGEX_LOOK_BEHIND_LIMIT);
  HHVM_RC_INT_SAME(U_REGEX_MAX_LT_MIN);
  HHVM_RC_INT_SAME(U_REGEX_MISMATCHED_PAREN);
  HHVM_RC_INT_SAME(U_REGEX_NUMBER_TOO_BIG);
  HHVM_RC_INT_SAME(U_REGEX_PROPERTY_SYNTAX);
  HHVM_RC_INT_SAME(U_REGEX_RULE_SYNTAX);
  HHVM_RC_INT_SAME(U_REGEX_SET_CONTAINS_STRING);
  HHVM_RC_INT_SAME(U_REGEX_UNIMPLEMENTED);
  HHVM_RC_INT_SAME(U_RESOURCE_TYPE_MISMATCH);
  HHVM_RC_INT_SAME(U_RULE_MASK_ERROR);
  HHVM_RC_INT_SAME(U_SAFECLONE_ALLOCATED_WARNING);
  HHVM_RC_INT_SAME(U_SORT_KEY_TOO_SHORT_WARNING);
  HHVM_RC_INT_SAME(U_STANDARD_ERROR_LIMIT);
  HHVM_RC_INT_SAME(U_STATE_OLD_WARNING);
  HHVM_RC_INT_SAME(U_STATE_TOO_OLD_ERROR);
  HHVM_RC_INT_SAME(U_STRING_NOT_TERMINATED_WARNING);
  HHVM_RC_INT_SAME(U_TOO_MANY_ALIASES_ERROR);
  HHVM_RC_INT_SAME(U_TRAILING_BACKSLASH);
  HHVM_RC_INT_SAME(U_TRUNCATED_CHAR_FOUND);
  HHVM_RC_INT_SAME(U_UNCLOSED_SEGMENT);
  HHVM_RC_INT_SAME(U_UNDEFINED_SEGMENT_REFERENCE);
  HHVM_RC_INT_SAME(U_UNDEFINED_VARIABLE);
  HHVM_RC_INT_SAME(U_UNEXPECTED_TOKEN);
  HHVM_RC_INT_SAME(U_UNMATCHED_BRACES);
  HHVM_RC_INT_SAME(U_UNQUOTED_SPECIAL);
  HHVM_RC_INT_SAME(U_UNSUPPORTED_ATTRIBUTE);
  HHVM_RC_INT_SAME(U_UNSUPPORTED_ERROR);
  HHVM_RC_INT_SAME(U_UNSUPPORTED_ESCAPE_SEQUENCE);
  HHVM_RC_INT_SAME(U_UNSUPPORTED_PROPERTY);
  HHVM_RC_INT_SAME(U_UNTERMINATED_QUOTE);
  HHVM_RC_INT_SAME(U_USELESS_COLLATOR_ERROR);
  HHVM_RC_INT_SAME(U_USING_DEFAULT_WARNING);
  HHVM_RC_INT_SAME(U_USING_FALLBACK_WARNING);
  HHVM_RC_INT_SAME(U_VARIABLE_RANGE_EXHAUSTED);
  HHVM_RC_INT_SAME(U_VARIABLE_RANGE_OVERLAP);
  HHVM_RC_INT_SAME(U_ZERO_ERROR);

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
  HHVM_RC_INT_SAME(U_STRINGPREP_PROHIBITED_ERROR);
  HHVM_RC_INT_SAME(U_STRINGPREP_UNASSIGNED_ERROR);
  HHVM_RC_INT_SAME(U_STRINGPREP_CHECK_BIDI_ERROR);
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
      arg.toObject()->instanceof(SystemLib::getDateTimeInterfaceClass())) {
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
