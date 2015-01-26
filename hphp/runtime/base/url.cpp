/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/url.h"

#include "hphp/runtime/base/types.h"

namespace HPHP {
namespace URL {

const char *getServerObject(const char* url) {
  assert(url);
  const char *p;
  if (strncmp(url, "http://", 7) == 0) {
    p = strchr(url + 7, '/');
  } else if (strncmp(url, "https://", 8) == 0) {
    p = strchr(url + 8, '/');
  } else {
    p = strchr(url, '/');
  }
  
  if (p) {
    while (*(p + 1) == '/') p++;
    return p;
  }
  return url;
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

