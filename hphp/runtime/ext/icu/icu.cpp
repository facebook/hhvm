#include "hphp/runtime/ext/icu/icu.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_REQUEST_LOCAL(IntlError, s_intl_error);

namespace Intl {

IMPLEMENT_REQUEST_LOCAL(Converter, s_intl_converters);

String u16(const char *u8, int32_t u8_len, UErrorCode &error) {
  error = U_ZERO_ERROR;
  int32_t outlen = ucnv_toUChars(s_intl_converters->utf8(),
                                 nullptr, 0, u8, u8_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return uninit_null();
  }
  String ret = String(sizeof(UChar) * (outlen + 1), ReserveString);
  UChar *out = (UChar*)ret->mutableData();
  error = U_ZERO_ERROR;
  outlen = ucnv_toUChars(s_intl_converters->utf8(),
                         out, outlen + 1, u8, u8_len, &error);
  if (U_FAILURE(error)) {
    return uninit_null();
  }
  ret.setSize(outlen * sizeof(UChar));
  return ret;
}

String u8(const UChar *u16, int32_t u16_len, UErrorCode &error) {
  error = U_ZERO_ERROR;
  int32_t outlen = ucnv_fromUChars(s_intl_converters->utf8(),
                                   nullptr, 0, u16, u16_len, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    return uninit_null();
  }
  String ret(outlen, ReserveString);
  char *out = ret->mutableData();
  error = U_ZERO_ERROR;
  outlen = ucnv_fromUChars(s_intl_converters->utf8(),
                           out, outlen + 1, u16, u16_len, &error);
  if (U_FAILURE(error)) {
    return uninit_null();
  }
  ret.setSize(outlen);
  return ret;
}

} // namespace Intl

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
