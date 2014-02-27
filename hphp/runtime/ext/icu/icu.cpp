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

#include <unicode/uloc.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_REQUEST_LOCAL(IntlGlobalError, s_intl_error);

namespace Intl {

/////////////////////////////////////////////////////////////////////////////
// INI Setting

/* gcc 4.7 doesn't support thread_locale storage
 * required for dynamic initializers (like std::string)
 * So wrap it up in a RequestEventHandler until we set
 * gcc 4.8 as our minimum version
 */
class DefaultLocale : public RequestEventHandler {
 public:
  void requestInit() override {}
  void requestShutdown() override {}

  std::string getDefaultLocale() const { return m_defaultLocale; }
  void setDefaultLocale(const std::string& locale) {
    m_defaultLocale = locale;
  }

 private:
  std::string m_defaultLocale;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(DefaultLocale, s_default_locale);

std::string icu_get_default_locale(void *p) {
  return s_default_locale->getDefaultLocale();
}

bool icu_on_update_default_locale(const String& value, void *p) {
  s_default_locale->setDefaultLocale(value->toCppString());
  return true;
}

void IntlExtension::bindIniSettings() {
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   "intl.default_locale", "",
                   icu_on_update_default_locale, icu_get_default_locale,
                   nullptr);
}

const String GetDefaultLocale() {
  String locale(s_default_locale->getDefaultLocale());
  if (locale.empty()) {
    locale = String(uloc_getDefault(), CopyString);
  }
  return locale;
}

bool SetDefaultLocale(const String& locale) {
  s_default_locale->setDefaultLocale(locale->toCppString());
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

String u16(const char *u8, int32_t u8_len, UErrorCode &error) {
  error = U_ZERO_ERROR;
  if (u8_len == 0) {
    return empty_string;
  }
  int32_t outlen;
  u_strFromUTF8(nullptr, 0, &outlen, u8, u8_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return null_string;
  }
  String ret = String(sizeof(UChar) * (outlen + 1), ReserveString);
  UChar *out = (UChar*)ret->mutableData();
  error = U_ZERO_ERROR;
  u_strFromUTF8(out, outlen + 1, &outlen, u8, u8_len, &error);
  if (U_FAILURE(error)) {
    return null_string;
  }
  ret.setSize(outlen * sizeof(UChar));
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
  char *out = ret->mutableData();
  error = U_ZERO_ERROR;
  u_strToUTF8(out, outlen + 1, &outlen, u16, u16_len, &error);
  if (U_FAILURE(error)) {
    return null_string;
  }
  ret.setSize(outlen);
  return ret;
}

bool ustring_from_char(icu::UnicodeString& ret,
                       const String& str,
                       UErrorCode &error) {
  int32_t capacity = str.size() + 1;
  UChar *utf16 = ret.getBuffer(capacity);
  int32_t utf16_len = 0;
  error = U_ZERO_ERROR;
  u_strFromUTF8WithSub(utf16, ret.getCapacity(), &utf16_len,
                       str.c_str(), str.size(),
                       U_SENTINEL /* no substitution */,
                       nullptr, &error);
  ret.releaseBuffer(utf16_len);
  if (U_FAILURE(error)) {
    ret.setToBogus();
    return false;
  }
  return true;
}

double VariantToMilliseconds(CVarRef arg) {
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
