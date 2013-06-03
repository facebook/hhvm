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

#include "hphp/util/json.h"

namespace HPHP { namespace JSON {
///////////////////////////////////////////////////////////////////////////////
// statics

std::string Escape(const char *s) {
  std::string ret;
  char hex[3];
  for (const char *p = s; *p; p++) {
    switch (*p) {
    case '\r': ret += "\\r";  break;
    case '\n': ret += "\\n";  break;
    case '\t': ret += "\\t";  break;
    case '/':  ret += "\\/";  break;
    case '\"': ret += "\\\""; break;
    case '\\': ret += "\\\\"; break;
    default:
      if (*p < ' ') {
        snprintf(hex, sizeof(hex), "%02x", *p);
        ret += "\\u00" + std::string(hex);
      } else {
        ret += *p;
      }
      break;
    }
  }
  return ret;
}
}}
