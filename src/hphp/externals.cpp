/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
//
const char *g_source_root = "";
const char *g_class_map[] = { NULL};
const char *g_source_info[] = { NULL};
const char *g_source_cls2file[] = { NULL};
const char *g_source_func2file[] = { NULL};
const char *g_paramrtti_map[] = { NULL};

Object create_object(const char *s, const Array &params, bool init,
                     ObjectData *root) {
  return Object();
}

Variant invoke(const char* function,
               CArrRef params, int64 hash,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  return true;
}

Variant invoke_static_method(const char* cls, MethodIndex methodIndex,
                             const char *function,
                             CArrRef params, bool fatal /* = true */) {
  return null;
}

Variant invoke_static_method_mil(const char* cls,
                                 const char *function,
                                 CArrRef params, bool fatal /* = true */) {
  return null;
}

class GlobalVariables;
GlobalVariables *get_global_variables() { return NULL;}
class LVariableTable;
LVariableTable *get_variable_table() { return NULL;}
class Globals;
Globals *get_globals() { return NULL; }
class SystemGlobals;
SystemGlobals *get_system_globals() { return NULL;}
void init_global_variables() {}
void init_static_variables() {}
void free_global_variables() {}
Array get_global_state() { return Array(); }

Variant invoke_file(CStrRef path, bool once /* = false */,
                    LVariableTable* variables /* = NULL */,
                    const char *currDir /* = NULL */) {
  return false;
}

Variant get_constant(CStrRef name) { return name;}
Variant get_builtin_constant(CStrRef name) { return name;}
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
namespace Eval {
Variant invoke_from_eval(const char *function, VariableEnvironment &env,
                         const FunctionCallExpression *caller,
                         int64 hash /* = -1 */, bool fatal /* = true */) {
  return Variant();
}

Variant invoke_from_eval(const char *function, VariableEnvironment &env,
                         const FunctionCallExpression *caller,
                         bool fatal /* = true */) {
  return Variant();
}
}

// Class Invoke Tables dummies
extern const MethodIndex methodIndexMapInit[];
const MethodIndex methodIndexMapInit[] = {MethodIndex(0,0)};
const char * methodIndexMapInitName[] = {NULL};

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

///////////////////////////////////////////////////////////////////////////////
}
