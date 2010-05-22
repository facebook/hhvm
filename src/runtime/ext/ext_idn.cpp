/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_idn.h>
#include <unicode/uidna.h>
#include <unicode/ustring.h>
#include <runtime/base/zend/intl_convert.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(idn);
///////////////////////////////////////////////////////////////////////////////

// NOTE: We rely on "runtime/ext/ext_array.cpp" to call u_init() to
// initialize the ICU library for us during startup. For more info
// see the ICUInitializer class.

enum {
  INTL_IDN_TO_ASCII = 0,
  INTL_IDN_TO_UTF8
};

Variant php_intl_idn_to(CStrRef domain, Variant errorcode, int mode) {
  long option = 0;
  UChar* ustring = NULL;
  int ustring_len = 0;
  UErrorCode status;
  char     *converted_utf8 = NULL;
  int32_t   converted_utf8_len;
  UChar*    converted = NULL;
  int32_t   converted_ret_len;

  // Convert the string to UTF-16
  status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&ustring, &ustring_len,
      (char*)domain.data(), domain.size(), &status);
  if (U_FAILURE(status)) {
    free(ustring);
    errorcode = status;
    return false;
  }

  // Call the appropriate IDN function
  int converted_len = (ustring_len > 1) ? ustring_len : 1;
  for (;;) {
    UParseError parse_error;
    status = U_ZERO_ERROR;
    converted = (UChar*)malloc(sizeof(UChar)*converted_len);
    // If the malloc failed, bail out
    if (!converted) {
      free(ustring);
      errorcode = U_MEMORY_ALLOCATION_ERROR;
      return false;
    }
    if (mode == INTL_IDN_TO_ASCII) {
      converted_ret_len = uidna_IDNToASCII(ustring,
          ustring_len, converted, converted_len,
          (int32_t)option, &parse_error, &status);
    } else {
      converted_ret_len = uidna_IDNToUnicode(ustring,
          ustring_len, converted, converted_len,
          (int32_t)option, &parse_error, &status);
    }
    if (status != U_BUFFER_OVERFLOW_ERROR)
      break;
    // If we have a buffer overflow error, try again with a larger buffer
    free(converted);
    converted = NULL;
    converted_len = converted_len * 2;
  }
  free(ustring);
  if (U_FAILURE(status)) {
    free(converted);
    errorcode = status;
    return false;
  }

  // Convert the string back to UTF-8
  status = U_ZERO_ERROR;
  intl_convert_utf16_to_utf8(&converted_utf8, &converted_utf8_len,
      converted, converted_ret_len, &status);
  free(converted);
  if (U_FAILURE(status)) {
    free(converted_utf8);
    errorcode = status;
    return false;
  }

  // Return the string
  return String(converted_utf8, converted_utf8_len, AttachString);
}

Variant f_idn_to_ascii(CStrRef domain, Variant errorcode /* = null */) {
  return php_intl_idn_to(domain, ref(errorcode), INTL_IDN_TO_ASCII);
}

Variant f_idn_to_unicode(CStrRef domain, Variant errorcode /* = null */) {
  return php_intl_idn_to(domain, ref(errorcode), INTL_IDN_TO_UTF8);
}

Variant f_idn_to_utf8(CStrRef domain, Variant errorcode /* = null */) {
  return php_intl_idn_to(domain, ref(errorcode), INTL_IDN_TO_UTF8);
}

///////////////////////////////////////////////////////////////////////////////
}
