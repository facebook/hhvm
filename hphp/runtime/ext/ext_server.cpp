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
#include "hphp/runtime/ext/ext_server.h"

#include <boost/lexical_cast.hpp>

#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/server/rpc-request-handler.h"

#define DANGLING_HEADER "HPHP_DANGLING"

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION_VERSION(server, NO_EXTENSION_VERSION_YET);

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

  std::string url = "http://localhost:" +
    boost::lexical_cast<std::string>(SatelliteServerInfo::DanglingServerPort) +
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

///////////////////////////////////////////////////////////////////////////////
// Pagelet Server

const int64_t k_PAGELET_NOT_READY = PAGELET_NOT_READY;
const int64_t k_PAGELET_READY     = PAGELET_READY;
const int64_t k_PAGELET_DONE      = PAGELET_DONE;

bool f_pagelet_server_is_enabled() {
  return PageletServer::Enabled();
}

const StaticString s_Host("Host");

Resource f_pagelet_server_task_start(const String& url,
                                     const Array& headers /* = null_array */,
                                     const String& post_data /* = null_string */,
                                     const Array& files /* = null_array */) {
  String remote_host;
  Transport *transport = g_context->getTransport();
  int timeout = ThreadInfo::s_threadInfo->m_reqInjectionData.getRemainingTime();
  if (transport) {
    remote_host = transport->getRemoteHost();
    if (!headers.exists(s_Host) && RuntimeOption::SandboxMode) {
      Array tmp = headers;
      tmp.set(s_Host, transport->getHeader("Host"));
      return PageletServer::TaskStart(url, tmp, remote_host,
                                      post_data, files, timeout);
    }
  }
  return PageletServer::TaskStart(url, headers, remote_host,
                                  post_data, files, timeout);
}

int64_t f_pagelet_server_task_status(const Resource& task) {
  return PageletServer::TaskStatus(task);
}

String f_pagelet_server_task_result(const Resource& task, VRefParam headers,
                                    VRefParam code,
                                    int64_t timeout_ms /* = 0 */) {
  Array rheaders;
  int rcode;
  String response = PageletServer::TaskResult(task, rheaders, rcode,
                                              timeout_ms);
  headers = rheaders;
  code = rcode;
  return response;
}

void f_pagelet_server_flush() {
  ExecutionContext *context = g_context.getNoCheck();
  Transport *transport = context->getTransport();
  if (transport &&
      transport->getThreadType() == Transport::ThreadType::PageletThread) {
    // this method is only meaningful in a pagelet thread
    context->obFlushAll();
    String content = context->obDetachContents();
    std::string s(content.data(), content.size());
    if (!s.empty()) {
      PageletServer::AddToPipeline(s);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// xbox

bool f_xbox_send_message(const String& msg,
                         VRefParam retRef,
                         int64_t timeout_ms,
                         const String& host /* = "localhost" */) {
  Array ret;
  auto b = XboxServer::SendMessage(msg, ret, timeout_ms, host);
  retRef = ret;
  return b;
}

bool f_xbox_post_message(const String& msg, const String& host /* = "localhost" */) {
  return XboxServer::PostMessage(msg, host);
}

Resource f_xbox_task_start(const String& message) {
  return XboxServer::TaskStart(message);
}

bool f_xbox_task_status(const Resource& task) {
  return XboxServer::TaskStatus(task);
}

int64_t f_xbox_task_result(const Resource& task, int64_t timeout_ms, VRefParam ret) {
  return XboxServer::TaskResult(task, timeout_ms, ret);
}

Variant f_xbox_process_call_message(const String& msg) {
  Variant v = unserialize_from_string(msg);
  if (!v.isArray()) {
    raise_error("Error decoding xbox call message");
  }
  Array arr = v.toArray();
  if (arr.size() != 2 || !arr.exists(0) || !arr.exists(1)) {
    raise_error("Error decoding xbox call message");
  }
  Variant fn = arr.rvalAt(0);
  if (fn.isArray()) {
    Array farr = fn.toArray();
    if (!array_is_valid_callback(farr)) {
      raise_error("Error decoding xbox call message");
    }
  } else if (!fn.isString()) {
    raise_error("Error decoding xbox call message");
  }
  Variant args = arr.rvalAt(1);
  if (!args.isArray()) {
    raise_error("Error decoding xbox call message");
  }
  return vm_call_user_func(fn, args.toArray());
}

int64_t f_xbox_get_thread_timeout() {
  auto server_info = XboxServer::GetServerInfo();
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
  auto server_info = XboxServer::GetServerInfo();
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

int64_t f_xbox_get_thread_time() {
  RPCRequestHandler *handler = XboxServer::GetRequestHandler();
  if (handler) {
    return time(nullptr) - handler->getLastResetTime();
  }
  throw Exception("Not an xbox worker!");
}

///////////////////////////////////////////////////////////////////////////////
}
