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
#ifndef incl_HPHP_INTERCEPT_H_
#define incl_HPHP_INTERCEPT_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/util/case-insensitive.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// fb_intercept()

/**
 * When an interceptable point is hit, invoke handler with all parameters
 * instead. If it returns FALSE, continues execution. Otherwise, return its
 * return.
 */
bool register_intercept(const String& name, CVarRef callback, CVarRef data);

/**
 * Check to see if it is actually intercepted for current request.
 */
Variant *get_intercept_handler(const String& name, char* flag);

/**
 * Check to see if it is actually intercepted for current request,
 * when we already know its flag is set
 */
Variant *get_enabled_intercept_handler(const String& name);

/**
 * Call intercept handler with original parameters.
 */
bool handle_intercept(CVarRef handler, const String& name, CArrRef params,
                      Variant &ret);

/**
 * Removes a previously registered flag.
 */
void unregister_intercept_flag(const String& name, char *flag);

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

/**
 * Functions on the specified list will be the only ones that are allowed
 * to rename. Without calling this function, all functions are allowed to
 * rename, even including built-in functions.
 */
void check_renamed_functions(CArrRef names);

/**
 * Checks to see if the specified function is on the allow list.
 */
bool check_renamed_function(const String& name);

/**
 * Rename a function.
 */
void rename_function(const String& old_name, const String& new_name);

/**
 * Get the actual function to call.
 */
String get_renamed_function(const String& name);

extern DECLARE_THREAD_LOCAL_NO_CHECK(bool, s_hasRenamedFunction);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_INTERCEPT_H_
