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

#ifndef incl_HPHP_CPP_BASE_EXTERNALS_H_
#define incl_HPHP_CPP_BASE_EXTERNALS_H_

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

#endif // incl_HPHP_CPP_BASE_HPHP_H_
