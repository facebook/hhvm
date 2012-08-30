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

#include <runtime/base/complex_types.h>
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/externals.h>

using std::pair;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *g_source_root = "";
const char *g_class_map[] = { NULL};
const char *g_source_info[] = { NULL};
const char *g_source_cls2file[] = { NULL};
const char *g_source_func2file[] = { NULL};
const char *g_paramrtti_map[] = { NULL};

Object create_object_only(CStrRef s, ObjectData *root) {
  return Object();
}
ObjectData *create_object_only_no_init(CStrRef s, ObjectData *root) {
  return 0;
}
class GlobalVariables;
GlobalVariables *get_global_variables() { return NULL;}
GlobalVariables *get_global_variables_check() { return NULL;}
class LVariableTable;
LVariableTable *get_variable_table() { return NULL;}
class Globals;
Globals *get_globals() { return NULL; }
class SystemGlobals;
SystemGlobals *get_system_globals() { return NULL;}
void init_global_variables() {}
void init_literal_varstrings() {}
void init_builtin_constant_table() {}
void init_constant_table() {}
void init_static_variables() {}
void free_global_variables() {}
void free_global_variables_after_sweep() {}
Array get_global_state() { return Array(); }

Variant invoke_file(CStrRef path, bool once /* = false */,
                    LVariableTable* variables /* = NULL */,
                    const char *currDir /* = NULL */) {
  return false;
}
bool hphp_could_invoke_file(CStrRef s, void*) { return false; }

Variant get_constant(CStrRef name, bool error) { return name;}
Variant get_builtin_constant(CStrRef name, bool error) { return name;}
ConstantType check_constant(CStrRef name) { return NoneBuiltinConstant;}
Variant get_class_constant(CStrRef s, const char *prop,
                           int fatal /* = true */) {
  return null;
}

Variant get_static_property(CStrRef s, const char *prop) {
  return Variant();
}
Variant *get_static_property_lv(CStrRef s, const char *prop) {
  return NULL;
}

Variant get_class_var_init(CStrRef s, const char *var) {
  return null;
}

Array get_global_array_wrapper() {
  return Array();
}

bool get_call_info(const CallInfo *&ci, void *&extra,
    const char *s, strhash_t hash) {
  return false;
}
bool get_call_info_no_eval(const CallInfo *&ci, void *&extra,
    const char *s, strhash_t hash) {
  return false;
}
bool get_call_info_static_method(MethodCallPackage &info) {
  return false;
}
const ObjectStaticCallbacks * get_object_static_callbacks(CStrRef s) {
  return NULL;
}

bool has_eval_support = false;

HphpBinary::Type getHphpBinaryType() {
  return HphpBinary::hphpc;
}

#ifdef HHVM
const long long hhbc_ext_funcs_count = 0;
const HhbcExtFuncInfo hhbc_ext_funcs[] = {};
const long long hhbc_ext_class_count = 0;
const HhbcExtClassInfo hhbc_ext_classes[] = {};
#endif
///////////////////////////////////////////////////////////////////////////////
}
