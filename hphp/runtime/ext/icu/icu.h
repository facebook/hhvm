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
#ifndef incl_HPHP_ICU_H
#define incl_HPHP_ICU_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"
#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

namespace Intl {

/* Common error handling logic used by all Intl classes
 */
class IntlError {
 public:
  void setError(UErrorCode code, const char *format = nullptr, ...);
  void clearError(bool clearGlobalError = true);

  Object getException(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return SystemLib::AllocExceptionObject(buffer);
  }

  UErrorCode getErrorCode() const { return m_errorCode; }

  String getErrorMessage(bool appendCode = true) const {
    if (!appendCode) {
      return m_errorMessage;
    }
    auto errorName = u_errorName(m_errorCode);
    if (m_errorMessage.empty()) {
      return errorName;
    }
    return m_errorMessage + ": " + errorName;
  }

 private:
  std::string m_errorMessage;
  UErrorCode m_errorCode{U_ZERO_ERROR};
};

template<class T>
T* GetData(ObjectData* obj, const String& ctx) {
  auto const ret = Native::data<T>(obj);
  if (!ret->isValid()) {
    ret->setError(U_ILLEGAL_ARGUMENT_ERROR,
                  "Found unconstructed %s", ctx.c_str());
    return nullptr;
  }
  return ret;
}

template<class T>
T* GetData(Object, const String&);  // get a link error.

/////////////////////////////////////////////////////////////////////////////

const String GetDefaultLocale();
inline String localeOrDefault(const String& str) {
  return str.empty() ? GetDefaultLocale() : str;
}
bool SetDefaultLocale(const String& locale);
double VariantToMilliseconds(const Variant& arg);

// Common encoding conversions UTF8<->UTF16
icu::UnicodeString u16(const char* u8, int32_t u8_len, UErrorCode &error,
                       UChar32 subst = 0);
inline icu::UnicodeString u16(const String& u8, UErrorCode& error,
                       UChar32 subst = 0) {
  return u16(u8.c_str(), u8.size(), error, subst);
}
String u8(const UChar *u16, int32_t u16_len, UErrorCode &error);
inline String u8(const icu::UnicodeString& u16, UErrorCode& error) {
  return u8(u16.getBuffer(), u16.length(), error);
}

class IntlExtension final : public Extension {
 public:
  IntlExtension() : Extension("intl", "1.1.0") {}

  void moduleInit() override {
    bindConstants();
    initICU(); // HHVM-specific ICU functions
    initLocale();
    initNumberFormatter();
    initTimeZone();
    initIterator();
    initDateFormatter();
    initDatePatternGenerator();
    initCalendar();
    initGrapheme();
    initBreakIterator(); // Must come after initIterator()
    initUChar();
    initUConverter();
    initUcsDet();
    initUSpoof();
    initMisc();
    initCollator();
    initMessageFormatter();
    initNormalizer();
    initResourceBundle();
    initTransliterator();
  }

  void threadInit() override {
    bindIniSettings();
  }
  void threadShutdown() override;
 private:
  void bindIniSettings();
  void bindConstants();
  void initICU();
  void initLocale();
  void initNumberFormatter();
  void initTimeZone();
  void initIterator();
  void initDateFormatter();
  void initDatePatternGenerator();
  void initCalendar();
  void initGrapheme();
  void initBreakIterator();
  void initUChar();
  void initUConverter();
  void initUcsDet();
  void initUSpoof();
  void initMisc();
  void initCollator();
  void initMessageFormatter();
  void initNormalizer();
  void initResourceBundle();
  void initTransliterator();
};

} // namespace Intl

/* Request global error set by all Intl classes
 * and accessed via intl_get_error_code|message()
 */
struct IntlGlobalError final : RequestEventHandler, Intl::IntlError {
  IntlGlobalError() {}
  void requestInit() override {}
  void requestShutdown() override {
    clearError();
  }
};
DECLARE_EXTERN_REQUEST_LOCAL(IntlGlobalError, s_intl_error);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_ICU_H
