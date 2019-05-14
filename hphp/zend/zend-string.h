/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HPHP_ZEND_ZEND_STRING_H_
#define incl_HPHP_ZEND_ZEND_STRING_H_

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#include <folly/Range.h>

namespace HPHP {
//////////////////////////////////////////////////////////////////////

/**
 * Low-level string functions PHP uses.
 *
 * 1. If a function returns a char *, it has malloc-ed a new string and it's
 *    caller's responsibility to free it.
 *
 * 2. If a function takes "int &len" right after the 1st string parameter, it
 *    is input string's length, and in return, it's return string's length.
 *
 * 3. All functions work with binary strings and all returned strings are
 *    NULL terminated, regardless of whether it's a binary string.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
int string_copy(char *dst, const char *src, int siz);

/**
 * Compare two binary strings.
 */
inline int string_strcmp(const char *s1, int len1, const char *s2, int len2) {
  int minlen = len1 < len2 ? len1 : len2;
  int retval;

  retval = memcmp(s1, s2, minlen);
  if (!retval) {
    return (len1 - len2);
  }

  return (retval > 0) - (retval < 0);
}
/**
 * Compare two binary strings of the first n bytes.
 */
inline int string_strncmp(const char *s1, int len1, const char *s2, int len2,
                          int len) {
  int minlen = len1 < len2 ? len1 : len2;
  int retval;

  if (len < minlen) {
    if (UNLIKELY(len < 0)) len = 0;
    minlen = len;
  }
  retval = memcmp(s1, s2, minlen);
  if (!retval) {
    return (len < len1 ? len : len1) - (len < len2 ? len : len2);
  } else {
    return retval;
  }
}
/**
 * Compare two binary strings of the first n bytes, ignore case.
 */
inline int string_strncasecmp(const char *s1, int len1,
                              const char *s2, int len2, int len) {
  int minlen = len1 < len2 ? len1 : len2;
  int c1, c2;

  if (len < minlen) {
    if (UNLIKELY(len < 0)) len = 0;
    minlen = len;
  }
  while (minlen--) {
    c1 = tolower((int)*(unsigned char *)s1++);
    c2 = tolower((int)*(unsigned char *)s2++);
    if (c1 != c2) {
      return c1 - c2;
    }
  }
  return (len < len1 ? len : len1) - (len < len2 ? len : len2);
}

/**
 * Compare strings.
 */
int string_ncmp(const char *s1, const char *s2, int len);
int string_natural_cmp(char const *a, size_t a_len,
                       char const *b, size_t b_len, int fold_case);


/**
 * Duplicate a binary string. Note that NULL termination is needed even for
 * a binary string, because String class only wraps such a "safe" one that can
 * work with any functions that takes a C-string.
 */
inline char *string_duplicate(const char *s, int len) {
  char *ret = (char *)malloc(len + 1);
  memcpy(ret, s, len);
  ret[len] = '\0';
  return ret;
}

/**
 * Hashing a string.
 */
char *string_rot13(const char *input, int len);
int   string_crc32(const char *p, int len);
char *string_crypt(const char *key, const char *salt);
char *string_sha1(const char *arg, int arg_len, bool raw, int &out_len);

struct Md5Digest {
  Md5Digest(const char* s, int len);
  uint8_t digest[16];
};

std::string string_md5(folly::StringPiece);
std::string string_sha1(folly::StringPiece);

/*
 * Convert input[len] to a malloced, nul-terminated, lowercase, hex string
 */
char *string_bin2hex(const char *input, int &len);
char *string_bin2hex(const char* input, int len, char* output);

//////////////////////////////////////////////////////////////////////
}

#endif
