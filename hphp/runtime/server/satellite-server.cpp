/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/util/util.h"
#include "folly/Memory.h"

using folly::make_unique;
using std::set;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::set<std::string> SatelliteServerInfo::InternalURLs;
int SatelliteServerInfo::DanglingServerPort = 0;

SatelliteServerInfo::SatelliteServerInfo(Hdf hdf, const IniSetting::Map &ini) {
  m_name = hdf.getName();

  // Kind of a hack, but XboxServer initializes us in another thread passing an
  // empty config. We can't bind system variables that late, so skip it all.
  if (m_name.empty()) {
    return;
  }

  RuntimeOption::Bind(m_port, ini, hdf, "Port");
  RuntimeOption::Bind(m_threadCount, ini, hdf, "ThreadCount");
  RuntimeOption::Bind(m_maxRequest, ini, hdf, "MaxRequest");
  RuntimeOption::Bind(m_maxDuration, ini, hdf, "MaxDuration");
  int32_t timeoutSeconds;
  RuntimeOption::Bind(timeoutSeconds, ini, hdf, "TimeoutSeconds");
  m_timeoutSeconds = std::chrono::seconds(timeoutSeconds);
  RuntimeOption::Bind(m_reqInitFunc, ini, hdf, "RequestInitFunction");
  RuntimeOption::Bind(m_reqInitDoc, ini, hdf, "RequestInitDocument");
  RuntimeOption::Bind(m_password, ini, hdf, "Password");
  RuntimeOption::Bind(m_passwords, ini, hdf, "Passwords");
  RuntimeOption::Bind(m_alwaysReset, ini, hdf, "AlwaysReset");

  std::string type;
  RuntimeOption::Bind(type, ini, hdf, "Type");
  if (type == "InternalPageServer") {
    m_type = SatelliteServer::Type::KindOfInternalPageServer;
    std::vector<std::string> urls;
    RuntimeOption::Bind(urls, ini, hdf, "URLs");
    for (unsigned int i = 0; i < urls.size(); i++) {
      m_urls.insert(Util::format_pattern(urls[i], true));
    }
    bool blockMainServer;
    RuntimeOption::Bind(blockMainServer, ini, hdf, "BlockMainServer");
    if (blockMainServer) {
      InternalURLs.insert(m_urls.begin(), m_urls.end());
    }
  } else if (type == "DanglingPageServer") {
    m_type = SatelliteServer::Type::KindOfDanglingPageServer;
    DanglingServerPort = m_port;
  } else if (type == "RPCServer") {
    m_type = SatelliteServer::Type::KindOfRPCServer;
  } else {
    m_type = SatelliteServer::Type::Unknown;
  }
}

bool SatelliteServerInfo::checkMainURL(const std::string& path) {
  String url(path.c_str(), path.size(), CopyString);
  for (auto iter = SatelliteServerInfo::InternalURLs.begin();
       iter != SatelliteServerInfo::InternalURLs.end(); ++iter) {
    Variant ret = preg_match
      (String(iter->c_str(), iter->size(), CopyString), url);
    if (ret.toInt64() > 0) {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// InternalPageServer: Server + allowed URL checking

class InternalPageServer : public SatelliteServer {
public:
  explicit InternalPageServer(std::shared_ptr<SatelliteServerInfo> info)
    : m_allowedURLs(info->getURLs()) {
    m_server = ServerFactoryRegistry::createServer
      (RuntimeOption::ServerType, RuntimeOption::ServerIP, info->getPort(),
       info->getThreadCount());
    m_server->setRequestHandlerFactory<HttpRequestHandler>(
      info->getTimeoutSeconds().count());
    m_server->setUrlChecker(std::bind(&InternalPageServer::checkURL, this,
                                      std::placeholders::_1));
  }

  virtual void start() {
    m_server->start();
  }
  virtual void stop() {
    m_server->stop();
    m_server->waitForEnd();
  }
  virtual int getActiveWorker() {
    return m_server->getActiveWorker();
  }
  virtual int getQueuedJobs() {
    return m_server->getQueuedJobs();
  }

private:
  bool checkURL(const std::string &path) const {
    String url(path.c_str(), path.size(), CopyString);
    for (const auto &allowed : m_allowedURLs) {
      Variant ret = preg_match
        (String(allowed.c_str(), allowed.size(), CopyString), url);
      if (ret.toInt64() > 0) {
        return true;
      }
    }
    return false;
  }

  ServerPtr m_server;
  std::set<std::string> m_allowedURLs;
};

///////////////////////////////////////////////////////////////////////////////
// DanglingPageServer: same as Server

class DanglingPageServer : public SatelliteServer {
public:
  explicit DanglingPageServer(std::shared_ptr<SatelliteServerInfo> info) {
    m_server = ServerFactoryRegistry::createServer
      (RuntimeOption::ServerType, RuntimeOption::ServerIP, info->getPort(),
       info->getThreadCount());
    m_server->setRequestHandlerFactory<HttpRequestHandler>(
      info->getTimeoutSeconds().count());
  }

  virtual void start() {
    m_server->start();
  }
  virtual void stop() {
    m_server->stop();
    m_server->waitForEnd();
  }
  virtual int getActiveWorker() {
    return m_server->getActiveWorker();
  }
  virtual int getQueuedJobs() {
    return m_server->getQueuedJobs();
  }
private:
  ServerPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
// RPCServer: Server + RPCRequestHandler

class RPCServer : public SatelliteServer {
public:
  explicit RPCServer(std::shared_ptr<SatelliteServerInfo> info) {
    m_server = ServerFactoryRegistry::createServer
      (RuntimeOption::ServerType, RuntimeOption::ServerIP, info->getPort(),
       info->getThreadCount());
    m_server->setRequestHandlerFactory([info] {
        auto handler = make_unique<RPCRequestHandler>(
          info->getTimeoutSeconds().count(), true);
      handler->setServerInfo(info);
      return handler;
    });
  }

  virtual void start() {
    m_server->start();
  }
  virtual void stop() {
    m_server->stop();
    m_server->waitForEnd();
  }
  virtual int getActiveWorker() {
    return m_server->getActiveWorker();
  }
  virtual int getQueuedJobs() {
    return m_server->getQueuedJobs();
  }
private:
  ServerPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
// SatelliteServer

std::shared_ptr<SatelliteServer>
SatelliteServer::Create(std::shared_ptr<SatelliteServerInfo> info) {
  std::shared_ptr<SatelliteServer> satellite;
  if (info->getPort()) {
    using SatelliteServerPtr = std::shared_ptr<SatelliteServer>;
    switch (info->getType()) {
    case Type::KindOfInternalPageServer:
      satellite = SatelliteServerPtr(new InternalPageServer(info));
      break;
    case Type::KindOfDanglingPageServer:
      satellite = SatelliteServerPtr(new DanglingPageServer(info));
      break;
    case Type::KindOfRPCServer:
      satellite = SatelliteServerPtr(new RPCServer(info));
      break;
    case Type::KindOfXboxServer:
      satellite = SatelliteServerPtr(new RPCServer(info));
      break;
    default:
      assert(false);
    }
    if (satellite) {
      satellite->setName(info->getName());
    }
  }
  return satellite;
}

///////////////////////////////////////////////////////////////////////////////
}
