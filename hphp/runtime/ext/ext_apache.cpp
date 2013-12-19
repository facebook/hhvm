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

#include "hphp/runtime/ext/ext_apache.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/transport.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_apache_note(const String& note_name,
                      const String& note_value /* = null_string */) {
  String prev = ServerNote::Get(note_name);
  if (!note_value.isNull()) {
    ServerNote::Add(note_name, note_value);
  }
  if (!prev.isNull()) {
    return prev;
  }
  return false;
}

Array f_apache_request_headers() {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getHeaders(headers);
    Array ret;
    for (HeaderMap::const_iterator iter = headers.begin();
         iter != headers.end(); ++iter) {
      const vector<string> &values = iter->second;
      ret.set(String(iter->first), String(values.back()));
    }
    return ret;
  }
  return Array();
}

Array f_apache_response_headers() {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getResponseHeaders(headers);
    Array ret;
    for (HeaderMap::const_iterator iter = headers.begin();
         iter != headers.end(); ++iter) {
      const vector<string> &values = iter->second;
      ret.set(String(iter->first), String(values.back()));
    }
    return ret;
  }
  return Array();
}

bool f_apache_setenv(const String& variable, const String& value,
                     bool walk_to_top /* = false */) {
  return false;
}

Array f_getallheaders() {
  return f_apache_request_headers();
}

const StaticString
  s_restart_time("restart_time"),
  s_max_clients("max_clients"),
  s_active_clients("active_clients"),
  s_queued_requests("queued_requests"),
  s_child_status("child_status");

Variant f_apache_get_config() {
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

Variant f_apache_get_scoreboard() {
  Array child_status;
  for (int i = 0; i < RuntimeOption::ServerThreadCount; i++) {
    child_status.set(i, 2);
  }
  ArrayInit ret(1);
  ret.set(s_child_status, child_status);
  return ret.create();
}

///////////////////////////////////////////////////////////////////////////////
}
