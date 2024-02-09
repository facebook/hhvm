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

#include "hphp/runtime/server/xbox-request-handler.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/std/ext_std_output.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/util/process.h"
#include "hphp/util/stack-trace.h"

#include <folly/ScopeGuard.h>


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

THREAD_LOCAL(AccessLog::ThreadData, XboxRequestHandler::s_accessLogThreadData);

AccessLog XboxRequestHandler::s_accessLog(
  &(XboxRequestHandler::getAccessLogThreadData));

XboxRequestHandler::XboxRequestHandler()
  : RequestHandler(RuntimeOption::RequestTimeoutSeconds),
    m_logResets(false) {
}

XboxRequestHandler::~XboxRequestHandler() {
  assertx(!vmStack().isAllocated());
}

void XboxRequestHandler::initState(RequestId requestId) {
  hphp_session_init(Treadmill::SessionKind::RpcRequest, nullptr, requestId);
  m_context = g_context.getNoCheck();
  if (!is_any_cli_mode() && !m_cli) {
    m_context->obStart(uninit_null(),
                       0,
                       OBFlags::Default | OBFlags::OutputDisabled);
    m_context->obProtect(true);
  } else {
    // In command line mode, we want the xbox workers to
    // output to STDOUT
    m_context->obSetImplicitFlush(true);
    m_context->obStart(uninit_null(),
                       0,
                       OBFlags::Default | OBFlags::WriteToStdout);
    m_context->obProtect(true);
  }
  m_lastReset = time(0);

  if (m_logResets) {
    Logger::Info("initializing Xbox request handler");
  }
}

void XboxRequestHandler::setLogInfo(bool logInfo) {
  m_logResets = logInfo;
}

void XboxRequestHandler::teardownRequest(Transport*) noexcept {
  if (!vmStack().isAllocated()) return;

  hphp_context_exit();
  hphp_session_exit();
}

void XboxRequestHandler::handleRequest(Transport *transport) {
  auto const requestId = RequestId::allocate();
  initState(requestId);

  ExecutionProfiler ep(RequestInfo::RuntimeFunctions);

  Logger::OnNewRequest(requestId.id());
  GetAccessLog().onNewRequest();
  m_context->setTransport(transport);
  transport->enableCompression();
  InitFiniNode::RequestStart();

  ServerStatsHelper ssh("all", ServerStatsHelper::TRACK_MEMORY);
  Logger::Verbose("receiving %s", transport->getCommand().c_str());

  // will clear all extra logging when this function goes out of scope
  StackTraceNoHeap::ExtraLoggingClearer clearer;
  StackTraceNoHeap::AddExtraLogging("RPC-URL", transport->getUrl());

  // resolve virtual host
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assertx(vhost);
  if (vhost->disabled()) {
    transport->sendString("Virtual host disabled.", 404);
    transport->onSendEnd();
    GetAccessLog().log(transport, vhost);
    return;
  }

  auto& reqData = RequestInfo::s_requestInfo->m_reqInjectionData;
  reqData.setTimeout(vhost->getRequestTimeoutSeconds(getDefaultTimeout()));
  SCOPE_EXIT {
    reqData.setTimeout(0);  // can't throw when you pass zero
    reqData.setCPUTimeout(0);
    reqData.reset();
  };

  // resolve source root
  SourceRootInfo::WithRoot sri(transport);

  // set thread type
  if (m_serverInfo->getType() == SatelliteServer::Type::KindOfXboxServer) {
    transport->setThreadType(Transport::ThreadType::XboxThread);
  }

  // record request for debugging purpose
  bool ret;
  std::string tmpfile = HttpProtocol::RecordRequest(transport);
  if (m_cli) {
    cli_invoke(
      std::move(m_cli).value(),
      [&] (const std::string& prelude) {
        ret = executePHPFunction(transport);
      }
    );
  } else {
    ret = executePHPFunction(transport);
  }
  GetAccessLog().log(transport, vhost);
  /*
   * HPHP logs may need to access data in ServerStats, so we have to
   * clear the hashtable after writing the log entry.
   */
  ServerStats::Reset();
  HttpProtocol::ClearRecord(ret, tmpfile);
}

