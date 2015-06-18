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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(get_defined_functions);
bool HHVM_FUNCTION(function_exists, const String& function_name,
  bool autoload = true);
Variant HHVM_FUNCTION(call_user_func, const Variant& function,
  const Array& params = null_array);
Variant HHVM_FUNCTION(forward_static_call_array, const Variant& function,
  const Array& params);
Variant HHVM_FUNCTION(forward_static_call, const Variant& function,
  const Array& params = null_array);
String HHVM_FUNCTION(create_function, const String& args, const String& code);

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(func_get_arg, int arg_num);
// __SystemLib\\func_get_arg_sl
Variant HHVM_FUNCTION(SystemLib_func_get_arg_sl, int arg_num);

Variant HHVM_FUNCTION(func_get_args);
// __SystemLib\\func_get_args_sl
Variant HHVM_FUNCTION(SystemLib_func_get_args_sl);
Array hhvm_get_frame_args(const ActRec* ar, int offset);

/**
 * HipHop extension that allows requesting only a subset of function arguments.
 * Exposed as __SystemLib\func_slice_args.
 */
Variant HHVM_FUNCTION(SystemLib_func_slice_args, int offset);

int64_t HHVM_FUNCTION(func_num_args);
// __SystemLib\\func_num_arg_
int64_t HHVM_FUNCTION(SystemLib_func_num_arg_);

///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(register_postsend_function,
  const Variant& function, const Array& params = null_array);
void HHVM_FUNCTION(register_shutdown_function,
  const Variant& function, const Array& params = null_array);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_FUNCTION_H_
