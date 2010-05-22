/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_apache.h>
#include <runtime/base/server/http_server.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_note.h>
#include <runtime/base/server/transport.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_apache_child_terminate() {
  throw NotSupportedException(__func__, "apache is not in use");
}

Array f_apache_get_modules() {
  throw NotSupportedException(__func__, "apache is not in use");
}

String f_apache_get_version() {
  throw NotSupportedException(__func__, "apache is not in use");
}

String f_apache_getenv(CStrRef variable, bool walk_to_top /* = false */) {
  throw NotSupportedException(__func__, "apache is not in use");
}

Object f_apache_lookup_uri(CStrRef filename) {
  throw NotSupportedException(__func__, "apache is not in use");
}

Variant f_apache_note(CStrRef note_name,
                      CStrRef note_value /* = null_string */) {
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

bool f_apache_reset_timeout() {
  throw NotSupportedException(__func__, "apache is not in use");
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

bool f_apache_setenv(CStrRef variable, CStrRef value,
                     bool walk_to_top /* = false */) {
  return false;
}

int f_ascii2ebcdic(CStrRef ascii_str) {
  throw NotSupportedException(__func__, "apache is not in use");
}

int f_ebcdic2ascii(CStrRef ebcdic_str) {
  throw NotSupportedException(__func__, "apache is not in use");
}

Array f_getallheaders() {
  return f_apache_request_headers();
}

bool f_virtual(CStrRef filename) {
  throw NotSupportedException(__func__, "apache is not in use");
}

Variant f_apache_get_config() {
  Array ret;
  ret.set("restart_time", HttpServer::StartTime);
  ret.set("max_clients", RuntimeOption::ServerThreadCount);
  return ret;
}

Variant f_apache_get_scoreboard() {
  Array ret;
  Array child_status;
  for (int i = 0; i < RuntimeOption::ServerThreadCount; i++) {
    child_status.set(i, 2);
  }
  ret.set("child_status", child_status);
  return ret;
}

Variant f_apache_get_rewrite_rules() {
  throw NotSupportedException(__func__, "apache is not in use");
}

///////////////////////////////////////////////////////////////////////////////
}
