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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/vm/native-data.h"
#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct intl_error {
  UErrorCode code;
  String custom_error_message;
  intl_error() : code(U_ZERO_ERROR) {}
  void clear() {
    code = U_ZERO_ERROR;
    custom_error_message.reset();
  }
};

/* Request global error set by all Intl classes
 * and accessed via intl_get_error_code|message()
 */
class IntlGlobalError : public RequestEventHandler {
public:
  intl_error m_error;
  IntlGlobalError() {
    m_error.clear();
  }
  void requestInit() override {
    m_error.clear();
  }
  void requestShutdown() override {
    m_error.clear();
  }

  void set(UErrorCode code, const char *format, va_list args) {
    char message[1024];
    int message_len = vsnprintf(message, sizeof(message), format, args);
    m_error.code = code;
    m_error.custom_error_message = String(message, message_len, CopyString);
  }

  void set(UErrorCode code, const char *format, ...) {
    va_list args;
    va_start(args, format);
    set(code, format, args);
    va_end(args);
  }

  void clear() { m_error.clear(); }

  UErrorCode getErrorCode() const { return m_error.code; }
  const String getErrorMessage() const { return m_error.custom_error_message; }
};

DECLARE_EXTERN_REQUEST_LOCAL(IntlGlobalError, s_intl_error);

namespace Intl {

/* Common error handling logic used by all Intl classes
 */
class IntlError {
 public:
  void setError(intl_error& err) {
    s_intl_error->m_error = err;
    m_errorCode = err.code;
    if (err.custom_error_message.empty()) {
      m_errorMessage.clear();
    } else {
      m_errorMessage = err.custom_error_message->toCppString();
    }
  }

  void setError(UErrorCode code, const char *format, ...) {
    va_list args;
    va_start(args, format);
    s_intl_error->set(code, format, args);
    va_end(args);
    m_errorCode = code;
    if (s_intl_error->m_error.custom_error_message.empty()) {
      m_errorMessage.clear();
    } else {
      m_errorMessage =
        s_intl_error->m_error.custom_error_message->toCppString();
    }
  }

  void setError(UErrorCode code) {
    const char *errorMsg = u_errorName(code);
    setError(code, "%s", errorMsg);
  }

  void clearError() {
    m_errorCode = U_ZERO_ERROR;
    m_errorMessage.clear();
  }

  void throwException(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    throw Object(SystemLib::AllocExceptionObject(buffer));
  }

  UErrorCode getErrorCode() const { return m_errorCode; }

  String getErrorMessage() const {
    auto errorName = u_errorName(m_errorCode);
    if (m_errorMessage.empty()) {
      return errorName;
    }
    return m_errorMessage + ": " + errorName;
  }

 private:
   /* NativeData instances can't contain sweepable objects
    * Map around s_intl_error's use of String
    * TODO: Finish intl extension and unify s_intl_error back
    * onto std::string -sgolemon(2014-02-20)
    */
  std::string m_errorMessage;
  UErrorCode m_errorCode;
};

template<class T>
T* GetData(Object obj, const String& ctx) {
  if (obj.isNull()) {
    raise_error("NULL object passed");
    return nullptr;
  }
  auto ret = Native::data<T>(obj.get());
  if (!ret) {
    return nullptr;
  }
  if (!ret->isValid()) {
    ret->setError(U_ILLEGAL_ARGUMENT_ERROR,
                  "Found unconstructed %s", ctx.c_str());
    return nullptr;
  }
  return ret;
}

/////////////////////////////////////////////////////////////////////////////

const String GetDefaultLocale();
inline String localeOrDefault(const String& str) {
  return str.empty() ? GetDefaultLocale() : str;
}
bool SetDefaultLocale(const String& locale);
double VariantToMilliseconds(CVarRef arg);

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

class IntlExtension : public Extension {
 public:
  // Some apps/frameworks get confused by a claim that
  // the intl extension is loaded, yet not all the classes exist
  // Lie for now by using another name.  Change it when intl
  // coverage is complete
  IntlExtension() : Extension("intl.not-done", "1.1.0") {}

  void moduleInit() override {
    bindConstants();
    initLocale();
    initNumberFormatter();
    initTimeZone();
    initIterator();
    initDateFormatter();
    initCalendar();
    initGrapheme();
  }

  void threadInit() override {
    bindIniSettings();
  }

 private:
  void bindIniSettings();
  void bindConstants();
  void initLocale();
  void initNumberFormatter();
  void initTimeZone();
  void initIterator();
  void initDateFormatter();
  void initCalendar();
  void initGrapheme();
};

} // namespace Intl

extern Intl::IntlExtension s_intl_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_ICU_H
