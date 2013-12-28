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
#ifndef incl_HPHP_ICU_H
#define incl_HPHP_ICU_H

#include "hphp/runtime/base/base-includes.h"
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

class IntlError : public RequestEventHandler {
public:
  intl_error m_error;
  IntlError() {
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

DECLARE_EXTERN_REQUEST_LOCAL(IntlError, s_intl_error);

namespace Intl {

extern const StaticString s_resdata;
class IntlResourceData : public SweepableResourceData {
 public:
  template<class T>
  static T* GetResData(Object obj, const String& ctx) {
    if (obj.isNull()) {
      raise_error("NULL object passed");
      return nullptr;
    }
    auto res = obj->o_get(s_resdata, false, ctx);
    if (!res.isResource()) {
      return nullptr;
    }
    auto ret = res.toResource().getTyped<T>(false, false);
    if (!ret) {
      return nullptr;
    }
    if (ret->isInvalid()) {
      ret->setError(U_ILLEGAL_ARGUMENT_ERROR,
                    "Found unconstructed %s", ctx.c_str());
      return nullptr;
    }
    return ret;
  }

  Object WrapResData(const String& ctx) {
    auto cls = Unit::lookupClass(ctx.get());
    auto obj = ObjectData::newInstance(cls);
    Object ret(obj);
    obj->o_set(s_resdata, Resource(this), ctx);
    return ret;
  }

  void setError(UErrorCode code, const char *format, ...) {
    va_list args;
    va_start(args, format);
    s_intl_error->set(code, format, args);
    va_end(args);
    m_error = s_intl_error->m_error;
  }

  void setError(UErrorCode code) {
    const char *errorMsg = u_errorName(code);
    setError(code, "%s", errorMsg);
  }

  void throwException(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    throw Object(SystemLib::AllocExceptionObject(buffer));
  }

  UErrorCode getErrorCode() const {
    return m_error.code;
  }

  String getErrorMessage() const {
    return m_error.custom_error_message;
  }

 private:
  intl_error m_error;
};

class RequestData : public RequestEventHandler {
 public:
  void requestInit() override {}

  void requestShutdown() override {
    if (m_utf8) {
      ucnv_close(m_utf8);
      m_utf8 = nullptr;
    }
  }

  UConverter* utf8() {
    if (!m_utf8) {
      UErrorCode error = U_ZERO_ERROR;
      m_utf8 = ucnv_open("utf-8", &error);
      assert(U_SUCCESS(error));
    }
    return m_utf8;
  }

  const std::string& getDefaultLocale() const { return m_defaultLocale; }
  void setDefaultLocale(const std::string& loc) { m_defaultLocale = loc; }

 private:
  UConverter *m_utf8 = nullptr;
  std::string m_defaultLocale;
};

DECLARE_EXTERN_REQUEST_LOCAL(RequestData, s_intl_request);

const String GetDefaultLocale();
bool SetDefaultLocale(const String& locale);

// Common encoding conversions UTF8<->UTF16
String u16(const char *u8, int32_t u8_len, UErrorCode &error);
inline String u16(const String u8, UErrorCode &error) {
  return u16(u8.c_str(), u8.size(), error);
}
String u8(const UChar *u16, int32_t u16_len, UErrorCode &error);
inline String u8(const String &u16, UErrorCode &error) {
  return u8((const UChar *)u16.c_str(), u16.size() / sizeof(UChar), error);
}
inline String u8(const icu::UnicodeString& u16, UErrorCode &error) {
  return u8(u16.getBuffer(), u16.length(), error);
}

bool ustring_from_char(icu::UnicodeString& ret,
                       const String& str,
                       UErrorCode &error);

class IntlExtension : public Extension {
 public:
  // Some apps/frameworks get confused by a claim that
  // the intl extension is loaded, yet not all the classes exist
  // Lie for now by using another name.  Change it when intl
  // coverage is complete
  IntlExtension() : Extension("intl.not-done") {}

  void moduleInit() override {
    bindIniSettings();
    initLocale();
    initNumberFormatter();
    initTimeZone();
    initIterator();
  }

 private:
  static bool icu_on_update_default_locale(const String& value, void *p) {
    s_intl_request->setDefaultLocale(value->data());
    return true;
  }
  static String icu_get_default_locale(void *p) {
    return s_intl_request->getDefaultLocale();
  }

  void bindIniSettings();
  void initLocale();
  void initNumberFormatter();
  void initTimeZone();
  void initIterator();
};

} // namespace Intl

extern Intl::IntlExtension s_intl_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_ICU_H
