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
#include "hphp/runtime/base/php-globals.h"

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/vm/name-value-table-wrapper.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString s_GLOBALS("GLOBALS");

extern GlobalNameValueTableWrapper* get_global_variables();

Array php_globals_as_array() {
  return Array(get_global_variables()->asArrayData());
}

//////////////////////////////////////////////////////////////////////

}
