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

#include <runtime/base/file/http_stream_wrapper.h>
#include <runtime/base/string_util.h>
#include <runtime/base/file/url_file.h>
#include <runtime/base/runtime_option.h>
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

File* HttpStreamWrapper::open(CStrRef filename, CStrRef mode,
                              CArrRef options) {
  if (RuntimeOption::ServerHttpSafeMode) {
    return NULL;
  }

  if (strncmp(filename.data(), "http://",  sizeof("http://")  - 1) &&
      strncmp(filename.data(), "https://", sizeof("https://") - 1)) {
    return NULL;
  }

  std::unique_ptr<UrlFile> file;
  if (options.isNull() || options["http"].isNull()) {
    file = std::unique_ptr<UrlFile>(NEWOBJ(UrlFile)());
  } else {
    Array opts = options["http"];
    String method = "GET";
    if (opts.exists("method")) {
      method = opts["method"].toString();
    }
    Array headers;
    if (opts.exists("header")) {
      Array lines = StringUtil::Explode(opts["header"].toString(), "\r\n");
      for (ArrayIter it(lines); it; ++it) {
        Array parts = StringUtil::Explode(it.second().toString(), ": ");
        headers.set(parts.rvalAt(0), parts.rvalAt(1));
      }
      if (opts.exists("user_agent") && !headers.exists("User-Agent")) {
        headers.set("User_Agent", opts["user_agent"]);
      }
    }
    int max_redirs = 20;
    if (opts.exists("max_redirects")) max_redirs = opts["max_redirects"];
    int timeout = -1;
    if (opts.exists("timeout")) timeout = opts["timeout"];
    file = std::unique_ptr<UrlFile>(NEWOBJ(UrlFile)(method.data(), headers,
                                                    opts["content"].toString(),
                                                    max_redirs, timeout));
  }
  bool ret = file->open(filename, mode);
  if (!ret) {
    raise_warning("Failed to open %s (%s)", filename.data(),
                  file->getLastError().c_str());
    return NULL;
  }
  return file.release();
}

///////////////////////////////////////////////////////////////////////////////
}
