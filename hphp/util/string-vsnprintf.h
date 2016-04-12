/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_STRING_VSNPRINTF_H_
#define incl_HPHP_STRING_VSNPRINTF_H_

#include <cstdarg>
#include <string>

#include "hphp/util/portability.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Printf into a std::string using a va_list.
 */
void string_vsnprintf(std::string& msg,
  ATTRIBUTE_PRINTF_STRING const char* fmt,
                          va_list ap) ATTRIBUTE_PRINTF(2,0);

//////////////////////////////////////////////////////////////////////

}

#endif
