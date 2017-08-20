/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include <set>
#include <map>
#include <algorithm>
#include <memory>

namespace HPHP { namespace Stream {
///////////////////////////////////////////////////////////////////////////////

struct RequestWrappers final : RequestEventHandler {
  void requestInit() override {}
  void requestShutdown() override {
    m_disabled.clear();
    m_wrappers.clear();
  }

  using DisabledSet = req::set<String>;
  using WrapperMap = req::map<String,req::unique_ptr<Wrapper>>;

  DisabledSet& disabled() {
    if (!m_disabled) m_disabled.emplace();
    return m_disabled.value();
  }
  WrapperMap& wrappers() {
    if (!m_wrappers) m_wrappers.emplace();
    return m_wrappers.value();
  }

private:
  req::Optional<DisabledSet> m_disabled;
  req::Optional<WrapperMap> m_wrappers;
};

// Global registry for wrappers
static std::map<std::string,Wrapper*> s_wrappers;
static __thread Wrapper* tl_fileHandler;

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
  String lscheme = HHVM_FN(strtolower)(scheme);
  bool ret = false;

  // Unregister request-specific wrappers entirely
  auto& wrappers = s_request_wrappers->wrappers();
  if (wrappers.find(lscheme) != wrappers.end()) {
    wrappers.erase(lscheme);
    ret = true;
  }

  // Disable builtin wrapper if it exists
  if (s_wrappers.find(lscheme.data()) == s_wrappers.end()) {
    // No builtin to disable
    return ret;
  }

  auto& disabled = s_request_wrappers->disabled();
  if (disabled.find(lscheme) != disabled.end()) {
    // Already disabled
    return ret;
  }

  // Disable it
  disabled.insert(lscheme);
  return true;
}

bool restoreWrapper(const String& scheme) {
  String lscheme = HHVM_FN(strtolower)(scheme);
  bool ret = false;

  // Unregister request-specific wrapper
  auto& wrappers = s_request_wrappers->wrappers();
  if (wrappers.find(lscheme) != wrappers.end()) {
    wrappers.erase(lscheme);
    ret = true;
  }

  // Un-disable builtin wrapper
  auto& disabled = s_request_wrappers->disabled();
  if (disabled.find(lscheme) == disabled.end()) {
    // Not disabled
    return ret;
  }

  // Perform action un-disable
  disabled.erase(lscheme);
  return true;
}

bool registerRequestWrapper(const String& scheme,
                            req::unique_ptr<Wrapper> wrapper) {
  String lscheme = HHVM_FN(strtolower)(scheme);

  // Global, non-disabled wrapper
  auto& disabled = s_request_wrappers->disabled();
  if ((s_wrappers.find(lscheme.data()) != s_wrappers.end()) &&
      (disabled.find(lscheme) == disabled.end())) {
    return false;
  }

  // A wrapper has already been registered for that scheme
  auto& wrappers = s_request_wrappers->wrappers();
  if (wrappers.find(lscheme) != wrappers.end()) {
    return false;
  }

  wrappers[lscheme] = std::move(wrapper);
  return true;
}

Array enumWrappers() {
  Array ret = Array::Create();

  // Enum global wrappers which are not disabled
  auto& disabled = s_request_wrappers->disabled();
  for (auto& e : s_wrappers) {
    if (disabled.find(e.first) == disabled.end()) {
      ret.append(e.first);
    }
  }

  // Enum request local wrappers
  for (auto& e : s_request_wrappers->wrappers()) {
    ret.append(e.first);
  }
  return ret;
}

Wrapper* getWrapper(const String& scheme, bool warn /*= false */) {
  /* As include() and require() support streams, we sometimes need to look up
   * a wrapper outside of a request - eg in HPHP::lookupUnit when using
   * StatCache. We can't look at the request locals then, as:
   * 1. requestShutdown has already been called
   * 2. dereferencing s_request_wrappers will call requestInit, and register
   *    a request shutdown event handler
   * 3. the list of request event handlers is a req::vector, so it gets lost
   *    at the end of the request.
   *
   * The result of this is that s_request_wrappers is no longer request-local -
   * requestInit() and requestShutdown() will never be called again. As it
   * holds references to request-allocated data, this leads to intermittent
   * segfaults.
   */
  bool have_request_wrappers = s_request_wrappers.getInited();

  String lscheme = HHVM_FN(strtolower)(scheme);

  if (tl_fileHandler && lscheme == s_file) {
    return tl_fileHandler;
  }

  // Request local wrapper?
  if (have_request_wrappers) {
    auto& wrappers = s_request_wrappers->wrappers();
    auto it = wrappers.find(lscheme);
    if (it != wrappers.end()) {
      return it->second.get();
    }
  }

  // Global, non-disabled wrapper?
  {
    auto disabledWrappers = [] () -> RequestWrappers::DisabledSet& {
      return s_request_wrappers->disabled();
    };
    auto it = s_wrappers.find(lscheme.data());
    if ((it != s_wrappers.end()) &&
        (!have_request_wrappers ||
        (disabledWrappers().find(lscheme) == disabledWrappers().end()))) {
      return it->second;
    }
  }

  if (warn) {
    raise_warning("Unknown URI scheme \"%s\"", scheme.c_str());
  }
  return nullptr;
}

String getWrapperProtocol(const char* uri_string, int* pathIndex) {
  /* Special case for PHP4 Backward Compatibility */
  if (!strncasecmp(uri_string, "zlib:", sizeof("zlib:") - 1)) {
    if (pathIndex != nullptr) *pathIndex = sizeof("zlib:") - 1;
    return s_compress_zlib;
  }

  // data wrapper can come with or without a double forward slash
  if (!strncasecmp(uri_string, "data:", sizeof("data:") - 1)) {
    if (pathIndex != nullptr) *pathIndex = sizeof("data:") - 1;
    return s_data;
  }

  int n = 0;
  const char* p = uri_string;
  while (*p && (isalnum(*p) || *p == '+' || *p == '-' || *p == '.')) {
    n++;
    p++;
  }
  const char* colon = nullptr;
  if (*p == ':' && n > 1 && (!strncmp("//", p + 1, 2))) {
    colon = p;
  }

  if (!colon) {
    if (pathIndex != nullptr) *pathIndex = 0;
    return s_file;
  }

  int len = colon - uri_string;
  if (pathIndex != nullptr) *pathIndex = len + sizeof("://") - 1;
  return String(uri_string, len, CopyString);
}

Wrapper* getWrapperFromURI(const String& uri,
                           int* pathIndex /* = NULL */,
                           bool warn /*= true */) {
  return getWrapper(getWrapperProtocol(uri.data(), pathIndex), warn);
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

void setThreadLocalFileHandler(Stream::Wrapper* wrapper) {
  tl_fileHandler = wrapper;
}

///////////////////////////////////////////////////////////////////////////////
}}
