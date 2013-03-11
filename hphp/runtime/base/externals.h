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

#ifndef __CPP_BASE_EXTERNALS_H__
#define __CPP_BASE_EXTERNALS_H__

///////////////////////////////////////////////////////////////////////////////

/**
 * All external dependencies of runtime/base. This file may not include any
 * generated file based on user code, as this file is included by some
 * runtime/base .cpp files.
 */

#include <runtime/base/types.h>
#include <runtime/vm/name_value_table_wrapper.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// global declarations that have generated implementations

/**
 * Invoking an arbitrary user-defined function.
 */
Variant invoke(const char *function, CArrRef params, strhash_t hash = -1,
    bool tryInterp = true, bool fatal = true);

/**
 * Getting a static property
 */
extern Variant get_static_property(CStrRef s, const char *prop);

/**
 * Getting the init value of a class variable
 */
extern Variant get_class_var_init(CStrRef s, const char *var);

/**
 * Class/function meta info entirely encoded here as a const char * array.
 */
extern const char *g_class_map[];
#if 0
extern const char *g_source_root;
extern const char *g_source_info[];
extern const char *g_source_cls2file[];
extern const char *g_source_func2file[];
extern const char *g_paramrtti_map[];
#endif

/**
 * Dynamically create an object.
 */
Object create_object(CStrRef s, const Array &params,
                     bool init = true, ObjectData *root = nullptr);
extern Object create_object_only(CStrRef s, ObjectData *root = nullptr);

/**
 * Returns a thread local global variable class pointer.
 */
typedef VM::GlobalNameValueTableWrapper GlobalVariables;
extern GlobalVariables *get_global_variables();
extern void init_global_variables();
extern void free_global_variables();
extern void free_global_variables_after_sweep();
extern Array get_global_state();
/**
 * Returns a thread local global variable table pointer.
 */
typedef VM::GlobalNameValueTableWrapper SystemGlobals;
extern SystemGlobals *get_system_globals();

/**
 * Precomputed literal strings
 */
extern StaticString literalStrings[];

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CPP_BASE_HPHP_H__
