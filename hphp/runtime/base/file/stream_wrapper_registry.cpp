/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/file/stream_wrapper_registry.h>
#include <runtime/base/file/file.h>
#include <runtime/base/string_util.h>
#include <runtime/base/file/file_stream_wrapper.h>
#include <runtime/base/file/php_stream_wrapper.h>
#include <runtime/base/file/http_stream_wrapper.h>

namespace HPHP { namespace Stream {
///////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string,Wrapper*> wrapper_map_t;

// Global registry for wrappers
static wrapper_map_t s_wrappers;

bool registerWrapper(const std::string &scheme, Wrapper *wrapper) {
  assert(s_wrappers.find(scheme) == s_wrappers.end());
  s_wrappers[scheme] = wrapper;
  return true;
}

Wrapper* getWrapper(CStrRef scheme) {
  String lscheme = StringUtil::ToLower(scheme);
  auto it = s_wrappers.find(lscheme.data());
  if (it != s_wrappers.end()) {
    return it->second;
  }
  return nullptr;
}

File* open(CStrRef uri, CStrRef mode, int options, CVarRef context) {
  const char *uri_string = uri.data();
  Wrapper *wrapper = nullptr;

  /* Special case for PHP4 Backward Compatability */
  if (!strncasecmp(uri_string, "zlib:", sizeof("zlib:") - 1)) {
    wrapper = getWrapper("compress.zlib");
  } else {
    const char *colon = strstr(uri_string, "://");
    if (colon) {
      wrapper = getWrapper(String(uri_string, colon - uri_string, CopyString));
    }
  }

  if (wrapper == nullptr) {
    wrapper = getWrapper("file");
  }
  assert(wrapper);

  return wrapper->open(uri, mode, options, context);
}

static FileStreamWrapper s_file_stream_wrapper;
static PhpStreamWrapper  s_php_stream_wrapper;
static HttpStreamWrapper s_http_stream_wrapper;

void RegisterCoreWrappers() {
  s_file_stream_wrapper.registerAs("file");
  s_php_stream_wrapper.registerAs("php");
  s_http_stream_wrapper.registerAs("http");
  s_http_stream_wrapper.registerAs("https");
}

///////////////////////////////////////////////////////////////////////////////
}}