void XboxRequestHandler::abortRequest(Transport *transport) {
  g_context.getCheck();
  GetAccessLog().onNewRequest();
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assertx(vhost);
  transport->sendString("Service Unavailable", 503);
  GetAccessLog().log(transport, vhost);
  if (!vmStack().isAllocated()) {
    hphp_memory_cleanup();
  }
}

const StaticString
  s_output("output"),
  s_return("return"),
  s_HPHP_RPC("HPHP_RPC"),
  s__ENV("_ENV");

bool XboxRequestHandler::executePHPFunction(Transport *transport) {
  std::string rpcFunc = transport->getCommand();
  {
    ServerStatsHelper ssh("input");
    RequestURI reqURI(rpcFunc);
    HttpProtocol::PrepareSystemVariables(transport, reqURI);
    auto env = php_global(s__ENV);
    env.asArrRef().set(s_HPHP_RPC, 1);
    php_global_set(s__ENV, std::move(env));
  }

  Array params;
  // single string parameter, used by xbox to avoid any en/decoding
  size_t size;
  const void *data = transport->getPostData(size);
  if (data && size) {
    params.append(String((char*)data, size, CopyString));
  }

  int code;

  Variant funcRet;
  std::string errorMsg = "Internal Server Error";
  std::string reqInitFunc, reqInitDoc;

  reqInitDoc = transport->getHeader("ReqInitDoc");
  if (reqInitDoc.empty() && m_serverInfo) {
    reqInitFunc = m_serverInfo->getReqInitFunc();
    reqInitDoc = m_serverInfo->getReqInitDoc();
  }

  if (!reqInitDoc.empty()) {
    reqInitDoc = (std::string)canonicalize_path(reqInitDoc, "", 0);
  }
  if (!reqInitDoc.empty()) {
    reqInitDoc = getSourceFilename(reqInitDoc);
  }

  bool ret = true;
  bool error = false;
  if (!rpcFunc.empty()) {
    ret = hphp_invoke(m_context, rpcFunc, true, params, &funcRet,
                      reqInitFunc, reqInitDoc, error, errorMsg,
                      true /* once */,
                      false /* warmupOnly */,
                      false /* richErrorMessage */,
                      RuntimeOption::EvalPreludePath,
                      true /* allowDynCallNoPointer */);
  }
  if (ret) {
    bool serializeFailed = false;
    String response;
    try {
      response = internal_serialize(funcRet);
    } catch (...) {
      serializeFailed = true;
    }

    if (serializeFailed) {
      code = 500;
      transport->sendString(
          "Serialization of the return value failed", 500);
    } else {
      transport->sendRaw(response.data(), response.size());
      code = transport->getResponseCode();
    }
  } else if (error) {
    code = 500;
    transport->sendString(errorMsg, 500);
  } else {
    code = 404;
    transport->sendString("Not Found", 404);
  }

  params.reset();

  transport->onSendEnd();
  ServerStats::LogPage(rpcFunc, code);

  m_context->onShutdownPostSend();
  // in case postsend/cleanup output something
  // PHP5 always provides _START.
  m_context->obClean(k_PHP_OUTPUT_HANDLER_START |
                     k_PHP_OUTPUT_HANDLER_CLEAN |
                     k_PHP_OUTPUT_HANDLER_END);
  m_context->restoreSession();
  // Context is long-lived, but cached transport is not, so clear it.
  m_context->setTransport(nullptr);
  return !error;
}

std::string XboxRequestHandler::getSourceFilename(const std::string &path) {
  if (path.empty() || path[0] == '/') return path;
  // If it is not a sandbox, sourceRoot will be the same as
  // RuntimeOption::SourceRoot.
  std::string sourceRoot = SourceRootInfo::GetCurrentSourceRoot();
  if (sourceRoot.empty()) {
    return Process::GetCurrentDirectory() + "/" + path;
  }
  return sourceRoot + path;
}

///////////////////////////////////////////////////////////////////////////////
}
