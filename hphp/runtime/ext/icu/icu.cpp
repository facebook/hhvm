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
#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/base/ini-setting.h"

#include <unicode/uloc.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_REQUEST_LOCAL(IntlError, s_intl_error);

namespace Intl {

IMPLEMENT_REQUEST_LOCAL(RequestData, s_intl_request);
const StaticString s_resdata("__resdata");

void IntlExtension::bindIniSettings() {
  IniSetting::Bind("intl.default_locale", "",
                   icu_on_update_default_locale, icu_get_default_locale,
                   nullptr);
}

const String GetDefaultLocale() {
  String locale(s_intl_request->getDefaultLocale());
  if (locale.empty()) {
    locale = String(uloc_getDefault(), CopyString);
  }
  return locale;
}

bool SetDefaultLocale(const String& locale) {
  s_intl_request->setDefaultLocale(locale->data());
  return true;
}

String u16(const char *u8, int32_t u8_len, UErrorCode &error) {
  error = U_ZERO_ERROR;
  int32_t outlen = ucnv_toUChars(s_intl_request->utf8(),
                                 nullptr, 0, u8, u8_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return uninit_null();
  }
  String ret = String(sizeof(UChar) * (outlen + 1), ReserveString);
  UChar *out = (UChar*)ret->mutableData();
  error = U_ZERO_ERROR;
  outlen = ucnv_toUChars(s_intl_request->utf8(),
                         out, outlen + 1, u8, u8_len, &error);
  if (U_FAILURE(error)) {
    return uninit_null();
  }
  ret.setSize(outlen * sizeof(UChar));
  return ret;
}

String u8(const UChar *u16, int32_t u16_len, UErrorCode &error) {
  error = U_ZERO_ERROR;
  int32_t outlen = ucnv_fromUChars(s_intl_request->utf8(),
                                   nullptr, 0, u16, u16_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return uninit_null();
  }
  String ret(outlen, ReserveString);
  char *out = ret->mutableData();
  error = U_ZERO_ERROR;
  outlen = ucnv_fromUChars(s_intl_request->utf8(),
                           out, outlen + 1, u16, u16_len, &error);
  if (U_FAILURE(error)) {
    return uninit_null();
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

} // namespace Intl

Intl::IntlExtension s_intl_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
