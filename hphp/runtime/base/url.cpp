/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/url.h"

#include "hphp/runtime/base/types.h"

namespace HPHP {
namespace URL {

const char *getServerObject(const char* url) {
  assert(url);
  int strip = 0;
  if (strncmp(url, "http://", 7) == 0) {
    strip = 7;
  } else if (strncmp(url, "https://", 8) == 0) {
    strip = 8;
  }
  const char *p = strchr(url + strip, '/');
  if (p) {
    while (*(p + 1) == '/') p++;
    return p;
  }
  if (strip == 0) return url;
  return "";
}

std::string getCommand(const char* serverObject) {
  assert(serverObject);
  if (!*serverObject) {
    return "";
  }

  while (*serverObject == '/') {
    ++serverObject;
  }
  const char *v = strchr(serverObject, '?');
  if (v) {
    return std::string(serverObject, v - serverObject);
  }
  return serverObject;
}

} // namespace URL
} // namespace HPHP

