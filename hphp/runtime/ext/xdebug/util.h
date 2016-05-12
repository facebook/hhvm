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

#ifndef incl_HPHP_XDEBUG_UTILS_H_
#define incl_HPHP_XDEBUG_UTILS_H_

#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* Writes a timestamp in the given file in the format used by xdebug. */
void xdebug_print_timestamp(FILE*);

/* Checks if the passed trigger is set as a cooke or as a GET/POST parameter. */
bool xdebug_trigger_set(const String&);

/* Fake URI's per IETF RFC 1738 and 2396 format. */
char* xdebug_path_to_url(const char*);
String xdebug_path_to_url(const String&);

/* Decodes the given URL into a file path. */
String xdebug_path_from_url(const String&);

/* Gets the current stack depth.  Top-level code has a stack depth of 0. */
size_t xdebug_stack_depth();

///////////////////////////////////////////////////////////////////////////////
}

#endif
