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

#include <runtime/base/class_info.h>
#include <runtime/base/complex_types.h>
#include <runtime/ext/ext_string.h>
#include <runtime/ext/ext_network.h>
#include <runtime/ext/ext_soap.h>
#include <runtime/base/program_functions.h>
#include <system/gen/sys/system_globals.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// These are normally code-generated and we are implementing them here
// purely for unit testing.

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const MethodIndex g_methodIndexMapInit[];
const MethodIndex g_methodIndexMapInit[] = {MethodIndex(0,0)};
const char * g_methodIndexMapInitName[] = {NULL};


const unsigned g_methodIndexHMapSize = 0;
const MethodIndexHMap g_methodIndexHMap[] = {MethodIndexHMap()};
const unsigned g_methodIndexReverseCallIndex[] = {0};
const char * g_methodIndexReverseIndex[] = {0};

// Used by test_ext_preg
static String test_preg_rep(CStrRef a, CStrRef b, CStrRef c) {
  return concat(f_strtoupper(c), a);
}

#define CLASS_INFO_EMPTY_ENTRY   "", NULL, NULL, NULL
#define METHOD_INFO_EMPTY_ENTRY  NULL, NULL, NULL, NULL, NULL, NULL, NULL

const char *g_class_map[] = {
  /* header */ (const char *)ClassInfo::IsSystem,
  NULL, CLASS_INFO_EMPTY_ENTRY,
  /* interfaces */
  NULL,
  /* methods    */
  (const char *)ClassInfo::IsPublic, "xbox_process_message",
  METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "test",        METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "lower",       METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "sumlen_step", METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "sumlen_fini", METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "hello",       METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "sub",         METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "add",         METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "sum",         METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "fault",       METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPublic, "strlen",      METHOD_INFO_EMPTY_ENTRY,
  NULL,
  /* properties */
  NULL,
  /* constants */
  NULL,

  /* header */ (const char *)ClassInfo::IsNothing,
  NULL, CLASS_INFO_EMPTY_ENTRY,
  /* interfaces */
  NULL,
  /* methods    */
  NULL,
  /* properties */
  NULL,
  /* constants */
  NULL,

  /* header */ (const char *)ClassInfo::IsSystem,
  "test", CLASS_INFO_EMPTY_ENTRY,
  /* interfaces */
  "itestable", NULL,
  /* methods    */
  (const char *)ClassInfo::IsPublic,    "foo",  METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsProtected, "func", METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPrivate,   "bar",  METHOD_INFO_EMPTY_ENTRY,
  NULL,
  /* properties */
  (const char *)ClassInfo::IsPublic,    "foo",
  (const char *)ClassInfo::IsProtected, "prop",
  (const char *)ClassInfo::IsPrivate,   "bar",
  NULL,
  /* constants */
  "const_foo", (const char*)ClassInfo::IsInterface, "s:1:\"f\";",
  NULL,

  /* header */ (const char *)ClassInfo::IsInterface,
  "itestable", CLASS_INFO_EMPTY_ENTRY,
  /* interfaces */
  NULL,
  /* methods    */
  (const char *)ClassInfo::IsPublic,  "foo", METHOD_INFO_EMPTY_ENTRY,
  (const char *)ClassInfo::IsPrivate, "bar", METHOD_INFO_EMPTY_ENTRY,
  NULL,
  /* properties */
  NULL,
  /* constants */
  NULL,

  NULL
};

const char *g_source_root = "";
const char *g_source_info[] = { NULL};
const char *g_source_cls2file[] = { "test", "test_file", 0, NULL};
const char *g_source_func2file[] = { NULL};
const char *g_paramrtti_map[] = { NULL};

Variant get_class_var_init(const char *s, const char *var) {
  return null;
}

Object create_object_only(const char *s, ObjectData *root /* = NULL */) {
  return create_builtin_object_only(s, root);
}

