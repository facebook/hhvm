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

#include "hphp/runtime/ext/xdebug/ext_xdebug.h"

#include "hphp/runtime/ext/ext_datetime.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Class containing generic utility routines used by xdebug. Similar to
// xdebug's "usefulstuff.c"
class XDebugUtils {
public:
  // Helper that writes a timestamp in the given file in the format used by
  // xdebug
  static void fprintTimestamp(FILE* f) {
    DateTime now(time(nullptr));
    fprintf(f, "[%d-%02d-%02d %02d:%02d:%02d]",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());
  }

  // Returns true iff the passed trigger is set as a cookie or as a GET/POST
  // parameter
  static bool isTriggerSet(const String& trigger);

  // Translated from xdebug.
  // xdebug desc: fake URI's per IETF RFC 1738 and 2396 format
  static char* pathToUrl(char* fileurl);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_UTILS_H_
