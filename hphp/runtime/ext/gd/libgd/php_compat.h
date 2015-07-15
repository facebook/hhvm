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

#ifndef incl_HPHP_LIBGD_COMPAT_H_
#define incl_HPHP_LIBGD_COMPAT_H_

// I died a little inside writing this, but we have to end the extern "C"
}

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-php-config.h"
#include "hphp/util/string-vsnprintf.h"

// And start the blasted C stuff again
extern "C" {

#define HAVE_LIBJPEG
#define HAVE_LIBPNG
#define emalloc HPHP::req::malloc
#define ecalloc HPHP::req::calloc
#define efree HPHP::req::free
#define erealloc HPHP::req::realloc
#define vspprintf HPHP::vspprintf
#define TSRMLS_CC
#define TSRMLS_FETCH()
#define MAXPATHLEN PATH_MAX
#define pemalloc(size, persistent) ((persistent)?malloc(size):emalloc(size))
#define pefree(ptr, persistent)  ((persistent)?free(ptr):efree(ptr))
#define pestrdup(s, persistent) ((persistent)?strdup(s):estrdup(s))
#define VCWD_GETCWD(buff, size) getcwd(buff, size)

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset) {
  return nmemb * size + offset;
}

inline void *safe_emalloc(size_t nmemb, size_t size, size_t offset) {
 return emalloc(safe_address(nmemb, size, offset));
}

inline char *estrndup(const char *s, unsigned int length) {
  char* ret = (char*) emalloc(length + 1);
  memcpy(ret, s, length);
  ret[length] = '\0';
  return ret;
}

inline char *estrdup(const char *s) {
  return estrndup(s, strlen(s));
}

#define E_ERROR        (1<<0L)
#define E_WARNING      (1<<1L)
#define E_NOTICE      (1<<3L)
inline void php_verror(const char *docref, const char *params, int type,
                       const char *format, va_list args) {
  std::string msg;
  HPHP::string_vsnprintf(msg, format, args);

  if (type == E_ERROR) {
    return HPHP::raise_error(msg);
  } else if (type == E_WARNING) {
    return HPHP::raise_warning(msg);
  } else if (type == E_NOTICE) {
    return HPHP::raise_notice(msg);
  }
  not_reached();
}
inline void php_error_docref(const char *docref, int type,
                             const char *format, ...) {
  va_list args;
  va_start(args, format);
  php_verror(docref, "", type, format, args);
  va_end(args);
}

// Force gdhelpers.h to run with thread safety.
#define ZTS
#define MUTEX_T pthread_mutex_t

// This abomination is required because of what happens in gdhelpers.h.
// They steal (x) away from us, so we just looked up the one place
// which used it and hard-coded it in here.  Yep.
#define tsrm_mutex_alloc(x) gdFontCacheMutex; \
    pthread_mutex_init(&gdFontCacheMutex, 0)

#define tsrm_mutex_free(x) pthread_mutex_destroy(&x)
#define tsrm_mutex_lock(x) pthread_mutex_lock(&x)
#define tsrm_mutex_unlock(x) pthread_mutex_unlock(&x)

// Double definition with libjpeg, which can affect PCH builds
#undef MAXJSAMPLE

// And double definition with libxml2
#undef ESC

#endif // incl_HPHP_LIBGD_COMPAT_H_
