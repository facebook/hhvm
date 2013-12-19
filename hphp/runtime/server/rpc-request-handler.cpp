/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/util/process.h"

#include "folly/ScopeGuard.h"

using std::set;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

RPCRequestHandler::RPCRequestHandler(int timeout, bool info)
  : RequestHandler(timeout),
    m_requestsSinceReset(0),
    m_reset(false),
    m_logResets(info),
    m_returnEncodeType(ReturnEncodeType::Json) {
  initState();
}

RPCRequestHandler::~RPCRequestHandler() {
  cleanupState();
}

void RPCRequestHandler::initState() {
  hphp_session_init();
  bool isServer = RuntimeOption::ServerExecutionMode();
  if (isServer) {
    m_context = hphp_context_init();
  } else {
    // In command line mode, we want the xbox workers to
    // output to STDOUT
    m_context = g_context.getNoCheck();
    m_context->obSetImplicitFlush(true);
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
  hphp_context_exit(m_context, false);
  hphp_session_exit();
}

bool RPCRequestHandler::needReset() const {
  return (m_reset ||
          m_serverInfo->alwaysReset() ||
          ((time(0) - m_lastReset) > m_serverInfo->getMaxDuration()) ||
          (m_requestsSinceReset >= m_serverInfo->getMaxRequest()));
}

void RPCRequestHandler::handleRequest(Transport *transport) {
  if (needReset()) {
    cleanupState();
    initState();
  }
  ++m_requestsSinceReset;

  ExecutionProfiler ep(ThreadInfo::RuntimeFunctions);

  Logger::OnNewRequest();
  HttpRequestHandler::GetAccessLog().onNewRequest();
  m_context->setTransport(transport);
  transport->enableCompression();

  ServerStatsHelper ssh("all", ServerStatsHelper::TRACK_MEMORY);
  Logger::Verbose("receiving %s", transport->getCommand().c_str());

  // will clear all extra logging when this function goes out of scope
  StackTraceNoHeap::ExtraLoggingClearer clearer;
  StackTraceNoHeap::AddExtraLogging("RPC-URL", transport->getUrl());

  // authentication
  const set<string> &passwords = m_serverInfo->getPasswords();
  if (!passwords.empty()) {
    set<string>::const_iterator iter =
      passwords.find(transport->getParam("auth"));
    if (iter == passwords.end()) {
      transport->sendString("Unauthorized", 401);
      transport->onSendEnd();
      HttpRequestHandler::GetAccessLog().log(transport, nullptr);
      /*
       * HPHP logs may need to access data in ServerStats, so we have to
       * clear the hashtable after writing the log entry.
       */
      ServerStats::Reset();
      return;
    }
  } else {
    const string &password = m_serverInfo->getPassword();
    if (!password.empty() && password != transport->getParam("auth")) {
      transport->sendString("Unauthorized", 401);
      transport->onSendEnd();
      HttpRequestHandler::GetAccessLog().log(transport, nullptr);
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
  if (transport->getParam("return") == "serialize") {
    returnEncodeType = ReturnEncodeType::Serialize;
  }

  // resolve virtual host
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assert(vhost);
  if (vhost->disabled()) {
    transport->sendString("Virtual host disabled.", 404);
    transport->onSendEnd();
    HttpRequestHandler::GetAccessLog().log(transport, vhost);
    return;
  }

  auto& reqData = ThreadInfo::s_threadInfo->m_reqInjectionData;
  reqData.setTimeout(vhost->getRequestTimeoutSeconds(getDefaultTimeout()));
  SCOPE_EXIT {
    reqData.setTimeout(0);
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
  HttpRequestHandler::GetAccessLog().log(transport, vhost);
  /*
   * HPHP logs may need to access data in ServerStats, so we have to
   * clear the hashtable after writing the log entry.
   */
  ServerStats::Reset();
  HttpProtocol::ClearRecord(ret, tmpfile);
}

void RPCRequestHandler::abortRequest(Transport *transport) {
  HttpRequestHandler::GetAccessLog().onNewRequest();
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assert(vhost);
  transport->sendString("Service Unavailable", 503);
  HttpRequestHandler::GetAccessLog().log(transport, vhost);
}

const StaticString
  s_output("output"),
  s_return("return"),
  s_HPHP_RPC("HPHP_RPC"),
  s__ENV("_ENV");

bool RPCRequestHandler::executePHPFunction(Transport *transport,
                                           SourceRootInfo &sourceRootInfo,
                                           ReturnEncodeType returnEncodeType) {
  string rpcFunc = transport->getCommand();
  {
    ServerStatsHelper ssh("input");
    RequestURI reqURI(rpcFunc);
    HttpProtocol::PrepareSystemVariables(transport, reqURI, sourceRootInfo);

    GlobalVariables *g = get_global_variables();
    g->getRef(s__ENV).set(s_HPHP_RPC, 1);
  }

  bool isFile = rpcFunc.rfind('.') != string::npos;
  string rpcFile;
  bool error = false;

  Array params;
  string sparams = transport->getParam("params");
  if (!sparams.empty()) {
    Variant jparams = f_json_decode(String(sparams), true);
    if (jparams.isArray()) {
      params = jparams.toArray();
    } else {
      error = true;
    }
  } else {
    vector<string> sparams;
    transport->getArrayParam("p", sparams);
    if (!sparams.empty()) {
      for (unsigned int i = 0; i < sparams.size(); i++) {
        Variant jparams = f_json_decode(String(sparams[i]), true);
        if (same(jparams, false)) {
          error = true;
          break;
        }
        params.append(jparams);
      }
    } else {
      // single string parameter, used by xbox to avoid any en/decoding
      int size;
      const void *data = transport->getPostData(size);
      if (data && size) {
        params.append(String((char*)data, size, CopyString));
      }
    }
  }

  if (transport->getIntParam("reset") == 1) {
    m_reset = true;
  }
  int output = transport->getIntParam("output");

  int code;
  if (!error) {
    Variant funcRet;
    string errorMsg = "Internal Server Error";
    string reqInitFunc, reqInitDoc;
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
    } else {
      rpcFile = transport->getParam("include");
      if (rpcFile.empty()) {
        rpcFile = transport->getParam("include_once");
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
        ret = hphp_invoke(m_context, rpcFile, false, Array(), uninit_null(),
                          reqInitFunc, reqInitDoc, error, errorMsg, runOnce);
      }
      // no need to do the initialization for a second time
      reqInitFunc.clear();
      reqInitDoc.clear();
    }
    if (ret && !rpcFunc.empty()) {
      ret = hphp_invoke(m_context, rpcFunc, true, params, ref(funcRet),
                        reqInitFunc, reqInitDoc, error, errorMsg);
    }
    if (ret) {
      bool serializeFailed = false;
      String response;
      switch (output) {
        case 0: {
          assert(returnEncodeType == ReturnEncodeType::Json ||
                 returnEncodeType == ReturnEncodeType::Serialize);
          try {
            response = (returnEncodeType == ReturnEncodeType::Json) ?
                       f_json_encode(funcRet) :
                       f_serialize(funcRet);
          } catch (...) {
            serializeFailed = true;
          }
          break;
        }
        case 1: response = m_context->obDetachContents(); break;
        case 2:
          response =
            f_json_encode(
              make_map_array(s_output, m_context->obDetachContents(),
                                      s_return, f_json_encode(funcRet)));
          break;
        case 3: response = f_serialize(funcRet); break;
      }
      if (serializeFailed) {
        code = 500;
        transport->sendString(
            "Serialization of the return value failed", 500);
        m_reset = true;
      } else {
        transport->sendRaw((void*)response.data(), response.size());
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
  m_context->obClean(); // in case postsend/cleanup output something
  m_context->restoreSession();
  return !error;
}

string RPCRequestHandler::getSourceFilename(const string &path,
                                            SourceRootInfo &sourceRootInfo) {
  if (path.empty() || path[0] == '/') return path;
  // If it is not a sandbox, sourceRoot will be the same as
  // RuntimeOption::SourceRoot.
  string sourceRoot = sourceRootInfo.path();
  if (sourceRoot.empty()) {
    return Process::GetCurrentDirectory() + "/" + path;
  }
  return sourceRoot + path;
}

///////////////////////////////////////////////////////////////////////////////
}
