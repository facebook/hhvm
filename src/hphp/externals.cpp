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

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *g_source_root = "";
const char *g_class_map[] = { NULL};
const char *g_source_info[] = { NULL};
const char *g_source_cls2file[] = { NULL};
const char *g_source_func2file[] = { NULL};
const char *g_paramrtti_map[] = { NULL};

Object create_object_only(const char *s, ObjectData *root) {
  return Object();
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
Array get_global_state() { return Array(); }

Variant invoke_file(CStrRef path, bool once /* = false */,
                    LVariableTable* variables /* = NULL */,
                    const char *currDir /* = NULL */) {
  return false;
}

Variant get_constant(CStrRef name, bool error) { return name;}
Variant get_builtin_constant(CStrRef name, bool error) { return name;}
Variant get_class_constant(const char *s, const char *prop,
                           bool fatal /* = true */) {
  return null;
}

Variant get_static_property(const char *s, const char *prop) {
  return Variant();
}
Variant *get_static_property_lv(const char *s, const char *prop) {
  return NULL;
}

Variant get_class_var_init(const char *s, const char *var) {
  return null;
}

Array get_global_array_wrapper() {
  return Array();
}

bool get_call_info(const CallInfo *&ci, void *&extra, 
    const char *s, int64 hash) {
  return false;
}
bool get_call_info_static_method(MethodCallPackage &info) {
  return false;
}
bool get_call_info_static_method_with_index(MethodCallPackage &info,
    MethodIndex mi) {
  return false;
}

const ObjectStaticCallbacks * get_object_static_callbacks(const char *s) {
  return NULL;
}

void fiber_marshal_global_state(GlobalVariables *g1, GlobalVariables *g2,
                                FiberReferenceMap &refMap) {
}

void fiber_unmarshal_global_state(GlobalVariables *g1, GlobalVariables *g2,
                                  FiberReferenceMap &refMap,
                                  char defstrategy,
                                  const vector<pair<string, char> > &resolver){
}
extern const MethodIndex g_methodIndexMapInit[];
const MethodIndex g_methodIndexMapInit[] = {MethodIndex(0,0)};
const char * g_methodIndexMapInitName[] = {NULL};
extern const MethodIndex g_methodIndexMapInitSys[];
extern const MethodIndex g_methodIndexMapInitNameSys[];

const unsigned g_methodIndexHMapSize = 0;
const MethodIndexHMap g_methodIndexHMap[] = {MethodIndexHMap()};
const unsigned g_methodIndexReverseCallIndex[] = {0};
const char * g_methodIndexReverseIndex[] = {0};

bool has_eval_support = false;
///////////////////////////////////////////////////////////////////////////////
}
