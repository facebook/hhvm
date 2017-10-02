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

#include "hphp/runtime/ext/xdebug/util.h"

#include "hphp/runtime/ext/datetime/ext_datetime.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/url/ext_url.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"
#include "hphp/runtime/vm/globals-array.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Globals.
const StaticString
  s_COOKIE("_COOKIE"),
  s_GET("_GET"),
  s_POST("_POST");

void xdebug_print_timestamp(FILE* f) {
  auto now = req::make<DateTime>(time(nullptr));
  fprintf(f, "[%d-%02d-%02d %02d:%02d:%02d]",
          now->year(), now->month(), now->day(),
          now->hour(), now->minute(), now->second());
}

bool xdebug_trigger_set(const String& trigger) {
  auto const globals = get_global_variables()->asArrayData();
  auto const exists = [&] (const StaticString& str) {
    auto const global = tvCastToArrayLike(globals->get(str).tv());
    return global.exists(trigger);
  };
  return exists(s_COOKIE) || exists(s_GET) || exists(s_POST);
}

// TODO(#3704) Clean this up-- this was taken from php5 xdebug
static char* xdebug_raw_url_encode(const char* s, size_t len) {
  static unsigned char hexchars[] = "0123456789ABCDEF";

  auto str = (unsigned char*)HPHP::req::malloc_noptrs(3 * len + 1);

  int x;
  int y;

  for (x = 0, y = 0; len--; x++, y++) {
    str[y] = (unsigned char) s[x];
    if ((str[y] < '0' && str[y] != '-' && str[y] != '.' && (str[y] != '/')) ||
        (str[y] < 'A' && str[y] > '9' && str[y] != ':') ||
        (str[y] > 'Z' && str[y] < 'a' && str[y] != '_' && str[y] != '\\') ||
        (str[y] > 'z')) {
      str[y++] = '%';
      str[y++] = hexchars[(unsigned char) s[x] >> 4];
      str[y] = hexchars[(unsigned char) s[x] & 15];
    }
  }

  str[y] = '\0';
  return (char*)str;
}

// TODO(#3704) Clean this up-- this was taken from php5 xdebug
char* xdebug_path_to_url(const char* path) {
  char* tmp = nullptr;

  auto encoded = xdebug_raw_url_encode(path, strlen(path));
  SCOPE_EXIT { HPHP::req::free(encoded); };

  if (strncmp(path, "phar://", 7) == 0) {
    // Ignore, phar is cool.
    tmp = xdstrdup(path);
  } else if (path[0] != '/' && path[0] != '\\' && path[1] != ':') {
    auto realpath = HHVM_FN(realpath)(String(path, CopyString));
    if (realpath.isString()) {
      auto realpath_str = realpath.toString().get()->mutableData();
      tmp = xdebug_sprintf("file://%s", realpath_str);
    } else {
      // Couldn't convert, use raw path.
      tmp = xdstrdup(encoded);
    }
  } else if (path[1] == '/' || path[1] == '\\') {
    // Convert UNC paths (eg. \\server\sharepath).
    // http://blogs.msdn.com/ie/archive/2006/12/06/file-uris-in-windows.aspx
    tmp = xdebug_sprintf("file:%s", encoded);
  } else if (path[0] == '/' || path[0] == '\\') {
    // Convert *nix paths (eg. /path).
    tmp = xdebug_sprintf("file://%s", encoded);
  } else if (path[1] == ':') {
    // Convert windows drive paths (eg. c:\path).
    tmp = xdebug_sprintf("file:///%s", encoded);
  } else {
    // No clue about it, use it raw.
    tmp = xdstrdup(encoded);
  }

  auto const l = strlen(tmp);
  // Convert '\' to '/'.
  for (size_t i = 0; i < l; i++) {
    if (tmp[i] == '\\') {
      tmp[i]='/';
    }
  }
  return tmp;
}

String xdebug_path_to_url(const String& path) {
  auto url = xdebug_path_to_url(path.data());
  SCOPE_EXIT { HPHP::req::free(url); };
  return String(url, CopyString);
}

String xdebug_path_from_url(const String& url) {
  auto constexpr prefix = "file://";

  auto decoded = HHVM_FN(rawurldecode)(url);

  // We want to remove "file://" if it exists.  If it doesn't just return the
  // URL.
  auto loc = decoded.find(prefix);
  if (loc < 0) {
    return url;
  }

  // php5 special cases this.
  loc += sizeof(prefix) - 1;
  if (decoded[loc] == '/' && decoded[loc + 2] == ':') {
    loc++;
  }
  return decoded.substr(loc);
}

size_t xdebug_stack_depth() {
  size_t depth = 0;
  auto fp = g_context->getStackFrame();
  while (fp != nullptr) {
    fp = g_context->getPrevVMState(fp);
    depth++;
  }
  return depth;
}

///////////////////////////////////////////////////////////////////////////////
}
