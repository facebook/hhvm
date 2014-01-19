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

#ifndef incl_HPHP_FILEINFO_COMPAT_H_
#define incl_HPHP_FILEINFO_COMPAT_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/zend-printf.h"

#define PCRE_CASELESS 0x00000001
#define PCRE_MULTILINE 0x00000002
#define emalloc HPHP::smart_malloc
#define ecalloc HPHP::smart_calloc
#define efree HPHP::smart_free
#define erealloc HPHP::smart_realloc
#define php_stream HPHP::File

inline char *estrndup(const char *s, unsigned int length) {
  char* ret = (char*) emalloc(length + 1);
  memcpy(ret, s, length);
  ret[length] = '\0';
  return ret;
}

inline int vspprintf(char **pbuf, size_t max_len, const char *format, va_list ap) {
  int ret = HPHP::vspprintf_ap(pbuf, max_len, format, ap);

  // *pbuf is a malloc()ed buf, but we need it emalloc()ed, *sigh*
  char* emalloced_buf = estrndup(*pbuf, ret);
  free(*pbuf);
  *pbuf = emalloced_buf;
  return ret;
}

inline int spprintf(char **pbuf, size_t max_len, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int cc = vspprintf(pbuf, max_len, format, ap);
  va_end(ap);
  return cc;
}

#ifndef HAVE_STRLCPY
size_t php_strlcpy(char *dst, const char *src, size_t siz);
#undef strlcpy
#define strlcpy php_strlcpy
#endif

#endif // incl_HPHP_FILEINFO_COMPAT_H_
