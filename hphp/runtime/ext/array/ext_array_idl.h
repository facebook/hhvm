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

#ifndef incl_HPHP_EXT_ARRAY_IDL_H_
#define incl_HPHP_EXT_ARRAY_IDL_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {

Variant f_array_map(int _argc, const Variant& callback, const Variant& arr1,
                    const Array& _argv = null_array);
bool f_array_multisort(int _argc,
                       VRefParam arr1,
                       const Array& _argv = null_array);

}

#endif // incl_HPHP_EXT_ARRAY_IDL_H_
