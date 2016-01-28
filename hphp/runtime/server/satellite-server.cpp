/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/config.h"
#include "hphp/util/text-util.h"
#include <folly/Memory.h>

using folly::make_unique;
using std::set;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::set<std::string> SatelliteServerInfo::InternalURLs;
int SatelliteServerInfo::DanglingServerPort = 0;

SatelliteServerInfo::SatelliteServerInfo(const IniSetting::Map& ini,
                                         const Hdf& hdf,
                                         const std::string& ini_key /* = "" */
                                        ) {
  m_name = hdf.exists() && !hdf.isEmpty() ? hdf.getName() : ini_key;
  m_name = hdf.getName();
  m_port = Config::GetUInt16(ini, hdf, "Port", 0, false);
  m_threadCount = Config::GetInt32(ini, hdf, "ThreadCount", 5, false);
  m_maxRequest = Config::GetInt32(ini, hdf, "MaxRequest", 500, false);
  m_maxDuration = Config::GetInt32(ini, hdf, "MaxDuration", 120, false);
  m_timeoutSeconds = std::chrono::seconds(
    Config::GetInt32(ini, hdf, "TimeoutSeconds",
                      RuntimeOption::RequestTimeoutSeconds, false));
  m_reqInitFunc = Config::GetString(ini, hdf, "RequestInitFunction", "", false);
  m_reqInitDoc = Config::GetString(ini, hdf, "RequestInitDocument", "", false);
  m_password = Config::GetString(ini, hdf, "Password", "", false);
  m_passwords = Config::GetSet(ini, hdf, "Passwords", m_passwords, false);
  m_alwaysReset = Config::GetBool(ini, hdf, "AlwaysReset", false, false);
  m_functions = Config::GetSet(ini, hdf, "Functions", m_functions, false);

  std::string type = Config::GetString(ini, hdf, "Type", "", false);
  if (type == "InternalPageServer") {
    m_type = SatelliteServer::Type::KindOfInternalPageServer;
    std::vector<std::string> urls;
    urls = Config::GetVector(ini, hdf, "URLs", urls, false);
    for (unsigned int i = 0; i < urls.size(); i++) {
      m_urls.insert(format_pattern(urls[i], true));
    }
    if (Config::GetBool(ini, hdf, "BlockMainServer", true, false)) {
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

std::unique_ptr<SatelliteServer>
SatelliteServer::Create(std::shared_ptr<SatelliteServerInfo> info) {
  std::unique_ptr<SatelliteServer> satellite;
  if (info->getPort()) {
    switch (info->getType()) {
    case Type::KindOfInternalPageServer:
      satellite.reset(new InternalPageServer(info));
      break;
    case Type::KindOfDanglingPageServer:
      satellite.reset(new DanglingPageServer(info));
      break;
    case Type::KindOfRPCServer:
      satellite.reset(new RPCServer(info));
      break;
    case Type::KindOfXboxServer:
      satellite.reset(new RPCServer(info));
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
