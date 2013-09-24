#include "zend.h"
#include "zend_globals.h"

#include "hphp/util/thread-local.h"

ZEND_API zend_compiler_globals compiler_globals;

static IMPLEMENT_THREAD_LOCAL(_zend_executor_globals, s_zend_executor_globals);

zval& EG_uninitialized_zval() {
  return s_zend_executor_globals.get()->uninitialized_zval;
}
zval*& EG_exception() {
  return s_zend_executor_globals.get()->exception;
}
zval*& EG_prev_exception() {
  return s_zend_executor_globals.get()->prev_exception;
}
HashTable& EG_regular_list() {
  return *s_zend_executor_globals.get()->regular_list;
}
HashTable& EG_persistent_list() {
  return *s_zend_executor_globals.get()->persistent_list;
}
zend_error_handling_t& EG_error_handling() {
  return s_zend_executor_globals.get()->error_handling;
}
HashTable* EG_function_table() {
  return s_zend_executor_globals.get()->function_table;
}
