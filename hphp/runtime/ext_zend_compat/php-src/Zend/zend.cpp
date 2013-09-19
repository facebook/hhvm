#include <Zend/zend_extensions.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>

/* true multithread-shared globals */
ZEND_API zend_class_entry *zend_standard_class_def = NULL;

ZEND_API void zend_make_printable_zval(zval *expr, zval *expr_copy, int *use_copy) {
  if (Z_TYPE_P(expr)==IS_STRING) {
    *use_copy = 0;
    return;
  }
  HPHP::StringData *str = tvCastToString(expr->tv());
  ZVAL_STRING(expr_copy, str->data(), str->size());
  *use_copy = 1;
}

#if defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(DARWIN) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__)
void zend_error_noreturn(int type, const char *format, ...) __attribute__ ((alias("zend_error"),noreturn));
#endif

ZEND_API void zend_error(int type, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  HPHP::raise_message(static_cast<HPHP::ErrorConstants::ErrorModes>(type), format, ap);
  va_end(ap);
}
