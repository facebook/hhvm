#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"
// Zend has a very specific include order, so I can't do this class' header
// before zend.h sadly
#include "hphp/runtime/ext_zend_compat/php-src/main/php_globals.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"

static IMPLEMENT_THREAD_LOCAL(_php_core_globals, s_php_core_globals);

BEGIN_EXTERN_C()
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

static bool bool_ini_get(const char* str) {
  HPHP::String ret;
  HPHP::IniSetting::Get(str, ret);
  return ret.toBoolean();
}

zend_bool PG_html_errors() { return bool_ini_get("html_errors"); }
zend_bool PG_display_errors() { return bool_ini_get("display_errors"); }
zend_bool PG_track_errors() { return bool_ini_get("track_errors"); }
zend_bool PG_log_errors() { return bool_ini_get("log_errors"); }

_arg_separators PG_arg_separator() {
  _arg_separators seps;
  seps.input = "&";
  seps.output = "&";
  return seps;
}

int64_t PG_memory_limit() {
  HPHP::String ret;
  HPHP::IniSetting::Get("memory_limit", ret);
  return ret.toInt64();
}

char* PG_docref_root() { return nullptr; }
char* PG_docref_ext() { return nullptr; }
zend_bool PG_during_request_startup() { return false; }
int64_t PG_log_errors_max_len() { return 1024; }
const char* PG_last_error_message() {
  return HPHP::g_context->getLastError().data();
}
const char* PG_last_error_file() {
  return HPHP::g_context->getLastErrorPath().data();
}
int PG_last_error_type() { return HPHP::g_context->getLastErrorNumber(); }
int PG_last_error_lineno() { return HPHP::g_context->getLastErrorLine(); }

END_EXTERN_C()
