#include "zend.h"
#include "zend_globals.h"

#include "hphp/runtime/base/externals.h"
#include "hphp/util/thread-local.h"

ZEND_API zend_compiler_globals compiler_globals;

static IMPLEMENT_THREAD_LOCAL(_zend_executor_globals, s_zend_executor_globals);

#define G(TYPE, MEMBER)                                   \
  std::add_lvalue_reference<TYPE>::type EG_ ## MEMBER() { \
    return s_zend_executor_globals.get()->MEMBER;         \
  }
EG_DEFAULT
#undef G

HashTable& EG_regular_list() {
  return *s_zend_executor_globals.get()->regular_list;
}
HashTable& EG_persistent_list() {
  return *s_zend_executor_globals.get()->persistent_list;
}

HashTable& EG_symbol_table() {
  return *HPHP::get_global_variables()->asArrayData();
}
int& EG_exit_status() { return HPHP::ExitException::ExitCode; }
