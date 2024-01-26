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

#include "hphp/runtime/base/http-stream-wrapper.h"

#include "hphp/runtime/base/configs/server.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/url-file.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/url/ext_url.h"
#include "hphp/runtime/server/cli-server.h"
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
  s_proxy("proxy"),
  s_content("content"),
  s_user_agent("user_agent"),
  s_User_Agent("User-Agent");

req::ptr<File> HttpStreamWrapper::open(const String& filename,
                                       const String& mode, int /*options*/,
                                       const req::ptr<StreamContext>& context) {
  if (Cfg::Server::HttpSafeMode && !is_cli_server_mode()) {
    return nullptr;
  }

  if (strncmp(filename.data(), "http://",  sizeof("http://")  - 1) &&
      strncmp(filename.data(), "https://", sizeof("https://") - 1)) {
    return nullptr;
  }

  Array headers;
  String method = s_GET;
  String post_data = null_string;
  String proxy_host;
  String proxy_user;
  String proxy_pass;
  int proxy_port = -1;
  int max_redirs = 20;
  int timeout = -1;
  bool ignore_errors = false;

  if (context && !context->getOptions().isNull() &&
      !context->getOptions()[s_http].isNull()) {
    Array opts = context->getOptions()[s_http].toArray();
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
        headers.set(parts.lookup(0), parts.lookup(1));
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
    if (opts.exists(s_proxy)) {
      Variant host = HHVM_FN(parse_url)(opts[s_proxy].toString(), k_PHP_URL_HOST);
      Variant port = HHVM_FN(parse_url)(opts[s_proxy].toString(), k_PHP_URL_PORT);
      if (!same(host, false) && !same(port, false)) {
        proxy_host = host.toString();
        proxy_port = port.toInt64();
        Variant user = HHVM_FN(parse_url)(opts[s_proxy].toString(), k_PHP_URL_USER);
        Variant pass = HHVM_FN(parse_url)(opts[s_proxy].toString(), k_PHP_URL_PASS);
        if (!same(user, false) && !same(pass, false)) {
          proxy_user = user.toString();
          proxy_pass = pass.toString();
        }
      }
    }
    post_data = opts[s_content].toString();
  }

  if (!headers.exists(s_User_Agent)) {
    auto default_user_agent = RequestInfo::s_requestInfo.getNoCheck()
      ->m_reqInjectionData.getUserAgent();
    if (!default_user_agent.empty()) {
      headers.set(s_User_Agent, default_user_agent);
    }
  }
  auto file = req::make<UrlFile>(method.data(), headers,
                                    post_data, max_redirs,
                                    timeout, ignore_errors);
  file->setStreamContext(context);
  file->setProxy(proxy_host, proxy_port, proxy_user, proxy_pass);
  bool ret = file->open(filename, mode);
  if (!ret) {
    raise_warning("Failed to open %s (%s)", filename.data(),
                  file->getLastError().c_str());
    return nullptr;
  }
  return file;
}

///////////////////////////////////////////////////////////////////////////////
}
