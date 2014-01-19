/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_APACHE_H_
#define incl_HPHP_EXT_APACHE_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_apache_note(
  const String& note_name, const String& note_value = null_string);
Array f_apache_request_headers();
Array f_apache_response_headers();
bool f_apache_setenv(
  const String& variable, const String& value, bool walk_to_top = false);
Array f_getallheaders();
Variant f_apache_get_config();
Variant f_apache_get_scoreboard();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_APACHE_H_
