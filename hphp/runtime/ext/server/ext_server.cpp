/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/server/ext_server.h"

#include <folly/Conv.h>

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/xbox-request-handler.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/util/boot-stats.h"

namespace HPHP {

static struct ServerExtension final : Extension {
  ServerExtension() : Extension("server", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleInit() override;
} s_server_extension;

int64_t HHVM_FUNCTION(hphp_thread_type) {
  Transport *transport = g_context->getTransport();
  return transport ? static_cast<int64_t>(transport->getThreadType()) : -1;
}

///////////////////////////////////////////////////////////////////////////////
// Pagelet Server

bool HHVM_FUNCTION(pagelet_server_is_enabled) {
  return PageletServer::Enabled();
}

const StaticString s_Host("Host");

OptResource HHVM_FUNCTION(pagelet_server_task_start,
                       const String& url,
                       const Array& headers /* = null_array */,
                       const String& post_data /* = null_string */,
                       const Array& files /* = null_array */,
                       int64_t desired_timeout /* = 0 */) {
  String remote_host;
  Transport *transport = g_context->getTransport();
  // If a non-zero timeout is requested, use it and cap it at the remaining
  // request time.
  int remaining_time =
    RequestInfo::s_requestInfo->m_reqInjectionData.getRemainingTime();
  int timeout = desired_timeout > 0 && desired_timeout <= remaining_time
    ? desired_timeout
    : remaining_time;

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

int64_t HHVM_FUNCTION(pagelet_server_task_status,
                      const OptResource& task) {
  return PageletServer::TaskStatus(task);
}

String HHVM_FUNCTION(pagelet_server_task_result,
                     const OptResource& task,
                     Array& headers,
                     int64_t& code,
                     int64_t timeout_ms /* = 0 */) {
  int rcode;
  String response = PageletServer::TaskResult(task, headers, rcode,
                                              timeout_ms);
  code = rcode;
  return response;
}

int64_t HHVM_FUNCTION(pagelet_server_tasks_started) {
  return g_context->getPageletTasksStarted();
}

void HHVM_FUNCTION(pagelet_server_flush) {
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

bool HHVM_FUNCTION(pagelet_server_is_done) {
  PageletTransport *job =
    dynamic_cast<PageletTransport *>(g_context->getTransport());
  if (job) {
    return job->isDone();
  }
  // if we aren't in a pagelet thread, this call is meaningless
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// xbox

OptResource HHVM_FUNCTION(xbox_task_start,
                       const String& message) {
  return XboxServer::TaskStart(message);
}

bool HHVM_FUNCTION(xbox_task_status,
                   const OptResource& task) {
  return XboxServer::TaskStatus(task);
}

int64_t HHVM_FUNCTION(xbox_task_result,
                      const OptResource& task,
                      int64_t timeout_ms,
                      Variant& ret) {
  auto result = XboxServer::TaskResult(task, timeout_ms, &ret);
  return result;
}

Variant HHVM_FUNCTION(xbox_process_call_message,
                      const String& msg) {
  Variant v =
    unserialize_from_string(msg, VariableUnserializer::Type::Internal);
  if (!v.isArray()) {
    raise_error("Error decoding xbox call message");
  }
  Array arr = v.toArray();
  if (arr.size() != 2 || !arr.exists(0) || !arr.exists(1)) {
    raise_error("Error decoding xbox call message");
  }
  Variant fn = arr[0];
  if (fn.isArray()) {
    Array farr = fn.toArray();
    if (!array_is_valid_callback(farr)) {
      raise_error("Error decoding xbox call message");
    }
  } else if (!fn.isString()) {
    raise_error("Error decoding xbox call message");
  }
  Variant args = arr[1];
  if (!args.isArray()) {
    raise_error("Error decoding xbox call message");
  }
  return vm_call_user_func(fn, args.toArray(), RuntimeCoeffects::fixme(),
                           false, true);
}

bool HHVM_FUNCTION(server_is_stopping) {
  if (HttpServer::Server) {
    if (auto const server = HttpServer::Server->getPageServer()) {
      return server->getStatus() == Server::RunStatus::STOPPING;
    }
  }
  // Return false if not running in server mode.
  return false;
}

bool HHVM_FUNCTION(server_is_prepared_to_stop) {
  if (isJitSerializing()) return true;
  auto const now = time(nullptr);
  auto const lastPrepareTime = HttpServer::GetPrepareToStopTime();
  if (lastPrepareTime == 0) return false;
  return (lastPrepareTime + RuntimeOption::ServerPrepareToStopTimeout) >= now;
}

int64_t HHVM_FUNCTION(server_health_level) {
  if (HttpServer::Server) {
    if (auto const server = HttpServer::Server->getPageServer()) {
      return healthLevelToInt(server->getHealthLevel());
    }
  }
  // If server is not yet started, e.g., when not running in server
  // mode, or before server starts, we assume everything is OK.
  return healthLevelToInt(HealthLevel::Bold);
}

int64_t HHVM_FUNCTION(server_uptime) {
  // return -1 if server is not yet started, e.g., when not running in
  // server mode.
  if (HttpServer::StartTime == 0) return -1;
  int64_t nSeconds = time(nullptr) - HttpServer::StartTime;
  if (nSeconds < 0) nSeconds = 0;
  return nSeconds;
}

int64_t HHVM_FUNCTION(server_process_start_time) {
  // Returns 0 when not running in server mode.
  return BootStats::startTimestamp();
}

void ServerExtension::moduleInit() {
  HHVM_RC_INT_SAME(PAGELET_NOT_READY);
  HHVM_RC_INT_SAME(PAGELET_READY);
  HHVM_RC_INT_SAME(PAGELET_DONE);

  HHVM_FE(hphp_thread_type);
  HHVM_FE(pagelet_server_is_enabled);
  HHVM_FE(pagelet_server_task_start);
  HHVM_FE(pagelet_server_task_status);
  HHVM_FE(pagelet_server_task_result);
  HHVM_FE(pagelet_server_tasks_started);
  HHVM_FE(pagelet_server_flush);
  HHVM_FE(pagelet_server_is_done);
  HHVM_FE(xbox_task_start);
  HHVM_FE(xbox_task_status);
  HHVM_FE(xbox_task_result);
  HHVM_FE(xbox_process_call_message);
  HHVM_FALIAS(HH\\server_is_stopping, server_is_stopping);
  HHVM_FALIAS(HH\\server_is_prepared_to_stop, server_is_prepared_to_stop);
  HHVM_FALIAS(HH\\server_health_level, server_health_level);
  HHVM_FALIAS(HH\\server_uptime, server_uptime);
  HHVM_FALIAS(HH\\server_process_start_time, server_process_start_time);
}

///////////////////////////////////////////////////////////////////////////////
}
