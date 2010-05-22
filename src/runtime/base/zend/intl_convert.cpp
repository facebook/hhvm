/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/
#include <runtime/base/zend/intl_convert.h>
#include <stdlib.h>

namespace HPHP {

void intl_convert_utf8_to_utf16(UChar** target, int* target_len,
                                const char* src, int  src_len,
                                UErrorCode* status) {
  UChar* dst_buf = NULL;
  int32_t dst_len = 0;

  /* If *target is NULL determine required destination buffer size
   * (pre-flighting). Otherwise, attempt to convert source string;
   * if *target buffer is not large enough it will be resized appropriately.
   */
  *status = U_ZERO_ERROR;

  u_strFromUTF8(*target, *target_len, &dst_len, src, src_len, status);

  if (*status == U_ZERO_ERROR) {
    /* String is converted successfuly */
    (*target)[dst_len] = 0;
    *target_len = dst_len;
    return;
  }

  /* Bail out if an unexpected error occured.
   * (U_BUFFER_OVERFLOW_ERROR means that *target buffer is not large enough).
   * (U_STRING_NOT_TERMINATED_WARNING usually means that the input string
   * is empty).
   */
  if (*status != U_BUFFER_OVERFLOW_ERROR &&
      *status != U_STRING_NOT_TERMINATED_WARNING) {
    return;
  }

  // Allocate memory for the destination buffer (it will be zero-terminated).
  dst_buf = (UChar *)malloc((dst_len + 1) * sizeof(UChar));

  /* Convert source string from UTF-8 to UTF-16. */
  *status = U_ZERO_ERROR;
  u_strFromUTF8(dst_buf, dst_len+1, NULL, src, src_len, status);
  if (U_FAILURE(*status)) {
    free(dst_buf);
    return;
  }

  dst_buf[dst_len] = 0;

  if (*target) free(*target);

  *target = dst_buf;
  *target_len = dst_len;
}

void intl_convert_utf16_to_utf8(char** target, int* target_len,
                                const UChar* src, int  src_len,
                                UErrorCode*  status) {
  char* dst_buf = NULL;
  int32_t dst_len;

  /* Determine required destination buffer size (pre-flighting). */
  *status = U_ZERO_ERROR;
  u_strToUTF8(NULL, 0, &dst_len, src, src_len, status);

  /* Bail out if an unexpected error occured.
   * (U_BUFFER_OVERFLOW_ERROR means that *target buffer is not large enough).
   * (U_STRING_NOT_TERMINATED_WARNING usually means that the input string
   * is empty).
   */
  if (*status != U_BUFFER_OVERFLOW_ERROR &&
      *status != U_STRING_NOT_TERMINATED_WARNING) {
    return;
  }

  // Allocate memory for the destination buffer (it will be zero-terminated).
  dst_buf = (char *)malloc(dst_len + 1);

  /* Convert source string from UTF-16 to UTF-8. */
  *status = U_ZERO_ERROR;
  u_strToUTF8(dst_buf, dst_len, NULL, src, src_len, status);
  if (U_FAILURE(*status)) {
    free(dst_buf);
    return;
  }

  /* U_STRING_NOT_TERMINATED_WARNING is OK for us => reset 'status'. */
  *status = U_ZERO_ERROR;

  dst_buf[dst_len] = 0;
  *target = dst_buf;
  *target_len = dst_len;
}

///////////////////////////////////////////////////////////////////////////////
}
