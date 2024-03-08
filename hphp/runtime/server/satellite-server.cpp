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

#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/xbox-request-handler.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/config.h"
#include "hphp/util/text-util.h"
#include <folly/Memory.h>

using std::make_unique;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::set<std::string> SatelliteServerInfo::InternalURLs;

SatelliteServerInfo::SatelliteServerInfo(const IniSetting::Map& ini,
                                         const Hdf& hdf,
                                         const std::string& ini_key /* = "" */
                                        ) {
  m_name = hdf.exists() && !hdf.isEmpty() ? hdf.getName() : ini_key;
  m_name = hdf.getName();
  m_port = Config::GetUInt16(ini, hdf, "Port", 0, false);
  m_serverIP = Config::GetString(ini, hdf, "IP",
                                 Cfg::Server::IP, false);
  m_threadCount = Config::GetInt32(ini, hdf, "ThreadCount", 5, false);
  m_timeoutSeconds = std::chrono::seconds(
    Config::GetInt32(ini, hdf, "TimeoutSeconds",
                      Cfg::Server::RequestTimeoutSeconds, false));
  m_reqInitFunc = Config::GetString(ini, hdf, "RequestInitFunction", "", false);
  m_reqInitDoc = Config::GetString(ini, hdf, "RequestInitDocument", "", false);
  m_functions = Config::GetSet(ini, hdf, "Functions", m_functions, false);

  std::string method  = Config::GetString(ini, hdf, "Method", "", false);
  if (method == "POST") {
    m_method = Transport::Method::POST;
  } else if (method == "GET") {
    m_method = Transport::Method::GET;
  } else {
    m_method = Transport::Method::AUTO;
  }

  std::string type = Config::GetString(ini, hdf, "Type", "", false);
  m_type = SatelliteServer::Type::Unknown;
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
// XboxServer: Server + RPCRequestHandler

struct XboxServer : SatelliteServer {
  explicit XboxServer(std::shared_ptr<SatelliteServerInfo> info) {
    m_server = ServerFactoryRegistry::createServer
      (Cfg::Server::Type, info->getServerIP(), info->getPort(),
       info->getThreadCount());
    m_server->setRequestHandlerFactory([info] {
        auto handler = make_unique<XboxRequestHandler>();
        handler->setLogInfo(true);
      handler->setServerInfo(info);
      return handler;
    });
  }

  void start() override {
    m_server->start();
  }
  void stop() override {
    m_server->stop();
    m_server->waitForEnd();
  }
  size_t getMaxThreadCount() override {
    return m_server->getActiveWorker();
  }
  int getActiveWorker() override {
    return m_server->getActiveWorker();
  }
  int getQueuedJobs() override {
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
    case Type::KindOfXboxServer:
      satellite.reset(new XboxServer(info));
      break;
    default:
      assertx(false);
    }
    if (satellite) {
      satellite->setName(info->getName());
    }
  }
  return satellite;
}

///////////////////////////////////////////////////////////////////////////////
}
