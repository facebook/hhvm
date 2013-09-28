#include "zend_interfaces.h"

#include "hphp/runtime/base/builtin-functions.h"

ZEND_API zval* zend_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2 TSRMLS_DC) {
  HPHP::String f_name(function_name, function_name_len, HPHP::CopyString);
  HPHP::ArrayInit paramInit(2);
  paramInit.set(0, tvAsVariant(arg1->tv()));
  paramInit.set(1, tvAsVariant(arg2->tv()));
  const HPHP::Array params(paramInit.create());
  HPHP::Variant ret = HPHP::vm_call_user_func(f_name, params);
  auto ref = ret.asRef()->m_data.pref;
  ref->incRefCount();
  *retval_ptr_ptr = ref;
  return ref;
}
