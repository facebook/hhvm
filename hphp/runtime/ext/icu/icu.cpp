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
    char message[1024];
    int message_len = vsnprintf(message, sizeof(message), format, args);
    m_errorMessage = std::string(message, message_len);
    va_end(args);
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

/* gcc 4.7 doesn't support thread_locale storage
 * required for dynamic initializers (like std::string)
 * So wrap it up in a RequestEventHandler until we set
 * gcc 4.8 as our minimum version
 */
struct DefaultLocale final : RequestEventHandler {
  void requestInit() override {}
  void requestShutdown() override {}
  std::string m_defaultLocale;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(DefaultLocale, s_default_locale);

void IntlExtension::bindIniSettings() {
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   "intl.default_locale", "",
                   &s_default_locale->m_defaultLocale);
}

const String GetDefaultLocale() {
  String locale(s_default_locale->m_defaultLocale);
  if (locale.empty()) {
    locale = String(uloc_getDefault(), CopyString);
  }
  return locale;
}

bool SetDefaultLocale(const String& locale) {
  s_default_locale->m_defaultLocale = locale.toCppString();
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
    return empty_string;
  }
  int32_t outlen;
  u_strToUTF8(nullptr, 0, &outlen, u16, u16_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return null_string;
  }
  String ret(outlen + 1, ReserveString);
  char *out = ret.get()->mutableData();
  error = U_ZERO_ERROR;
  u_strToUTF8(out, outlen + 1, &outlen, u16, u16_len, &error);
  if (U_FAILURE(error)) {
    return null_string;
  }
  ret.setSize(outlen);
  return ret;
}

double VariantToMilliseconds(const Variant& arg) {
  if (arg.isNumeric(true)) {
    return U_MILLIS_PER_SECOND * arg.toDouble();
  }
  // TODO: Handle object IntlCalendar and DateTime
  return NAN;
}

} // namespace Intl

Intl::IntlExtension s_intl_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
