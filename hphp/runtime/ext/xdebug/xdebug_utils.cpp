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

#include "hphp/runtime/ext/xdebug/xdebug_utils.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/ext_datetime.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/url/ext_url.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Globals
const static StaticString
  s_COOKIE("_COOOKIE"),
  s_GET("_GET"),
  s_POST("_POST");

bool XDebugUtils::isTriggerSet(const String& trigger) {
  const ArrayData* globals = get_global_variables()->asArrayData();
  Array get = globals->get(s_GET).toArray();
  Array post = globals->get(s_POST).toArray();
  Array cookies = globals->get(s_COOKIE).toArray();
  return cookies.exists(trigger) || get.exists(trigger) || post.exists(trigger);
}

// TODO(#4489053) Clean this up-- this was taken from php5 xdebug
static char *xdebug_raw_url_encode(char const *s, int len, int *new_length,
                                   int skip_slash) {
  static unsigned char hexchars[] = "0123456789ABCDEF";

  register int x, y;
  unsigned char *str;

  str = (unsigned char *) xdmalloc(3 * len + 1);
  for (x = 0, y = 0; len--; x++, y++) {
    str[y] = (unsigned char) s[x];
    if ((str[y] < '0' && str[y] != '-' && str[y] != '.' &&
         (str[y] != '/' || !skip_slash)) ||
        (str[y] < 'A' && str[y] > '9' && str[y] != ':') ||
        (str[y] > 'Z' && str[y] < 'a' && str[y] != '_' &&
         (str[y] != '\\' || !skip_slash)) ||
        (str[y] > 'z')) {
      str[y++] = '%';
      str[y++] = hexchars[(unsigned char) s[x] >> 4];
      str[y] = hexchars[(unsigned char) s[x] & 15];
    }
  }
  str[y] = '\0';
  if (new_length) {
    *new_length = y;
  }
  return ((char *) str);
}

// TODO(#4489053) Clean this up-- this was taken from php5 xdebug
char* XDebugUtils::pathToUrl(const char* fileurl) {
  int l, i, new_len;
  char *tmp = nullptr;
  char *encoded_fileurl;

  /* encode the url */
  encoded_fileurl = xdebug_raw_url_encode(fileurl, strlen(fileurl),
                                          &new_len, 1);

  if (strncmp(fileurl, "phar://", 7) == 0) {
    /* ignore, phar is cool */
    tmp = xdstrdup(fileurl);
  } else if (fileurl[0] != '/' && fileurl[0] != '\\' && fileurl[1] != ':') {
    String path(fileurl, CopyString);
    Variant realpath = f_realpath(path);
    if (realpath.isString()) {
      char* realpath_str = realpath.toString().get()->mutableData();
      tmp = xdebug_sprintf("file://%s", realpath_str);
    } else {
      // Couldn't convert, use raw path
      tmp = xdstrdup(encoded_fileurl);
    }
  } else if (fileurl[1] == '/' || fileurl[1] == '\\') {
    // convert UNC paths (eg. \\server\sharepath)
    // http://blogs.msdn.com/ie/archive/2006/12/06/file-uris-in-windows.aspx
    tmp = xdebug_sprintf("file:%s", encoded_fileurl);
  } else if (fileurl[0] == '/' || fileurl[0] == '\\') {
    /* convert *nix paths (eg. /path) */
    tmp = xdebug_sprintf("file://%s", encoded_fileurl);
  } else if (fileurl[1] == ':') {
    /* convert windows drive paths (eg. c:\path) */
    tmp = xdebug_sprintf("file:///%s", encoded_fileurl);
  } else {
    /* no clue about it, use it raw */
    tmp = xdstrdup(encoded_fileurl);
  }
  l = strlen(tmp);
  /* convert '\' to '/' */
  for (i = 0; i < l; i++) {
    if (tmp[i] == '\\') {
      tmp[i]='/';
    }
  }
  xdfree(encoded_fileurl); // Needs to be free
  return tmp;
}

String XDebugUtils::pathFromUrl(const String& fileurl) {
  // Decode the url.
  String decoded = HHVM_FN(rawurldecode)(fileurl);

  // We want to remove "file://" if it exists. If it doesn't
  // just return fileurl
  int loc = decoded.find("file://");
  if (loc < 0) {
    return String(fileurl.data(), CopyString);
  }

  // php5 special cases this.
  loc += sizeof("file://") - 1;
  if (decoded[loc] == '/' && decoded[loc + 2] == ':') {
    loc++;
  }
  return decoded.substr(loc, decoded.size() - loc);
}

int XDebugUtils::stackDepth() {
  int depth = 0;
  for (ActRec* fp = g_context->getStackFrame();
      fp != nullptr;
      fp = g_context->getPrevVMState(fp), depth++) {}
  return depth;
}

///////////////////////////////////////////////////////////////////////////////
}
