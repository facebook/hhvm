/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/mixed-array-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static __thread GlobalsArray* g_variables;
static __thread EnvConstants* g_envConstants;

GlobalsArray* get_global_variables() {
  assert(g_variables);
  return g_variables;
}

void free_global_variables() {
  g_variables = nullptr;
}

void free_global_variables_after_sweep() {
  g_variables = nullptr;
}

EnvConstants* get_env_constants() {
  assert(g_envConstants);
  return g_envConstants;
}

void EnvConstants::requestInit(EnvConstants* gt) {
  g_envConstants = gt;
}
void EnvConstants::requestExit() {
  g_envConstants = nullptr;
}

GlobalsArray::GlobalsArray(NameValueTable* tab)
  : ArrayData(kGlobalsKind)
  , m_tab(tab)
{
  Variant arr(staticEmptyArray());
#define X(s,v) tab->set(makeStaticString(#s), v.asTypedValue());

  X(argc,                 init_null_variant);
  X(argv,                 init_null_variant);
  X(_SERVER,              arr);
  X(_GET,                 arr);
  X(_POST,                arr);
  X(_COOKIE,              arr);
  X(_FILES,               arr);
  X(_ENV,                 arr);
  X(_REQUEST,             arr);
  X(_SESSION,             arr);
  X(HTTP_RAW_POST_DATA,   init_null_variant);
#undef X

  g_variables = this;
  assert(hasExactlyOneRef());
}

//////////////////////////////////////////////////////////////////////

}
