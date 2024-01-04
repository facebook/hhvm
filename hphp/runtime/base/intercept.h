/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#include <stdint.h>

namespace HPHP {

struct Array;
struct Func;
struct String;
struct Variant;

///////////////////////////////////////////////////////////////////////////////
// fb_intercept2()

/**
 * When an interceptable point is hit, invoke handler with all parameters
 * instead. If it returns FALSE, continues execution. Otherwise, return its
 * return.
 */
bool register_intercept(const String& name, const Variant& callback);

/**
 * Check to see if func is intercepted for current request.
 */
bool is_intercepted(const Func* func);

/**
 * If func is intercepted for the current request, return the handler.
 */
Variant* get_intercept_handler(const Func* func);

/**
 * Call intercept handler with original parameters.
 */
bool handle_intercept(const Variant& handler, const String& name,
                      const Array& params, Variant& ret);

/**
 * When a request terminates, reset all the translations of all the functions
 * intercepted by the request.
 */
void reset_all_intercepted_functions();

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

void rename_function(const String& old_name, const String& new_name);

///////////////////////////////////////////////////////////////////////////////

}