Variant invokeImpl(void *extra, CArrRef params) {
  const char *function = (const char*)extra;
  // for TestExtFunction
  if (strcasecmp(function, "test") == 0) {
    return params[0];
  }

  // for TestExtPreg::test_preg_replace_callback
  if (strcasecmp(function, "next_year") == 0) {
    Array matches = params[0].toArray();
    return matches[1].toString() + String(matches[2].toInt32() + 1);
  }

  // for TestExtArray::test_array_filter
  if (strcasecmp(function, "odd") == 0) {
    return params[0].toInt32() & 1;
  }
  if (strcasecmp(function, "even") == 0) {
    return !(params[0].toInt32() & 1);
  }

  // for TestExtArray::test_array_map
  if (strcasecmp(function, "cube") == 0) {
    int n = params[0].toInt32();
    return n * n * n;
  }

  // for TestExtArray::test_array_multisort
  if (strcasecmp(function, "strtolower") == 0) {
    return f_strtolower(params[0]);
  }

  // for TestExtArray::test_array_reduce
  if (strcasecmp(function, "rsum") == 0) {
    int v = params[0].toInt32();
    int w = params[1].toInt32();
    v += w;
    return v;
  }
  if (strcasecmp(function, "rmul") == 0) {
    int v = params[0].toInt32();
    int w = params[1].toInt32();
    v *= w;
    return v;
  }

  // for TestExtArray::test_array_walk_recursive
  if (strcasecmp(function, "test_print") == 0) {
    String item = params[0].toString();
    String key = params[1].toString();
    echo(key + ": " + item + "\n");
  }

  // for TestExtArray::test_array_walk
  if (strcasecmp(function, "test_alter") == 0) {
    Variant &item1 = lval(((Array&)params).lvalAt(0));
    String key = params[1];
    String prefix = params[2];
    item1 = prefix + ": " + item1;
  }

  // for TestExtArray::test_array_udiff
  if (strcasecmp(function, "comp_func") == 0) {
    int n1 = params[0].toInt32();
    int n2 = params[1].toInt32();
    if (n1 == n2) return 0;
    return n1 > n2 ? 1 : -1;
  }

  // for TestExtArray::test_usort
  if (strcasecmp(function, "reverse_comp_func") == 0) {
    int n1 = params[0].toInt32();
    int n2 = params[1].toInt32();
    if (n1 == n2) return 0;
    return n1 > n2 ? -1 : 1;
  }

  // for TestExtArray::test_array_uintersect
  if (strcasecmp(function, "strcasecmp") == 0) {
    String s1 = params[0].toString();
    String s2 = params[1].toString();
    return strcasecmp(s1.data(), s2.data());
  }

  // for TestExtArray::test_uasort
  if (strcasecmp(function, "reverse_strcasecmp") == 0) {
    String s1 = params[0].toString();
    String s2 = params[1].toString();
    return strcasecmp(s2.data(), s1.data());
  }

  // for TestExtFbml
  if (strcasecmp(function, "urltr") == 0) {
    String s1 = params[0].toString();
    String s2 = params[1].toString();
    return String("url:") + s1 + "=" + s2;
  }

  // for TestExtCurl::test_curl_exec
  if (strcasecmp(function, "curl_write_func") == 0) {
    print("curl_write_func called with ");
    print(params[1]);
    return params[1].toString().size();
  }

  // for TestExtPreg::test_preg_replace
  if (strcasecmp(function, "strtoupper") == 0) {
    return f_strtoupper(params[0].toString());
  }
  if (strcasecmp(function, "test_preg_rep") == 0) {
    return test_preg_rep(params[0].toString(), params[1].toString(),
                       params[2].toString());
  }
  if (strcasecmp(function, "sprintf") == 0) {
    return f_sprintf(params.size(), params[0],
                     params.slice(1, params.size() - 1, false));
  }

  // for TestExtSqlite3::test_sqlite3
  if (strcasecmp(function, "lower") == 0) {
    return f_strtolower(params[0]);
  }
  if (strcasecmp(function, "sumlen_step") == 0) {
    return params[0].toInt64() + f_strlen(params[2]);
  }
  if (strcasecmp(function, "sumlen_fini") == 0) {
    return params[0].toInt64();
  }

  // for TestExtSoap
  if (strcasecmp(function, "hello") == 0) {
    return "Hello World";
  }
  if (strcasecmp(function, "add") == 0) {
    return params[0].toInt32() + params[1].toInt32();
  }
  if (strcasecmp(function, "sub") == 0) {
    return params[0].toInt32() - params[1].toInt32();
  }
  if (strcasecmp(function, "sum") == 0) {
    int sum = 0;
    for (ArrayIter iter(params[0]); iter; ++iter) {
      sum += iter.second().toInt32();
    }
    return sum;
  }
  if (strcasecmp(function, "strlen") == 0) {
    return f_strlen(params[0]);
  }
  if (strcasecmp(function, "fault") == 0) {
    return Object((NEWOBJ(c_SoapFault)())->create("MyFault","My fault string"));
  }

  // for TestExtServer
  if (strcasecmp(function, "xbox_process_message") == 0) {
    return StringUtil::Reverse(params[0]);
  }

  return true;
}
CallInfo invokeImplCallInfo((void*)invokeImpl, NULL, 0, CallInfo::VarArgs, 0);
bool get_call_info(const CallInfo *&ci, void *&extra, const char *s,
    int64 hash /* = -1 */) {
  extra = (void*)s;
  ci = &invokeImplCallInfo;
  return true;
}

void init_static_variables() { SystemScalarArrays::initialize();}

class GlobalVariables : public SystemGlobals {};
static IMPLEMENT_THREAD_LOCAL(GlobalVariables, g_variables);
GlobalVariables *get_global_variables_check() { return NULL;}
GlobalVariables *get_global_variables() { return g_variables.get();}
LVariableTable *get_variable_table() { return g_variables.get(); }
Globals *get_globals() { return g_variables.get(); }
SystemGlobals *get_system_globals() { return g_variables.get(); }
void init_global_variables() {
  ThreadInfo::s_threadInfo->m_globals = get_global_variables();
  GlobalVariables::initialize();
}
void free_global_variables() { g_variables.destroy();}
void init_literal_varstrings() {}
bool has_eval_support = true;
Variant invoke_file(CStrRef path, bool once /* = false */,
                    LVariableTable* variables /* = NULL */,
                    const char *currentDir /* = NULL */) {
  String cmd = canonicalize_path(path, "", 0);
  if (path == "string") {
    echo("Hello, world!");
    return true;
  }
  if (cmd == "pageletserver") {
    SystemGlobals *g = (SystemGlobals*)get_global_variables();

    echo("pagelet postparam: ");
    echo(g->GV(HTTP_RAW_POST_DATA));
    echo("pagelet getparam: ");
    echo(g->GV(_GET)["getparam"]);
    echo("pagelet header: ");
    echo(g->GV(_SERVER)["HTTP_MYHEADER"]);
    f_header("ResponseHeader: okay");

    sleep(1); // give status check time to happen
    return true;
  }
  return false;
}

Variant get_static_property(const char *s, const char *prop) {
  return null;
}
Variant get_constant(CStrRef name, bool error) {
  return name;
}
Variant get_class_constant(const char *s, const char *prop,
                           bool fatal /* = true */) {
  return null;
}

Array get_global_array_wrapper() {
  return Array();
}

Variant *get_static_property_lv(const char *s, const char *prop) {
  return NULL;
}

bool get_call_info_static_method(MethodCallPackage &info) {
  return NULL;
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

Array get_global_state() { return Array(); }

///////////////////////////////////////////////////////////////////////////////
}
