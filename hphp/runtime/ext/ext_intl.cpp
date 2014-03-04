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

#include "hphp/runtime/ext/ext_intl.h"
#include "hphp/runtime/base/builtin-functions.h" // throw_expected_*
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/intl-convert.h"
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/base/zend-qsort.h"
#include <unicode/ustring.h>
#include <unicode/ucol.h> // icu
#include <unicode/uclean.h> // icu
#include <unicode/putil.h> // icu
#include <unicode/utypes.h>
#include <unicode/unorm.h>

#include "hphp/system/systemlib.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const int64_t q_Normalizer$$NONE     = UNORM_NONE;
const int64_t q_Normalizer$$FORM_D   = UNORM_NFD;
const int64_t q_Normalizer$$NFD      = UNORM_NFD;
const int64_t q_Normalizer$$FORM_KD  = UNORM_NFKD;
const int64_t q_Normalizer$$NFKD     = UNORM_NFKD;
const int64_t q_Normalizer$$FORM_C   = UNORM_NFC;
const int64_t q_Normalizer$$NFC      = UNORM_NFC;
const int64_t q_Normalizer$$FORM_KC  = UNORM_NFKC;
const int64_t q_Normalizer$$NFKC     = UNORM_NFKC;

///////////////////////////////////////////////////////////////////////////////

c_Normalizer::c_Normalizer(Class* cb) : ExtObjectData(cb) {
}

c_Normalizer::~c_Normalizer() {
}

void c_Normalizer::t___construct() {
}

///////////////////////////////////////////////////////////////////////////////

Variant c_Normalizer::ti_isnormalized(const String& input,
                                      int64_t form /* = q_Normalizer$$FORM_C */) {
  s_intl_error->clearError();

  switch (form) {
  case UNORM_NFD:
  case UNORM_NFKD:
  case UNORM_NFC:
  case UNORM_NFKC:
    break;
  default:
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "normalizer_isnormalized: "
                           "illegal normalization form");
    return uninit_null();
  }

  /* First convert the string to UTF-16. */
  UChar* uinput = NULL; int uinput_len = 0;
  UErrorCode status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&uinput, &uinput_len, input.data(), input.size(),
                             &status);

  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "Error converting string to UTF-16.");
    free(uinput);
    return false;
  }

  /* test string */
  UBool uret = unorm_isNormalizedWithOptions(uinput, uinput_len,
                                             (UNormalizationMode)form,
                                             (int32_t)0, &status);
  free(uinput);

  /* Bail out if an unexpected error occured. */
  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "Error testing if string is the given "
                                   "normalization form.");
    return false;
  }

  return uret;
}

Variant c_Normalizer::ti_normalize(const String& input,
                                   int64_t form /* = q_Normalizer$$FORM_C */) {
  s_intl_error->clearError();

  int expansion_factor = 1;
  switch(form) {
  case UNORM_NONE:
  case UNORM_NFC:
  case UNORM_NFKC:
    break;
  case UNORM_NFD:
  case UNORM_NFKD:
    expansion_factor = 3;
    break;
  default:
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "normalizer_normalize: "
                           "illegal normalization form");
    return uninit_null();
  }

  /* First convert the string to UTF-16. */
  UChar* uinput = NULL; int uinput_len = 0;
  UErrorCode status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&uinput, &uinput_len, input.data(), input.size(),
                             &status);

  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "Error converting string to UTF-16.");
    free(uinput);
    return uninit_null();
  }

  /* Allocate memory for the destination buffer for normalization */
  int uret_len = uinput_len * expansion_factor;
  UChar *uret_buf = (UChar*)malloc((uret_len + 1) * sizeof(UChar));

  /* normalize */
  int size_needed = unorm_normalize(uinput, uinput_len,
                                    (UNormalizationMode)form, (int32_t) 0,
                                    uret_buf, uret_len, &status);

  /* Bail out if an unexpected error occured.
   * (U_BUFFER_OVERFLOW_ERROR means that *target buffer is not large enough).
   * (U_STRING_NOT_TERMINATED_WARNING usually means that the input string
   * is empty).
   */
  if (U_FAILURE(status) &&
      status != U_BUFFER_OVERFLOW_ERROR &&
      status != U_STRING_NOT_TERMINATED_WARNING) {
    free(uret_buf);
    free(uinput);
    return uninit_null();
  }

  if (size_needed > uret_len) {
    /* realloc does not seem to work properly - memory is corrupted
     * uret_buf =  eurealloc(uret_buf, size_needed + 1); */
    free(uret_buf);
    uret_buf = (UChar*)malloc((size_needed + 1) * sizeof(UChar));
    uret_len = size_needed;

    status = U_ZERO_ERROR;

    /* try normalize again */
    size_needed = unorm_normalize( uinput, uinput_len,
                                   (UNormalizationMode)form, (int32_t) 0,
                                   uret_buf, uret_len, &status);

    /* Bail out if an unexpected error occured. */
    if (U_FAILURE(status)) {
      /* Set error messages. */
      s_intl_error->setError(status, "Error normalizing string");
      free(uret_buf);
      free(uinput);
      return uninit_null();
    }
  }

  free(uinput);

  /* the buffer we actually used */
  uret_len = size_needed;

  /* Convert normalized string from UTF-16 to UTF-8. */
  char* ret_buf = NULL; int ret_len = 0;
  intl_convert_utf16_to_utf8(&ret_buf, &ret_len, uret_buf, uret_len, &status);
  free(uret_buf);
  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "normalizer_normalize: "
                                   "error converting normalized text UTF-8");
    return uninit_null();
  }

  return String(ret_buf, ret_len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
