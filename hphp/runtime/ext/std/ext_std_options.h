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

#ifndef incl_HPHP_EXT_OPTIONS_H_
#define incl_HPHP_EXT_OPTIONS_H_

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(ini_get, const String& varname);
Variant HHVM_FUNCTION(ini_set,
                      const String& varname, const Variant& newvalue);
String HHVM_FUNCTION(php_sapi_name);
Variant HHVM_FUNCTION(php_uname, const String& mode = empty_string);
String HHVM_FUNCTION(sys_get_temp_dir);
Variant HHVM_FUNCTION(version_compare,
                      const String& version1,
                      const String& version2,
                      const String& sop = empty_string);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_OPTIONS_H_
