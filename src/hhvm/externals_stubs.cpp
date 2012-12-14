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
  SystemScalarArrays::initialize();
  init_builtin_constant_table();
}

Variant invoke_file(CStrRef s,
                    bool once,
                    LVariableTable* variables,
                    const char *currentDir) {
  ASSERT(!variables); // this LVariableTable is unused in HHVM
  {
    Variant r;
    if (eval_invoke_file_hook(r, s, once, variables, currentDir)) {
      return r;
    }
  }
  if (s.empty()) return vm_default_invoke_file(once);
  return throw_missing_file(s.c_str());
}

bool get_call_info(const CallInfo *&ci, void *&extra,
                   const char *s, strhash_t hash) {
  extra = NULL;
  const char *ss = get_renamed_function(s);
  if (ss != s) { s = ss; hash = -1; }
  return get_call_info_builtin(ci, extra, s, hash);
}

bool get_call_info_no_eval(const CallInfo *&ci, void *&extra,
                           const char *s, strhash_t hash) {
  extra = NULL;
  return get_call_info_builtin(ci, extra, s, hash);
}

Variant get_constant(CStrRef name, bool error) {
  return get_builtin_constant(name, error);
}

ObjectData *create_object_only_no_init(CStrRef s, ObjectData* root /* = NULL*/) {
  {
    if (ObjectData* r = eval_create_object_only_hook(s, root)) return r;
  }
  const ObjectStaticCallbacks *cwo = get_builtin_object_static_callbacks(s);
  if (LIKELY(cwo != 0)) return cwo->createOnlyNoInit(root);
  return 0;
}

Object create_object_only(CStrRef s, ObjectData* root /* = NULL*/) {
  ObjectData *obj = create_object_only_no_init(s, root);
  if (UNLIKELY(!obj)) throw_missing_class(s);
  Object r = obj;
  obj->init();
  return r;
}

bool get_call_info_static_method(MethodCallPackage &mcp) {
  StringData *s ATTRIBUTE_UNUSED (mcp.rootCls);
  const ObjectStaticCallbacks *cwo =
    get_builtin_object_static_callbacks(s);
  if (LIKELY(cwo != 0)) {
    return ObjectStaticCallbacks::GetCallInfo(cwo,mcp,-1);
  }
  if (mcp.m_fatal) throw_missing_class(s->data());
  return false;
}

Variant get_class_constant(CStrRef s,
                           const char *constant,
                           int fatal /* = true */) {
  {
    Variant r;
    if (eval_get_class_constant_hook(r, s, constant)) return r;
  }
  {
    const ObjectStaticCallbacks * cwo = get_builtin_object_static_callbacks(s);
    if (cwo) return cwo->os_constant(constant);
  }
  if (fatal > 0) {
    raise_error("Couldn't find constant %s::%s", s.data(), constant);
  } else if (!fatal) {
    raise_warning("Couldn't find constant %s::%s", s.data(), constant);
  }
  return null;
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
