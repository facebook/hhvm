/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/http-stream-wrapper.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/url-file.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/base/ini-setting.h"
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_GET("GET"),
  s_method("method"),
  s_http("http"),
  s_header("header"),
  s_ignore_errors("ignore_errors"),
  s_max_redirects("max_redirects"),
  s_timeout("timeout"),
  s_content("content"),
  s_user_agent("user_agent"),
  s_User_Agent("User-Agent");

File* HttpStreamWrapper::open(const String& filename, const String& mode,
                              int options, const Variant& context) {
  if (RuntimeOption::ServerHttpSafeMode) {
    return nullptr;
  }

  if (strncmp(filename.data(), "http://",  sizeof("http://")  - 1) &&
      strncmp(filename.data(), "https://", sizeof("https://") - 1)) {
    return nullptr;
  }

  std::unique_ptr<UrlFile> file;
  StreamContext *ctx = !context.isResource() ? nullptr :
                        context.toResource().getTyped<StreamContext>();
  Array headers;
  String method = s_GET;
  String post_data = null_string;
  int max_redirs = 20;
  int timeout = -1;
  bool ignore_errors = false;

  if (ctx && !ctx->getOptions().isNull() &&
      !ctx->getOptions()[s_http].isNull()) {
    Array opts = ctx->getOptions()[s_http].toArray();
    if (opts.exists(s_method)) {
      method = opts[s_method].toString();
    }
    if (opts.exists(s_header)) {
      Array lines;
      if (opts[s_header].isString()) {
        lines = StringUtil::Explode(
          opts[s_header].toString(), "\r\n").toArray();
      } else if (opts[s_header].isArray()) {
        lines = opts[s_header];
      }

      for (ArrayIter it(lines); it; ++it) {
        Array parts = StringUtil::Explode(
          it.second().toString(), ":", 2).toArray();
        headers.set(parts.rvalAt(0), parts.rvalAt(1));
      }
    }
    if (opts.exists(s_user_agent) && !headers.exists(s_User_Agent)) {
      headers.set(s_User_Agent, opts[s_user_agent]);
    }
    if (opts.exists(s_max_redirects)) {
      max_redirs = opts[s_max_redirects].toInt64();
    }
    if (opts.exists(s_timeout)) {
      timeout = opts[s_timeout].toInt64();
    }
    if (opts.exists(s_ignore_errors)) {
      ignore_errors = opts[s_ignore_errors].toBoolean();
    }
    post_data = opts[s_content].toString();
  }

  if (!headers.exists(s_User_Agent)) {
    auto default_user_agent = ThreadInfo::s_threadInfo.getNoCheck()
      ->m_reqInjectionData.getUserAgent();
    if (!default_user_agent.empty()) {
      headers.set(s_User_Agent, default_user_agent);
    }
  }
  file = std::unique_ptr<UrlFile>(newres<UrlFile>(method.data(), headers,
                                                  post_data, max_redirs,
                                                  timeout, ignore_errors));
  bool ret = file->open(filename, mode);
  if (!ret) {
    raise_warning("Failed to open %s (%s)", filename.data(),
                  file->getLastError().c_str());
    return nullptr;
  }
  return file.release();
}

///////////////////////////////////////////////////////////////////////////////
}
