/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/name-value-table-wrapper.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// global declarations that have generated implementations

/**
 * Invoking an arbitrary user-defined function.
 */
Variant invoke(const char *function, CVarRef params, strhash_t hash = -1,
    bool tryInterp = true, bool fatal = true);

/**
 * Getting a static property
 */
extern Variant get_static_property(const String& s, const char *prop);

/**
 * Getting the init value of a class variable
 */
extern Variant get_class_var_init(const String& s, const char *var);

/**
 * Class/function meta info entirely encoded here as a const char * array.
 */
extern const char *g_class_map[];

/**
 * Returns a thread local global variable class pointer.
 */
typedef GlobalNameValueTableWrapper GlobalVariables;
extern GlobalVariables *get_global_variables();
extern void free_global_variables();
extern void free_global_variables_after_sweep();

/**
 * These are things that look like constants to PHP, but their values aren't
 * known at compile time and are instead determined per request at startup time.
 * lvalProxy is not that (it's a "black hole" for certain types of assignments)
 * but there isn't really an obviously better place for it to live.
 *
 * The standalone k_ constants are similarly dynamic but invariant per process.
 */
struct EnvConstants {
  static void requestInit(EnvConstants* gt);
  static void requestExit();
  Variant __lvalProxy;
  Variant stgv_Variant[1];
#define k_SID stgv_Variant[0]
};
extern EnvConstants* get_env_constants();
extern String k_PHP_BINARY;
extern String k_PHP_BINDIR;
extern String k_PHP_OS;
extern String k_PHP_SAPI;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CPP_BASE_HPHP_H_
