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

SatelliteServerInfo::SatelliteServerInfo(Hdf hdf) {
  m_name = hdf.getName();
  m_port = hdf["Port"].getInt16(0);
  m_threadCount = hdf["ThreadCount"].getInt32(5);
  m_maxRequest = hdf["MaxRequest"].getInt32(500);
  m_maxDuration = hdf["MaxDuration"].getInt32(120);
  m_timeoutSeconds = std::chrono::seconds
    (hdf["TimeoutSeconds"].getInt32(RuntimeOption::RequestTimeoutSeconds));
  m_reqInitFunc = hdf["RequestInitFunction"].getString("");
  m_reqInitDoc = hdf["RequestInitDocument"].getString("");
  m_password = hdf["Password"].getString("");
  hdf["Passwords"].get(m_passwords);
  m_alwaysReset = hdf["AlwaysReset"].getBool(false);

  string type = hdf["Type"].getString();
  if (type == "InternalPageServer") {
    m_type = SatelliteServer::Type::KindOfInternalPageServer;
    vector<string> urls;
    hdf["URLs"].get(urls);
    for (unsigned int i = 0; i < urls.size(); i++) {
      m_urls.insert(Util::format_pattern(urls[i], true));
    }
    if (hdf["BlockMainServer"].getBool(true)) {
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
  for (std::set<string>::const_iterator iter =
         SatelliteServerInfo::InternalURLs.begin();
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
  explicit InternalPageServer(SatelliteServerInfoPtr info)
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
  explicit DanglingPageServer(SatelliteServerInfoPtr info) {
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
  explicit RPCServer(SatelliteServerInfoPtr info) {
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

SatelliteServerPtr SatelliteServer::Create(SatelliteServerInfoPtr info) {
  SatelliteServerPtr satellite;
  if (info->getPort()) {
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
