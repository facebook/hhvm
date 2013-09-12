#include "hphp/runtime/ext_zend_compat/php-src/main/php.h"
#include "hphp/runtime/base/zend-printf.h"

PHPAPI int vspprintf(char **pbuf, size_t max_len, const char *format, va_list ap) {
  int ret = HPHP::vspprintf(pbuf, max_len, format, ap);

  // *pbuf is a malloc()ed buf, but we need it emalloc()ed, *sigh*
  char* emalloced_buf = (char*) emalloc(ret);
  memcpy(emalloced_buf, *pbuf, ret);
  free(*pbuf);
  *pbuf = emalloced_buf;
  return ret;
}

PHPAPI int spprintf( char **pbuf, size_t max_len, const char *format, ...) {
  int cc;
  va_list ap;
  va_start(ap, format);
  cc = vspprintf(pbuf, max_len, format, ap);
  va_end(ap);
  return (cc);
}
