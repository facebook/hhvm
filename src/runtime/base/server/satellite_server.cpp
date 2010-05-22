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

#include <runtime/base/server/satellite_server.h>
#include <runtime/base/server/libevent_server.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/rpc_request_handler.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/preg.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::set<std::string> SatelliteServerInfo::InternalURLs;
int SatelliteServerInfo::DanglingServerPort = 0;

SatelliteServerInfo::SatelliteServerInfo(Hdf hdf) {
  m_name = hdf.getName();
  m_port = hdf["Port"].getInt16(0);
  m_threadCount = hdf["ThreadCount"].getInt32(5);
  m_maxRequest = hdf["MaxRequest"].getInt32(500);
  m_maxDuration = hdf["MaxDuration"].getInt32(120);
  m_timeoutSeconds =
    hdf["TimeoutSeconds"].getInt32(RuntimeOption::RequestTimeoutSeconds);
  m_warmupDoc = hdf["WarmupDocument"].getString("");
  m_reqInitFunc = hdf["RequestInitFunction"].getString("");
  m_password = hdf["Password"].getString("");

  string type = hdf["Type"];
  if (type == "InternalPageServer") {
    m_type = SatelliteServer::KindOfInternalPageServer;
    vector<string> urls;
    hdf["URLs"].get(urls);
    for (unsigned int i = 0; i < urls.size(); i++) {
      m_urls.insert(format_pattern(urls[i]));
    }
    if (hdf["BlockMainServer"].getBool(true)) {
      InternalURLs.insert(m_urls.begin(), m_urls.end());
    }
  } else if (type == "DanglingPageServer") {
    m_type = SatelliteServer::KindOfDanglingPageServer;
    DanglingServerPort = m_port;
  } else if (type == "RPCServer") {
    m_type = SatelliteServer::KindOfRPCServer;
  } else {
    m_type = SatelliteServer::UnknownType;
  }
}

///////////////////////////////////////////////////////////////////////////////
// InternalPageServer: LibEventServer + allowed URL checking

DECLARE_BOOST_TYPES(InternalPageServerImpl);
class InternalPageServerImpl : public LibEventServer {
public:
  InternalPageServerImpl(const std::string &address, int port, int thread,
                         int timeoutSeconds) :
    LibEventServer(address, port, thread, timeoutSeconds) {
  }
  void create(const std::set<std::string> &urls) {
    m_allowedURLs = urls;
  }

  virtual bool shouldHandle(const std::string &cmd) {
    String url(cmd.c_str(), cmd.size(), AttachLiteral);
    for (set<string>::const_iterator iter = m_allowedURLs.begin();
         iter != m_allowedURLs.end(); ++iter) {
      Variant ret = preg_match
        (String(iter->c_str(), iter->size(), AttachLiteral), url);
      if (ret.toInt64() > 0) {
        return true;
      }
    }
    return false;
  }

private:
  std::set<std::string> m_allowedURLs;
};

class InternalPageServer : public SatelliteServer {
public:
  InternalPageServer(SatelliteServerInfoPtr info) {
    InternalPageServerImplPtr server
      (new TypedServer<InternalPageServerImpl, HttpRequestHandler>
       (RuntimeOption::ServerIP, info->getPort(), info->getThreadCount(),
        info->getTimeoutSeconds()));
    server->create(info->getURLs());
    m_server = server;
  }

  virtual void start() {
    m_server->start();
  }
  virtual void stop() {
    m_server->stop();
    m_server->waitForEnd();
  }
private:
  ServerPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
// DanglingPageServer: same as LibEventServer

class DanglingPageServer : public SatelliteServer {
public:
  DanglingPageServer(SatelliteServerInfoPtr info) {
    m_server = ServerPtr
      (new TypedServer<LibEventServer, HttpRequestHandler>
       (RuntimeOption::ServerIP, info->getPort(), info->getThreadCount(),
        info->getTimeoutSeconds()));
  }

  virtual void start() {
    m_server->start();
  }
  virtual void stop() {
    m_server->stop();
    m_server->waitForEnd();
  }
private:
  ServerPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
// RPCServer: LibEventServer + RPCRequestHandler

static IMPLEMENT_THREAD_LOCAL(RPCRequestHandler, s_rpc_request_handler);

class RPCServerImpl : public LibEventServer {
public:
  RPCServerImpl(const std::string &address, SatelliteServerInfoPtr info)
    : LibEventServer(address, info->getPort(), info->getThreadCount(),
                     info->getTimeoutSeconds()),
      m_serverInfo(info) {
  }

  virtual RequestHandler *createRequestHandler() {
    s_rpc_request_handler->setServerInfo(m_serverInfo);
    if (s_rpc_request_handler->needReset() ||
        s_rpc_request_handler->incRequest() > m_serverInfo->getMaxRequest()) {
      s_rpc_request_handler.reset();
      s_rpc_request_handler->setServerInfo(m_serverInfo);
      s_rpc_request_handler->incRequest();
    }
    return s_rpc_request_handler.get();
  }

  virtual void releaseRequestHandler(RequestHandler *handler) {
    // do nothing
  }

  virtual void onThreadExit(RequestHandler *handler) {
    s_rpc_request_handler.reset();
  }

private:
  SatelliteServerInfoPtr m_serverInfo;
};

class RPCServer : public SatelliteServer {
public:
  RPCServer(SatelliteServerInfoPtr info) {
    m_server = ServerPtr(new RPCServerImpl(RuntimeOption::ServerIP, info));
  }

  virtual void start() {
    m_server->start();
  }
  virtual void stop() {
    m_server->stop();
    m_server->waitForEnd();
  }
private:
  ServerPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
// SatelliteServer

SatelliteServerPtr SatelliteServer::Create(SatelliteServerInfoPtr info) {
  SatelliteServerPtr satellite;
  switch (info->getType()) {
  case KindOfInternalPageServer:
    satellite = SatelliteServerPtr(new InternalPageServer(info));
    break;
  case KindOfDanglingPageServer:
    satellite = SatelliteServerPtr(new DanglingPageServer(info));
    break;
  case KindOfRPCServer:
    satellite = SatelliteServerPtr(new RPCServer(info));
    break;
  default:
    ASSERT(false);
  }
  if (satellite) {
    satellite->setName(info->getName());
  }
  return satellite;
}

///////////////////////////////////////////////////////////////////////////////
}
