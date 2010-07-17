/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
  if (m_reset) return true;
  return (time(0) - m_created) > m_serverInfo->getMaxDuration();
}

void RPCRequestHandler::handleRequest(Transport *transport) {
  Logger::OnNewRequest();
  transport->enableCompression();

  ServerStatsHelper ssh("all", true);
  Logger::Verbose("receiving %s", transport->getCommand().c_str());

  // will clear all extra logging when this function goes out of scope
  StackTraceNoHeap::ExtraLoggingClearer clearer;
  StackTraceNoHeap::AddExtraLogging("RPC-URL", transport->getUrl());

  // authentication
  const string &password = m_serverInfo->getPassword();
  if (!password.empty() && password != transport->getParam("auth")) {
    transport->sendString("Unauthorized", 401);
    transport->onSendEnd();
    return;
  }

  // resolve virtual host
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  ASSERT(vhost);
  if (vhost->disabled()) {
    transport->sendString("Virtual host disabled.", 404);
    transport->onSendEnd();
    return;
  }

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

  std::string rpcFunc = transport->getCommand();
  {
    ServerStatsHelper ssh("input");
    RequestURI reqURI(rpcFunc);
    HttpProtocol::PrepareSystemVariables(transport, reqURI, sourceRootInfo);
    sourceRootInfo.clear();
  }

  bool error = false;

  Array params;
  std::string sparams = transport->getParam("params");
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
    std::string errorMsg = "Internal Server Error";
    string warmupDoc, reqInitFunc;
    if (m_serverInfo) {
      warmupDoc = m_serverInfo->getWarmupDoc();
      reqInitFunc = m_serverInfo->getReqInitFunc();
    }
    if (warmupDoc.empty()) {
      warmupDoc = RuntimeOption::WarmupDocument;
      reqInitFunc = RuntimeOption::RequestInitFunction;
    }
    if (!warmupDoc.empty()) warmupDoc = canonicalize_path(warmupDoc, "", 0);
    if (!warmupDoc.empty()) warmupDoc = get_source_filename(warmupDoc.c_str());
    bool ret = hphp_invoke(m_context, rpcFunc, true, params, ref(funcRet),
                           warmupDoc, reqInitFunc, error, errorMsg);
    if (ret) {
      String response;
      switch (output) {
        case 0: response = f_json_encode(funcRet);   break;
        case 1: response = m_context->obDetachContents(); break;
        case 2:
          response =
            f_json_encode(CREATE_MAP2("output", m_context->obDetachContents(),
                                      "return", f_json_encode(funcRet)));
          break;
      }
      code = 200;
      transport->sendRaw((void*)response.data(), response.size());
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

  transport->onSendEnd();
  ServerStats::LogPage(rpcFunc, code);

  m_context->onShutdownPostSend();
  m_context->restoreSession();
  return !error;
}

///////////////////////////////////////////////////////////////////////////////
}
