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

#include "hphp/runtime/base/server/satellite_server.h"
#include "hphp/runtime/base/server/libevent_server.h"
#include "hphp/runtime/base/server/http_request_handler.h"
#include "hphp/runtime/base/server/rpc_request_handler.h"
#include "hphp/runtime/base/server/virtual_host.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/util/util.h"
#include "folly/Memory.h"

#include <boost/make_shared.hpp>

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
  m_timeoutSeconds =
    hdf["TimeoutSeconds"].getInt32(RuntimeOption::RequestTimeoutSeconds);
  m_reqInitFunc = hdf["RequestInitFunction"].getString("");
  m_reqInitDoc = hdf["RequestInitDocument"].getString("");
  m_password = hdf["Password"].getString("");
  hdf["Passwords"].get(m_passwords);
  m_alwaysReset = hdf["AlwaysReset"].getBool(false);

  string type = hdf["Type"].getString();
  if (type == "InternalPageServer") {
    m_type = SatelliteServer::KindOfInternalPageServer;
    vector<string> urls;
    hdf["URLs"].get(urls);
    for (unsigned int i = 0; i < urls.size(); i++) {
      m_urls.insert(Util::format_pattern(urls[i], true));
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

bool SatelliteServerInfo::checkMainURL(const std::string& path) {
  String url(path.c_str(), path.size(), AttachLiteral);
  for (std::set<string>::const_iterator iter =
         SatelliteServerInfo::InternalURLs.begin();
       iter != SatelliteServerInfo::InternalURLs.end(); ++iter) {
    Variant ret = preg_match
      (String(iter->c_str(), iter->size(), AttachLiteral), url);
    if (ret.toInt64() > 0) {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// InternalPageServer: LibEventServer + allowed URL checking

class InternalPageServer : public SatelliteServer {
public:
  explicit InternalPageServer(SatelliteServerInfoPtr info)
    : m_allowedURLs(info->getURLs()) {
    m_server = boost::make_shared<LibEventServer>(
        RuntimeOption::ServerIP, info->getPort(), info->getThreadCount(),
        info->getTimeoutSeconds());
    m_server->setRequestHandlerFactory<HttpRequestHandler>();
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

private:
  bool checkURL(const std::string &path) const {
    String url(path.c_str(), path.size(), AttachLiteral);
    for (const auto &allowed : m_allowedURLs) {
      Variant ret = preg_match
        (String(allowed.c_str(), allowed.size(), AttachLiteral), url);
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
// DanglingPageServer: same as LibEventServer

class DanglingPageServer : public SatelliteServer {
public:
  explicit DanglingPageServer(SatelliteServerInfoPtr info) {
    m_server = boost::make_shared<LibEventServer>(
        RuntimeOption::ServerIP, info->getPort(), info->getThreadCount(),
        info->getTimeoutSeconds());
    m_server->setRequestHandlerFactory<HttpRequestHandler>();
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

class RPCServer : public SatelliteServer {
public:
  explicit RPCServer(SatelliteServerInfoPtr info) {
    m_server = boost::make_shared<LibEventServer>(
        RuntimeOption::ServerIP, info->getPort(), info->getThreadCount(),
        info->getTimeoutSeconds());
    m_server->setRequestHandlerFactory([info] {
      auto handler = make_unique<RPCRequestHandler>();
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
private:
  ServerPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
// SatelliteServer

SatelliteServerPtr SatelliteServer::Create(SatelliteServerInfoPtr info) {
  SatelliteServerPtr satellite;
  if (info->getPort()) {
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
    case KindOfXboxServer:
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
