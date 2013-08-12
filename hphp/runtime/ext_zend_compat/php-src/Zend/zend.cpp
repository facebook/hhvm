#include <Zend/zend_extensions.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>

zval zval_used_for_init;

ZEND_API void zend_make_printable_zval(zval *expr, zval *expr_copy, int *use_copy) {
	if (Z_TYPE_P(expr)==IS_STRING) {
    *use_copy = 0;
    return;
  }
  HPHP::StringData *str = tvCastToString(expr);
  ZVAL_STRING(expr_copy, str->data(), str->size());
  *use_copy = 1;
}
