#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"
BEGIN_EXTERN_C()
#include "hphp/runtime/ext_zend_compat/php-src/main/php_globals.h"
END_EXTERN_C()
#include "hphp/runtime/ext_hhvm/ext_zend_compat.h"

static IMPLEMENT_THREAD_LOCAL(_php_core_globals, s_php_core_globals);
zval** PG_http_globals() {
  auto* globals = HPHP::get_global_variables();
  auto* tl_globals = s_php_core_globals.get()->http_globals;

  auto copyEnv = [&](int zval_key, const HPHP::StaticString& global_key) {
    auto v = globals->get(global_key).asTypedValue();
    HPHP::zBoxAndProxy(const_cast<HPHP::TypedValue*>(v));
    tl_globals[zval_key] = v->m_data.pref;
    tl_globals[zval_key]->incRefCount();
  };

  copyEnv(TRACK_VARS_POST, s__POST);
  copyEnv(TRACK_VARS_GET, s__GET);
  copyEnv(TRACK_VARS_COOKIE, s__COOKIE);
  copyEnv(TRACK_VARS_SERVER, s__SERVER);
  copyEnv(TRACK_VARS_ENV, s__ENV);
  copyEnv(TRACK_VARS_FILES, s__FILES);

  return tl_globals;
}

_arg_separators PG_arg_separator() {
  _arg_separators seps;
  seps.input = "&";
  seps.output = "&";
  return seps;
}
