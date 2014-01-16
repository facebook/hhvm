/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_UTIL_ZEND_STRING_H_
#define incl_HPHP_UTIL_ZEND_STRING_H_

#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <string>

namespace HPHP {
//////////////////////////////////////////////////////////////////////

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
std::string string_md5(const char* s, int len);

/*
 * Convert input[len] to a malloced, nul-terminated, lowercase, hex string
 */
char *string_bin2hex(const char *input, int &len);
char *string_bin2hex(const char* input, int len, char* output);

//////////////////////////////////////////////////////////////////////
}

#endif
