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
/*
 * Global variables in HHVM are currently partially using a legacy
 * hphpc-style implementation, even though it's mostlly using a new
 * system (in VM::VarEnv).
 *
 * We need to remove GV()-style accesses to super globals from
 * extensions in order to fully remove this.  TODO(#1158383)
 */

class GlobalVariables : public SystemGlobals {
DECLARE_SMART_ALLOCATION(GlobalVariables);
public:
  GlobalVariables() {
    memset(&tgv_bool, 0, sizeof(tgv_bool));
    memset(&tgv_int, 0, sizeof(tgv_int));
    memset(&tgv_int64, 0, sizeof(tgv_int64));
    memset(&tgv_double, 0, sizeof(tgv_double));
    memset(&tgv_RedeclaredCallInfoConstPtr, 0,
      sizeof(tgv_RedeclaredCallInfoConstPtr));
  }

  RedeclaredCallInfoConst* tgv_RedeclaredCallInfoConstPtr[1];
  RedeclaredObjectStaticCallbacksConst* tgv_RedeclaredObjectStaticCallbacksConstPtr[1];
  Variant tgv_Variant[1];
  bool tgv_bool[1];
  #define run_pm_php$hhvm_php tgv_bool[0]
  double tgv_double[1];
  int tgv_int[1];
  int64 tgv_int64[1];

  // LVariableTable methods.  These are not actually called in the VM.
  ssize_t staticSize() const { NOT_REACHED(); }
  CVarRef getRefByIdx(ssize_t idx, Variant& k) { NOT_REACHED(); }
  ssize_t getIndex(const char* s, strhash_t prehash) const { NOT_REACHED(); }
  Variant& getImpl(CStrRef s) { NOT_REACHED(); }
  bool exists(CStrRef s) const { NOT_REACHED(); }
};

IMPLEMENT_SMART_ALLOCATION(GlobalVariables)

//////////////////////////////////////////////////////////////////////

static __thread GlobalVariables* g_variables;

void init_global_variables() {
  GlobalVariables* g = get_global_variables_check();
  ThreadInfo::s_threadInfo->m_globals = g;
  g->initialize();
}

GlobalVariables* get_global_variables() {
  ASSERT(g_variables);
  return g_variables;
}

GlobalVariables* get_global_variables_check() {
  if (!g_variables) g_variables = NEW(GlobalVariables)();
  return g_variables;
}

void free_global_variables() {
  if (g_variables) DELETE(GlobalVariables)(g_variables);
  g_variables = NULL;
}

void free_global_variables_after_sweep() {
  g_variables = NULL;
}

Globals*        get_globals()        { return get_global_variables(); }
SystemGlobals*  get_system_globals() { return get_global_variables(); }
LVariableTable* get_variable_table() { return nullptr; }

//////////////////////////////////////////////////////////////////////

}

