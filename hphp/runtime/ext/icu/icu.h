#ifndef incl_HPHP_ICU_H
#define incl_HPHP_ICU_H

#include "hphp/runtime/base/base-includes.h"
#include <unicode/utypes.h>
#include <unicode/ucnv.h>

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

  UErrorCode getErrorCode() const { return m_error.code; }
  const String getErrorMessage() const { return m_error.custom_error_message; }
};

DECLARE_EXTERN_REQUEST_LOCAL(IntlError, s_intl_error);

namespace Intl {

class Converter : public RequestEventHandler {
 public:
  void requestInit() override {
    UErrorCode error = U_ZERO_ERROR;
    m_utf8 = ucnv_open("utf-8", &error);
    assert(U_SUCCESS(error));
  }

  void requestShutdown() override {
    ucnv_close(m_utf8);
  }

  UConverter* utf8() const { return m_utf8; }

 private:
  UConverter *m_utf8;
};

DECLARE_EXTERN_REQUEST_LOCAL(Converter, s_intl_converters);

inline const String GetDefaultLocale() {
  // TODO: Move this to Locale when we have it
  return "en_US";
}

// Common encoding conversions UTF8<->UTF16
String u16(const char *u8, int32_t u8_len, UErrorCode &error);
inline String u16(const String u8, UErrorCode &error) {
  return u16(u8.c_str(), u8.size(), error);
}
String u8(const UChar *u16, int32_t u16_len, UErrorCode &error);
inline String u8(const String &u16, UErrorCode &error) {
  return u8((const UChar *)u16.c_str(), u16.size() / sizeof(UChar), error);
}

} // namespace Intl

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_ICU_H
