/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_FUNCTION_H_
#define incl_HPHP_EXT_FUNCTION_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_get_defined_functions();
bool f_function_exists(const String& function_name, bool autoload = true);
bool f_is_callable(const Variant& v, bool syntax = false,
                   VRefParam name = uninit_null());
Variant f_call_user_func(
  int _argc, const Variant& function, const Array& _argv = null_array);
Variant f_call_user_func_array(const Variant& function, const Variant& params);
Variant f_forward_static_call_array(const Variant& function, const Array& params);
Variant f_forward_static_call(
  int _argc, const Variant& function, const Array& _argv = null_array);
Variant f_get_called_class();
String f_create_function(const String& args, const String& code);

///////////////////////////////////////////////////////////////////////////////

/**
 * PHP's func_get_arg() is transformed to this function with some extra
 * parameters to help the implementation.
 */
Variant f_func_get_arg(int arg_num);

/**
 * PHP's func_get_args() is transformed to this function with some extra
 * parameters to help the implementation.
 */
Variant f_func_get_args();
Array hhvm_get_frame_args(const ActRec* ar, int offset);

/**
 * HipHop extension that allows requesting only a subset of function arguments.
 * Exposed as __SystemLib\func_slice_args.
 */
Variant f_func_slice_args(int offset);

/**
 * HPHP actually inlines this function, so this is degenerated.
 */
int64_t f_func_num_args();

///////////////////////////////////////////////////////////////////////////////

void f_register_postsend_function(
  int _argc, const Variant& function, const Array& _argv = null_array);
void f_register_shutdown_function(
  int _argc, const Variant& function, const Array& _argv = null_array);
void f_register_cleanup_function(
  int _argc, const Variant& function, const Array& _argv = null_array);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_FUNCTION_H_
