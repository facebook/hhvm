/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/server/rpc_request_handler.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/server/http_protocol.h>
#include <runtime/base/server/access_log.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/base/server/request_uri.h>
#include <runtime/ext/ext_json.h>
#include <util/process.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

RPCRequestHandler::RPCRequestHandler() : m_count(0), m_reset(false) {
  hphp_session_init();
  m_context = hphp_context_init();
  m_created = time(0);

  Logger::ResetRequestCount();
  Logger::Info("creating new RPC request handler");
}

RPCRequestHandler::~RPCRequestHandler() {
  hphp_context_exit(m_context, false);
  hphp_session_exit();
}

bool RPCRequestHandler::needReset() const {
  if (m_reset || m_serverInfo->alwaysReset()) return true;
  return (time(0) - m_created) > m_serverInfo->getMaxDuration();
}

void RPCRequestHandler::handleRequest(Transport *transport) {
  ExecutionProfiler ep(ThreadInfo::RuntimeFunctions);

  Logger::OnNewRequest();
  m_context->setTransport(transport);
  transport->enableCompression();

  ServerStatsHelper ssh("all", true);
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
      return;
    }
  } else {
    const string &password = m_serverInfo->getPassword();
    if (!password.empty() && password != transport->getParam("auth")) {
      transport->sendString("Unauthorized", 401);
      transport->onSendEnd();
      return;
    }
  }

  // resolve virtual host
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  ASSERT(vhost);
  if (vhost->disabled()) {
    transport->sendString("Virtual host disabled.", 404);
    transport->onSendEnd();
    return;
  }
  vhost->setRequestTimeoutSeconds();

  // resolve source root
  string host = transport->getHeader("Host");
  SourceRootInfo sourceRootInfo(host.c_str());

  // set thread type
  switch (m_serverInfo->getType()) {
  case SatelliteServer::KindOfRPCServer:
    transport->setThreadType(Transport::RpcThread);
    break;
  case SatelliteServer::KindOfXboxServer:
    transport->setThreadType(Transport::XboxThread);
    break;
  default:
    break;
  }

  // record request for debugging purpose
  std::string tmpfile = HttpProtocol::RecordRequest(transport);
  bool ret = executePHPFunction(transport, sourceRootInfo);
  HttpProtocol::ClearRecord(ret, tmpfile);
}

bool RPCRequestHandler::executePHPFunction(Transport *transport,
                                           SourceRootInfo &sourceRootInfo) {
  // reset timeout counter
  ThreadInfo::s_threadInfo->m_reqInjectionData.started = time(0);

  string rpcFunc = transport->getCommand();
  {
    ServerStatsHelper ssh("input");
    RequestURI reqURI(rpcFunc);
    HttpProtocol::PrepareSystemVariables(transport, reqURI, sourceRootInfo);
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
        params.append(String((char*)data, size, AttachLiteral));
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
    string warmupDoc, reqInitFunc, reqInitDoc;
    if (m_serverInfo) {
      warmupDoc = m_serverInfo->getWarmupDoc();
      reqInitFunc = m_serverInfo->getReqInitFunc();
      reqInitDoc = m_serverInfo->getReqInitDoc();
    }
    if (!warmupDoc.empty()) warmupDoc = canonicalize_path(warmupDoc, "", 0);
    if (!warmupDoc.empty()) {
      warmupDoc = getSourceFilename(warmupDoc, sourceRootInfo);
    }

    if (!reqInitDoc.empty()) reqInitDoc = canonicalize_path(reqInitDoc, "", 0);
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
        rpcFile = canonicalize_path(rpcFile, "", 0);
        rpcFile = getSourceFilename(rpcFile, sourceRootInfo);
        ret = hphp_invoke(m_context, rpcFile, false, Array(), null,
                          warmupDoc, reqInitFunc, reqInitDoc,
                          error, errorMsg, runOnce);
      }
      // no need to do the initialization for a second time
      warmupDoc.clear();
      reqInitFunc.clear();
      reqInitDoc.clear();
    }
    if (ret && !rpcFunc.empty()) {
      ret = hphp_invoke(m_context, rpcFunc, true, params, ref(funcRet),
                        warmupDoc, reqInitFunc, reqInitDoc,
                        error, errorMsg);
    }
    if (ret) {
      String response;
      switch (output) {
        case 0: response = f_json_encode(funcRet); break;
        case 1: response = m_context->obDetachContents(); break;
        case 2:
          response =
            f_json_encode(CREATE_MAP2("output", m_context->obDetachContents(),
                                      "return", f_json_encode(funcRet)));
          break;
        case 3: response = f_serialize(funcRet); break;
      }
      transport->sendRaw((void*)response.data(), response.size());
      code = transport->getResponseCode();
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
