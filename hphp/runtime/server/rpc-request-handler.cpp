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

#include "hphp/runtime/server/rpc-request-handler.h"

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

THREAD_LOCAL(AccessLog::ThreadData, RPCRequestHandler::s_accessLogThreadData);

AccessLog RPCRequestHandler::s_accessLog(
  &(RPCRequestHandler::getAccessLogThreadData));

RPCRequestHandler::RPCRequestHandler(int timeout, bool info)
  : RequestHandler(timeout),
    m_requestsSinceReset(0),
    m_reset(false),
    m_logResets(info),
    m_returnEncodeType(ReturnEncodeType::Json) {
}

RPCRequestHandler::~RPCRequestHandler() {
  if (vmStack().isAllocated()) cleanupState();
}

void RPCRequestHandler::initState() {
  hphp_session_init(Treadmill::SessionKind::RpcRequest);
  m_context = g_context.getNoCheck();
  if (!is_any_cli_mode()) {
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

  Logger::ResetRequestCount();
  if (m_logResets) {
    Logger::Info("initializing RPC request handler");
  }

  m_reset = false;
  m_requestsSinceReset = 0;
}

void RPCRequestHandler::cleanupState() {
  hphp_context_exit();
  hphp_session_exit();
}

bool RPCRequestHandler::needReset() const {
  return (m_reset ||
          !vmStack().isAllocated() ||
          m_serverInfo->alwaysReset() ||
          ((time(0) - m_lastReset) > m_serverInfo->getMaxDuration()) ||
          (m_requestsSinceReset >= m_serverInfo->getMaxRequest()));
}

bool RPCRequestHandler::ignoreParams() const {
  // Xbox requests should not try to extract param information from the call
  // message we pass through the POST data
  return m_serverInfo->getType() == SatelliteServer::Type::KindOfXboxServer;
}

void RPCRequestHandler::handleRequest(Transport *transport) {
  if (needReset()) {
    if (vmStack().isAllocated()) cleanupState();
    initState();
  }
  ++m_requestsSinceReset;

  ExecutionProfiler ep(RequestInfo::RuntimeFunctions);

  Logger::OnNewRequest();
  GetAccessLog().onNewRequest();
  m_context->setTransport(transport);
  transport->enableCompression();
  InitFiniNode::RequestStart();

  ServerStatsHelper ssh("all", ServerStatsHelper::TRACK_MEMORY);
  Logger::Verbose("receiving %s", transport->getCommand().c_str());

  // will clear all extra logging when this function goes out of scope
  StackTraceNoHeap::ExtraLoggingClearer clearer;
  StackTraceNoHeap::AddExtraLogging("RPC-URL", transport->getUrl());

  // Checking functions whitelist
  const std::set<std::string> &functions = m_serverInfo->getFunctions();
  if (!functions.empty()) {
    auto iter = functions.find(transport->getCommand());
    if (iter == functions.end()) {
      transport->sendString("Forbidden", 403);
      transport->onSendEnd();
      GetAccessLog().log(transport, nullptr);
      /*
       * HPHP logs may need to access data in ServerStats, so we have to
       * clear the hashtable after writing the log entry.
       */
      ServerStats::Reset();
      return;
    }
  }

  // authentication
  const std::set<std::string> &passwords = m_serverInfo->getPasswords();
  if (!passwords.empty()) {
    auto iter = passwords.find(
      transport->getParam("auth", m_serverInfo->getMethod())
    );
    if (iter == passwords.end()) {
      transport->sendString("Unauthorized", 401);
      transport->onSendEnd();
      GetAccessLog().log(transport, nullptr);
      /*
       * HPHP logs may need to access data in ServerStats, so we have to
       * clear the hashtable after writing the log entry.
       */
      ServerStats::Reset();
      return;
    }
  } else {
    const std::string &password = m_serverInfo->getPassword();
    if (!password.empty() &&
      password != transport->getParam("auth", m_serverInfo->getMethod())
    ) {
      transport->sendString("Unauthorized", 401);
      transport->onSendEnd();
      GetAccessLog().log(transport, nullptr);
      /*
       * HPHP logs may need to access data in ServerStats, so we have to
       * clear the hashtable after writing the log entry.
       */
      ServerStats::Reset();
      return;
    }
  }

  // return encoding type
  ReturnEncodeType returnEncodeType = m_returnEncodeType;
  if (!ignoreParams() &&
      transport->getParam("return", m_serverInfo->getMethod()) == "serialize") {
    returnEncodeType = ReturnEncodeType::Serialize;
  }

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
  SourceRootInfo sourceRootInfo(transport);

  // set thread type
  switch (m_serverInfo->getType()) {
  case SatelliteServer::Type::KindOfRPCServer:
    transport->setThreadType(Transport::ThreadType::RpcThread);
    break;
  case SatelliteServer::Type::KindOfXboxServer:
    transport->setThreadType(Transport::ThreadType::XboxThread);
    break;
  default:
    break;
  }

  // record request for debugging purpose
  std::string tmpfile = HttpProtocol::RecordRequest(transport);
  bool ret = executePHPFunction(transport, sourceRootInfo, returnEncodeType);
  GetAccessLog().log(transport, vhost);
  /*
   * HPHP logs may need to access data in ServerStats, so we have to
   * clear the hashtable after writing the log entry.
   */
  ServerStats::Reset();
  HttpProtocol::ClearRecord(ret, tmpfile);
}

void RPCRequestHandler::abortRequest(Transport *transport) {
  g_context.getCheck();
  GetAccessLog().onNewRequest();
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assertx(vhost);
  transport->sendString("Service Unavailable", 503);
  GetAccessLog().log(transport, vhost);
  if (!vmStack().isAllocated()) {
    hphp_memory_cleanup();
  }
  m_reset = true;
}

const StaticString
  s_output("output"),
  s_return("return"),
  s_HPHP_RPC("HPHP_RPC"),
  s__ENV("_ENV");

bool RPCRequestHandler::executePHPFunction(Transport *transport,
                                           SourceRootInfo &sourceRootInfo,
                                           ReturnEncodeType returnEncodeType) {
  std::string rpcFunc = transport->getCommand();
  {
    ServerStatsHelper ssh("input");
    RequestURI reqURI(rpcFunc);
    HttpProtocol::PrepareSystemVariables(transport, reqURI, sourceRootInfo);
    auto env = php_global(s__ENV);
    env.asArrRef().set(s_HPHP_RPC, 1);
    php_global_set(s__ENV, std::move(env));
  }

  bool isFile = rpcFunc.rfind('.') != std::string::npos;
  std::string rpcFile;
  bool error = false;

  Array params;
  Transport::Method requestMethod = m_serverInfo->getMethod();

  if (ignoreParams()) {
    // single string parameter, used by xbox to avoid any en/decoding
    size_t size;
    const void *data = transport->getPostData(size);
    if (data && size) {
      params.append(String((char*)data, size, CopyString));
    }
  } else {
    std::string sparams = transport->getParam("params", requestMethod);
    if (!sparams.empty()) {
      auto jparams = Variant::attach(
        HHVM_FN(json_decode)(String(sparams), true)
      );
      if (jparams.isArray()) {
        params = jparams.toArray();
      } else {
        error = true;
      }
    } else {
      std::vector<std::string> sparams;
      transport->getArrayParam("p", sparams, requestMethod);
      for (unsigned int i = 0; i < sparams.size(); i++) {
        auto jparams = Variant::attach(
          HHVM_FN(json_decode)(String(sparams[i]), true)
        );
        if (same(jparams, false)) {
          error = true;
          break;
        }
        params.append(jparams);
      }
    }
  }

  int output = 0;
  if (!ignoreParams()) {
    if (transport->getIntParam("reset", requestMethod) == 1) {
      m_reset = true;
    }
    output = transport->getIntParam("output", requestMethod);
  }

  int code;
  if (!error) {
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
      reqInitDoc = getSourceFilename(reqInitDoc, sourceRootInfo);
    }

    bool runOnce = false;
    bool ret = true;
    if (isFile) {
      rpcFile = rpcFunc;
      rpcFunc.clear();
    } else if (!ignoreParams()) {
      rpcFile = transport->getParam("include", requestMethod);
      if (rpcFile.empty()) {
        rpcFile = transport->getParam("include_once", requestMethod);
        runOnce = true;
      }
    }
    if (!rpcFile.empty()) {
      // invoking a file through rpc
      bool forbidden = false;
      if (!RuntimeOption::ForbiddenFileExtensions.empty()) {
        const char *ext = rpcFile.c_str() + rpcFile.rfind('.') + 1;
        if (RuntimeOption::ForbiddenFileExtensions.find(ext) !=
            RuntimeOption::ForbiddenFileExtensions.end()) {
          forbidden = true;
        }
      }
      if (!forbidden) {
        rpcFile = (std::string) canonicalize_path(rpcFile, "", 0);
        rpcFile = getSourceFilename(rpcFile, sourceRootInfo);
        ret = hphp_invoke(m_context, rpcFile, false, Array(), nullptr,
                          reqInitFunc, reqInitDoc, error, errorMsg, runOnce,
                          false /* warmupOnly */,
                          false /* richErrorMessage */,
                          RuntimeOption::EvalPreludePath,
                          true /* allowDynCallNoPointer */);
      }
      // no need to do the initialization for a second time
      reqInitFunc.clear();
      reqInitDoc.clear();
    }
    if (ret && !rpcFunc.empty()) {
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
      switch (output) {
        case 0: {
          try {
            switch (returnEncodeType) {
              case ReturnEncodeType::Json:
                response = Variant::attach(HHVM_FN(json_encode)(funcRet)).toString();
                break;
              case ReturnEncodeType::Serialize:
                response = HHVM_FN(serialize)(funcRet);
                break;
              case ReturnEncodeType::Internal:
                response = internal_serialize(funcRet);
                break;
            }
          } catch (...) {
            serializeFailed = true;
          }
          break;
        }
        case 1: response = m_context->obDetachContents(); break;
        case 2:
          response = Variant::attach(HHVM_FN(json_encode)(
            make_dict_array(
              s_output,
              m_context->obDetachContents(),
              s_return,
              Variant::attach(HHVM_FN(json_encode)(funcRet))
            )
          )).toString();
          break;
        case 3: response = HHVM_FN(serialize)(funcRet); break;
      }
      if (serializeFailed) {
        code = 500;
        transport->sendString(
            "Serialization of the return value failed", 500);
        m_reset = true;
      } else {
        transport->sendRaw(response.data(), response.size());
        code = transport->getResponseCode();
      }
    } else if (error) {
      code = 500;
      transport->sendString(errorMsg, 500);
      m_reset = true;
    } else {
      code = 404;
      transport->sendString("Not Found", 404);
    }
  } else {
    code = 400;
    transport->sendString("Bad Request", 400);
  }

  params.reset();
  sourceRootInfo.clear();

  transport->onSendEnd();
  ServerStats::LogPage(isFile ? rpcFile : rpcFunc, code);

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

std::string RPCRequestHandler::getSourceFilename(const std::string &path,
                                            SourceRootInfo &sourceRootInfo) {
  if (path.empty() || path[0] == '/') return path;
  // If it is not a sandbox, sourceRoot will be the same as
  // RuntimeOption::SourceRoot.
  std::string sourceRoot = sourceRootInfo.path();
  if (sourceRoot.empty()) {
    return Process::GetCurrentDirectory() + "/" + path;
  }
  return sourceRoot + path;
}

///////////////////////////////////////////////////////////////////////////////
}
