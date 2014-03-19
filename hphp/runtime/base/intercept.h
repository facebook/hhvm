/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

class Array;
class String;
struct Variant;

///////////////////////////////////////////////////////////////////////////////
// fb_intercept()

/**
 * When an interceptable point is hit, invoke handler with all parameters
 * instead. If it returns FALSE, continues execution. Otherwise, return its
 * return.
 */
bool register_intercept(const String& name, const Variant& callback, const Variant& data);

/**
 * Check to see if it is actually intercepted for current request.
 */
Variant* get_intercept_handler(const String& name, char* flag);

/**
 * Call intercept handler with original parameters.
 */
bool handle_intercept(const Variant& handler, const String& name, const Array& params,
                      Variant& ret);

/**
 * Removes a previously registered flag.
 */
void unregister_intercept_flag(const String& name, char* flag);

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

void rename_function(const String& old_name, const String& new_name);

///////////////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_INTERCEPT_H_
