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

#ifndef __CPP_BASE_EXTERNALS_H__
#define __CPP_BASE_EXTERNALS_H__

///////////////////////////////////////////////////////////////////////////////

/**
 * All external dependencies of runtime/base. This file may not include any
 * generated file based on user code, as this file is included by some runtime/base
 * .cpp files.
 */

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// global declarations that have generated implementations

/**
 * Invoking an arbitrary user-defined function.
 */
extern Variant invoke(const char *function, CArrRef params, int64 hash = -1,
                      bool tryInterp = true, bool fatal = true);

/**
 * Invoking an arbitrary system function. This is the fallback for invoke.
 */
extern Variant invoke_builtin(const char *s, const Array &params, int64 hash,
                              bool fatal);

/**
 * Invoking an arbitrary static method.
 */
extern Variant invoke_static_method(const char *s, const char *method,
                                    const Array &params, bool fatal = true);
/**
 * defined in builtin_functions.cpp, used for "static::" resolution
 */
Variant invoke_static_method_bind(CStrRef s, const char *method,
                                  const Array &params, bool fatal = true);

/**
 * Invoking an arbitrary system static method.
 */
extern Variant invoke_builtin_static_method(const char *s, const char *method,
                                            const Array &params, bool fatal);

/**
 * Getting a static property
 */
extern Variant get_static_property(const char *s, const char *prop);
extern Variant get_builtin_static_property(const char *s, const char *prop);
extern Variant *get_static_property_lv(const char *s, const char *prop);
extern Variant *get_builtin_static_property_lv(const char *s, const char *prop);

// defined in builtin_functions.cpp
Variant &get_static_property_lval(const char *s, const char *prop);

/**
 * Getting the init value of a class variable
 */
extern Variant get_class_var_init(const char *s, const char *var);
extern Variant get_builtin_class_var_init(const char *s, const char *var);

/**
 * Getting a constant
 */
extern Variant get_constant(CStrRef name);
extern Variant get_builtin_constant(CStrRef name);

/**
 * Getting a class constant
 */
extern Variant get_class_constant(const char *s, const char *prop,
                                  bool fatal = true);
extern Variant get_builtin_class_constant(const char *s, const char *prop,
                                          bool fatal = true);

/**
 * Class/function meta info entirely encoded here as a const char * array.
 */
extern const char *g_class_map[];
extern const char *g_source_root;
extern const char *g_source_info[];
extern const char *g_source_cls2file[];
extern const char *g_source_func2file[];
extern const char *g_paramrtti_map[];

/**
 * Dynamically create an object.
 */
extern Object create_object(const char *s, const Array &params,
                            bool init = true, ObjectData *root = NULL);
/**
 * Dynamically create a system object.
 */
extern Object create_builtin_object(const char *s, const Array &params,
                                    bool init = true, ObjectData *root = NULL);
/**
 * Dynamically include a file.
 */
class LVariableTable;
extern Variant invoke_file(CStrRef file, bool once = false,
                           LVariableTable* variables = NULL,
                           const char *currentDir = NULL);

/**
 * Initializes constant strings and scalar arrays.
 */
extern void init_static_variables();

/**
 * Returns a thread local global variable class pointer.
 */
class GlobalVariables;
extern GlobalVariables *get_global_variables();
extern void init_global_variables();
extern void free_global_variables();
extern Array get_global_array_wrapper();
extern void output_global_state(FILE *fp);

/**
 * Returns a thread local global variable table pointer.
 */
extern LVariableTable *get_variable_table();
class Globals;
extern Globals *get_globals();
class SystemGlobals;
extern SystemGlobals *get_system_globals();

/**
 * Precomputed literal strings
 */
extern StaticString literalStrings[];

extern unsigned int *getRTTICounter(int id);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CPP_BASE_HPHP_H__
