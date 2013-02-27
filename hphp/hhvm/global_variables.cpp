/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/base/hphp.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static __thread GlobalVariables* g_variables;

void init_global_variables() {
  always_assert(false);
}

GlobalVariables* get_global_variables() {
  assert(g_variables);
  return g_variables;
}

void free_global_variables() {
  g_variables = nullptr;
}

void free_global_variables_after_sweep() {
  g_variables = nullptr;
}

SystemGlobals*  get_system_globals() { return get_global_variables(); }
LVariableTable* get_variable_table() { return nullptr; }

VM::GlobalNameValueTableWrapper::GlobalNameValueTableWrapper(
  NameValueTable* tab) : NameValueTableWrapper(tab) {

  VarNR arr(HphpArray::GetStaticEmptyArray());
#define X(s,v)                                          \
  tab->migrateSet(StringData::GetStaticString(#s),      \
                  gvm_##s.asTypedValue());              \
  gvm_##s.v;

  X(argc,                 setNull());
  X(argv,                 setNull());
  X(_SERVER,              assignVal(arr));
  X(_GET,                 assignVal(arr));
  X(_POST,                assignVal(arr));
  X(_COOKIE,              assignVal(arr));
  X(_FILES,               assignVal(arr));
  X(_ENV,                 assignVal(arr));
  X(_REQUEST,             assignVal(arr));
  X(_SESSION,             assignVal(arr));
  X(HTTP_RAW_POST_DATA,   setNull());
  X(http_response_header, setNull());
#undef X

  ThreadInfo::s_threadInfo->m_globals = g_variables = this;
}

//////////////////////////////////////////////////////////////////////

}

