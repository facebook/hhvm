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

#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/php-stream-wrapper.h"
#include "hphp/runtime/base/http-stream-wrapper.h"
#include "hphp/runtime/base/data-stream-wrapper.h"
#include "hphp/runtime/base/glob-stream-wrapper.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/ext/ext_string.h"
#include <set>
#include <map>

namespace HPHP { namespace Stream {
///////////////////////////////////////////////////////////////////////////////

namespace {
class RequestWrappers : public RequestEventHandler {
 public:
  virtual void requestInit() {}
  virtual void requestShutdown() {
    m_disabled.clear();
    m_wrappers.clear();
  }

  std::set<String> m_disabled;
  std::map<String,std::unique_ptr<Wrapper>> m_wrappers;
};
} // empty namespace

typedef std::map<std::string,Wrapper*> wrapper_map_t;

// Global registry for wrappers
static wrapper_map_t s_wrappers;

// Request local registry for user defined wrappers and disabled builtins
IMPLEMENT_STATIC_REQUEST_LOCAL(RequestWrappers, s_request_wrappers);

bool registerWrapper(const std::string &scheme, Wrapper *wrapper) {
  assert(s_wrappers.find(scheme) == s_wrappers.end());
  s_wrappers[scheme] = wrapper;
  return true;
}

const StaticString
  s_file("file"),
  s_compress_zlib("compress.zlib"),
  s_data("data");

bool disableWrapper(const String& scheme) {
  String lscheme = f_strtolower(scheme);

  if (lscheme.same(s_file)) {
    // Zend quietly succeeds, but does nothing
    return true;
  }

  bool ret = false;

  // Unregister request-specific wrappers entirely
  if (s_request_wrappers->m_wrappers.find(lscheme) !=
      s_request_wrappers->m_wrappers.end()) {
    s_request_wrappers->m_wrappers.erase(lscheme);
    ret = true;
  }

  // Disable builtin wrapper if it exists
  if (s_wrappers.find(lscheme.data()) == s_wrappers.end()) {
    // No builtin to disable
    return ret;
  }

  if (s_request_wrappers->m_disabled.find(lscheme) !=
      s_request_wrappers->m_disabled.end()) {
    // Already disabled
    return ret;
  }

  // Disable it
  s_request_wrappers->m_disabled.insert(lscheme);
  return true;
}

bool restoreWrapper(const String& scheme) {
  String lscheme = f_strtolower(scheme);
  bool ret = false;

  // Unregister request-specific wrapper
  if (s_request_wrappers->m_wrappers.find(lscheme) !=
    s_request_wrappers->m_wrappers.end()) {
    s_request_wrappers->m_wrappers.erase(lscheme);
    ret = true;
  }

  // Un-disable builtin wrapper
  if (s_request_wrappers->m_disabled.find(lscheme) ==
      s_request_wrappers->m_disabled.end()) {
    // Not disabled
    return ret;
  }

  // Perform action un-disable
  s_request_wrappers->m_disabled.erase(lscheme);
  return true;
}

bool registerRequestWrapper(const String& scheme,
                            std::unique_ptr<Wrapper> wrapper) {
  String lscheme = f_strtolower(scheme);

  // Global, non-disabled wrapper
  if ((s_wrappers.find(lscheme.data()) != s_wrappers.end()) &&
      (s_request_wrappers->m_disabled.find(lscheme) ==
       s_request_wrappers->m_disabled.end())) {
    return false;
  }

  // A wrapper has already been registered for that scheme
  if (s_request_wrappers->m_wrappers.find(lscheme) !=
      s_request_wrappers->m_wrappers.end()) {
    return false;
  }

  s_request_wrappers->m_wrappers[lscheme] = std::move(wrapper);
  return true;
}

Array enumWrappers() {
  Array ret = Array::Create();

  // Enum global wrappers which are not disabled
  for (auto it = s_wrappers.begin(); it != s_wrappers.end(); ++it) {
    if (s_request_wrappers->m_disabled.find(it->first) ==
        s_request_wrappers->m_disabled.end()) {
      ret.append(it->first);
    }
  }

  // Enum request local wrappers
  for (auto it = s_request_wrappers->m_wrappers.begin();
       it != s_request_wrappers->m_wrappers.end(); ++it) {
    ret.append(it->first);
  }
  return ret;
}

Wrapper* getWrapper(const String& scheme) {
  String lscheme = f_strtolower(scheme);

  // Request local wrapper?
  {
    auto it = s_request_wrappers->m_wrappers.find(lscheme);
    if (it != s_request_wrappers->m_wrappers.end()) {
      return it->second.get();
    }
  }

  // Global, non-disabled wrapper?
  {
    auto it = s_wrappers.find(lscheme.data());
    if ((it != s_wrappers.end()) &&
        (s_request_wrappers->m_disabled.find(lscheme) ==
         s_request_wrappers->m_disabled.end())) {
      return it->second;
    }
  }

  return nullptr;
}

Wrapper* getWrapperFromURI(const String& uri) {
  const char *uri_string = uri.data();

  /* Special case for PHP4 Backward Compatability */
  if (!strncasecmp(uri_string, "zlib:", sizeof("zlib:") - 1)) {
    return getWrapper(s_compress_zlib);
  }

  // data wrapper can come with or without a double forward slash
  if (!strncasecmp(uri_string, "data:", sizeof("data:") - 1)) {
    return getWrapper(s_data);
  }

  const char *colon = strstr(uri_string, "://");
  if (!colon) {
    return getWrapper(s_file);
  }

  int len = colon - uri_string;
  if (Wrapper *w = getWrapper(String(uri_string, len, CopyString))) {
    return w;
  }
  return getWrapper(s_file);
}

static FileStreamWrapper s_file_stream_wrapper;
static PhpStreamWrapper  s_php_stream_wrapper;
static HttpStreamWrapper s_http_stream_wrapper;
static DataStreamWrapper s_data_stream_wrapper;
static GlobStreamWrapper s_glob_stream_wrapper;

void RegisterCoreWrappers() {
  s_file_stream_wrapper.registerAs("file");
  s_php_stream_wrapper.registerAs("php");
  s_http_stream_wrapper.registerAs("http");
  s_http_stream_wrapper.registerAs("https");
  s_data_stream_wrapper.registerAs("data");
  s_glob_stream_wrapper.registerAs("glob");
}

///////////////////////////////////////////////////////////////////////////////
}}
