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

#include <runtime/ext/ext_server.h>
#include <runtime/base/server/satellite_server.h>
#include <runtime/base/server/pagelet_server.h>
#include <runtime/base/server/xbox_server.h>
#include <runtime/base/server/http_protocol.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/server/rpc_request_handler.h>

using namespace std;
using namespace boost;

#define DANGLING_HEADER "HPHP_DANGLING"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(server);
///////////////////////////////////////////////////////////////////////////////
// dangling server

bool f_dangling_server_proxy_old_request() {
  static bool s_detected_dangling_server = true;

  if (!s_detected_dangling_server ||
      SatelliteServerInfo::DanglingServerPort == 0) {
    return false;
  }

  Transport *transport = g_context->getTransport();
  if (transport == NULL) {
    return false;
  }
  if (!transport->getHeader(DANGLING_HEADER).empty()) {
    // if we are processing a dangling server request, do not do it again
    return false;
  }

  string url = "http://localhost:" +
    lexical_cast<string>(SatelliteServerInfo::DanglingServerPort) +
    transport->getServerObject();

  int code = 0;
  std::string error;
  StringBuffer response;
  HeaderMap headers;
  headers[DANGLING_HEADER].push_back("1");
  if (!HttpProtocol::ProxyRequest(transport, false, url, code, error,
                                  response, &headers)) {
    s_detected_dangling_server = false;
    return false;
  }
  transport->setResponse(code, "dangling_server_proxy_old_request");
  echo(response.detach());
  return true;
}

bool f_dangling_server_proxy_new_request(CStrRef host) {
  if (host.empty()) {
    raise_warning("proxy new request needs host name");
    return false;
  }

  Transport *transport = g_context->getTransport();
  if (transport == NULL) {
    return false;
  }
  if (!transport->getHeader(DANGLING_HEADER).empty()) {
    // if we are processing a dangling server request, do not do it again
    return false;
  }

  string url = string("http://") + host.data() + ":" +
    lexical_cast<string>(RuntimeOption::ServerPort) +
    transport->getServerObject();

  int code = 0;
  std::string error;
  StringBuffer response;
  HeaderMap headers;
  headers[DANGLING_HEADER].push_back("1");
  if (!HttpProtocol::ProxyRequest(transport, false, url, code, error,
                                  response, &headers)) {
    return false;
  }
  transport->setResponse(code, "dangling_server_proxy_new_request");
  echo(response.detach());
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Pagelet Server

const int64 k_PAGELET_NOT_READY = PAGELET_NOT_READY;
const int64 k_PAGELET_READY     = PAGELET_READY;
const int64 k_PAGELET_DONE      = PAGELET_DONE;

bool f_pagelet_server_is_enabled() {
  return PageletServer::Enabled();
}

Object f_pagelet_server_task_start(CStrRef url,
                                   CArrRef headers /* = null_array */,
                                   CStrRef post_data /* = null_string */) {
  String remote_host;
  Transport *transport = g_context->getTransport();
  if (transport) {
    remote_host = transport->getRemoteHost();
    if (!headers.exists("Host") && RuntimeOption::SandboxMode) {
      Array tmp = headers;
      tmp.set("Host", transport->getHeader("Host"));
      return PageletServer::TaskStart(url, tmp, remote_host, post_data);
    }
  }
  return PageletServer::TaskStart(url, headers, remote_host, post_data);
}

int64 f_pagelet_server_task_status(CObjRef task) {
  return PageletServer::TaskStatus(task);
}

String f_pagelet_server_task_result(CObjRef task, Variant headers,
                                    Variant code) {
  Array rheaders;
  int rcode;
  String response = PageletServer::TaskResult(task, rheaders, rcode);
  headers = rheaders;
  code = rcode;
  return response;
}

void f_pagelet_server_flush() {
  ExecutionContext *context = g_context.getNoCheck();
  Transport *transport = context->getTransport();
  if (transport && transport->getThreadType() == Transport::PageletThread) {
    // this method is only meaningful in a pagelet thread
    context->obFlushAll();
    String content = context->obDetachContents();
    string s(content.data(), content.size());
    if (!s.empty()) {
      PageletServer::AddToPipeline(s);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// xbox

bool f_xbox_send_message(CStrRef msg, Variant ret, int64 timeout_ms,
                         CStrRef host /* = "localhost" */) {
  return XboxServer::SendMessage(msg, ret, timeout_ms, host);
}

bool f_xbox_post_message(CStrRef msg, CStrRef host /* = "localhost" */) {
  return XboxServer::PostMessage(msg, host);
}

Object f_xbox_task_start(CStrRef message) {
  return XboxServer::TaskStart(message);
}

bool f_xbox_task_status(CObjRef task) {
  return XboxServer::TaskStatus(task);
}

int64 f_xbox_task_result(CObjRef task, int64 timeout_ms, Variant ret) {
  return XboxServer::TaskResult(task, timeout_ms, ret);
}

int f_xbox_get_thread_timeout() {
  XboxServerInfoPtr server_info = XboxServer::GetServerInfo();
  if (server_info) {
    return server_info->getMaxDuration();
  }
  throw Exception("Not an xbox worker!");
}

void f_xbox_set_thread_timeout(int timeout) {
  if (timeout < 0) {
    raise_warning("Cannot set timeout/duration to a negative number.");
    return;
  }
  XboxServerInfoPtr server_info = XboxServer::GetServerInfo();
  if (server_info) {
    server_info->setMaxDuration(timeout);
  } else {
    throw Exception("Not an xbox worker!");
  }
}

void f_xbox_schedule_thread_reset() {
  RPCRequestHandler *handler = XboxServer::GetRequestHandler();
  if (handler) {
    handler->setReset();
  } else {
    throw Exception("Not an xbox worker!");
  }
}

int f_xbox_get_thread_time() {
  RPCRequestHandler *handler = XboxServer::GetRequestHandler();
  if (handler) {
    return time(NULL) - handler->getCreationTime();
  }
  throw Exception("Not an xbox worker!");
}

///////////////////////////////////////////////////////////////////////////////
}
