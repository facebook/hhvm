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
#include "runtime/eval/eval.h"
#include "runtime/base/externals.h"

/*
 * HHVM's stubs for various hphpc-style externals.h functions.
 *
 * These exist for legacy reasons---much of the runtime assumed
 * generated versions of some of these functions would exist (as
 * output from hphpc).
 *
 * In the VM, we still fake a few of them while phasing this out.
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

HphpBinary::Type getHphpBinaryType() { return HphpBinary::hhvm; }

void init_static_variables() {
  init_builtin_constant_table();
}

Variant invoke_file(CStrRef s,
                    bool once,
                    LVariableTable* variables,
                    const char *currentDir) {
  assert(!variables); // this LVariableTable is unused in HHVM
  {
    Variant r;
    if (eval_invoke_file_hook(r, s, once, variables, currentDir)) {
      return r;
    }
  }
  if (s.empty()) return vm_default_invoke_file(once);
  return throw_missing_file(s.c_str());
}

Variant get_constant(CStrRef name, bool error) {
  return get_builtin_constant(name, error);
}

Object create_object_only(CStrRef s, ObjectData* root /* = NULL*/) {
  ObjectData *obj = eval_create_object_only_hook(s, root);
  if (UNLIKELY(!obj)) throw_missing_class(s);
  Object r = obj;
  obj->init();
  return r;
}

void init_literal_varstrings() {
  extern void sys_init_literal_varstrings();
  sys_init_literal_varstrings();
}

//////////////////////////////////////////////////////////////////////

const char *g_source_cls2file[] = {};
const char *g_source_func2file[] = {};
const char *g_paramrtti_map[] = {};
const char *g_source_root = "";
const char *g_source_info[] = {};

bool has_eval_support = true;

//////////////////////////////////////////////////////////////////////

}
