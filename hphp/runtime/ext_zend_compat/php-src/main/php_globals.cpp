#include "main/php_globals.h"

struct _php_core_globals {
  const zval *http_globals[6];
};

static IMPLEMENT_THREAD_LOCAL(_php_core_globals, s_php_core_globals);
const zval** PG_http_globals() {
  auto* globals = HPHP::get_global_variables();
  auto* tl_globals = s_php_core_globals.get()->http_globals;
  tl_globals[TRACK_VARS_POST] = globals->get(s__POST).asRef()->m_data.pref;
  tl_globals[TRACK_VARS_GET] = globals->get(s__GET).asRef()->m_data.pref;
  tl_globals[TRACK_VARS_COOKIE] = globals->get(s__COOKIE).asRef()->m_data.pref;
  tl_globals[TRACK_VARS_SERVER] = globals->get(s__SERVER).asRef()->m_data.pref;
  tl_globals[TRACK_VARS_ENV] = globals->get(s__ENV).asRef()->m_data.pref;
  tl_globals[TRACK_VARS_FILES] = globals->get(s__FILES).asRef()->m_data.pref;
  tl_globals[TRACK_VARS_REQUEST] = globals->get(s__REQUEST).asRef()->m_data.pref;

  tl_globals[TRACK_VARS_POST]->incRefCount();
  tl_globals[TRACK_VARS_GET]->incRefCount();
  tl_globals[TRACK_VARS_COOKIE]->incRefCount();
  tl_globals[TRACK_VARS_SERVER]->incRefCount();
  tl_globals[TRACK_VARS_ENV]->incRefCount();
  tl_globals[TRACK_VARS_FILES]->incRefCount();
  tl_globals[TRACK_VARS_REQUEST]->incRefCount();
  return tl_globals;
}

_arg_separators PG_arg_separator() {
  _arg_separators seps;
  seps.input = "&";
  seps.output = "&";
  return seps;
}
