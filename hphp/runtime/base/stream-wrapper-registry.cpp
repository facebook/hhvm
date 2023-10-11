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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/data-stream-wrapper.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/glob-stream-wrapper.h"
#include "hphp/runtime/base/http-stream-wrapper.h"
#include "hphp/runtime/base/php-stream-wrapper.h"
#include "hphp/runtime/base/req-optional.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/string-hash-set.h"
#include "hphp/runtime/base/string-hash-map.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/rds-local.h"

namespace HPHP::Stream {
///////////////////////////////////////////////////////////////////////////////

// Global registry for wrappers
static hphp_string_map<Wrapper*> s_wrappers;
static RDS_LOCAL(Wrapper*, rl_fileHandler);

bool registerWrapper(const std::string &scheme, Wrapper *wrapper) {
  assertx(!s_wrappers.count(scheme));
  s_wrappers[scheme] = wrapper;
  return true;
}

const StaticString
  s_file("file"),
  s_compress_zlib("compress.zlib"),
  s_data("data");

Array enumWrappers() {
  VecInit ret{s_wrappers.size()};
  for (auto const& e : s_wrappers) {
    ret.append(e.first);
  }
  return ret.toArray();
}

Wrapper* getWrapper(const String& scheme, bool warn /*= false */) {
  String lscheme = HHVM_FN(strtolower)(scheme);

  if (*rl_fileHandler && lscheme == s_file) {
    return *rl_fileHandler;
  }

  {
    auto it = s_wrappers.find(lscheme.data());
    if (it != s_wrappers.end()) {
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

Stream::Wrapper* getThreadLocalFileHandler() {
  return *rl_fileHandler;
}

void setThreadLocalFileHandler(Stream::Wrapper* wrapper) {
  *rl_fileHandler = wrapper;
}

///////////////////////////////////////////////////////////////////////////////
}
