/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/transport.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(apache_note, const String& note_name,
                      const String& note_value /* = null_string */) {
  String prev = ServerNote::Get(note_name);
  if (!note_value.empty()) {
    ServerNote::Add(note_name, note_value);
  }
  if (!prev.isNull()) {
    return prev;
  }
  return false;
}

static Array get_headers(HeaderMap& headers) {
  Array ret;
  for (auto& iter : headers) {
    const auto& values = iter.second;
    if (!values.size()) {
      continue;
    }
    ret.set(String(iter.first), String(values.back()));
  }
  return ret;
}

Array HHVM_FUNCTION(apache_request_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getHeaders(headers);
    return get_headers(headers);
  }
  return Array();
}

Array HHVM_FUNCTION(apache_response_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getResponseHeaders(headers);
    return get_headers(headers);
  }
  return Array();
}

bool HHVM_FUNCTION(apache_setenv, const String& variable, const String& value,
                     bool walk_to_top /* = false */) {
  return false;
}

Array HHVM_FUNCTION(getallheaders) {
  return HHVM_FN(apache_request_headers)();
}

const StaticString
  s_restart_time("restart_time"),
  s_max_clients("max_clients"),
  s_active_clients("active_clients"),
  s_queued_requests("queued_requests"),
  s_child_status("child_status");

Array HHVM_FUNCTION(apache_get_config) {
  int workers = 0, queued = 0;
  if (HttpServer::Server) {
    workers = HttpServer::Server->getPageServer()->getActiveWorker();
    queued = HttpServer::Server->getPageServer()->getQueuedJobs();
  }
  ArrayInit ret(4);
  ret.set(s_restart_time, HttpServer::StartTime);
  ret.set(s_max_clients, RuntimeOption::ServerThreadCount);
  ret.set(s_active_clients, workers);
  ret.set(s_queued_requests, queued);
  return ret.create();
}

///////////////////////////////////////////////////////////////////////////////

class ApacheExtension : public Extension {
 public:
  ApacheExtension() : Extension("apache") {}
  virtual void moduleInit() {
    HHVM_FE(apache_note);
    HHVM_FE(apache_request_headers);
    HHVM_FE(apache_response_headers);
    HHVM_FE(apache_setenv);
    HHVM_FE(getallheaders);
    HHVM_FE(apache_get_config);

    loadSystemlib();
  }
} s_apache_extension;

///////////////////////////////////////////////////////////////////////////////
}
