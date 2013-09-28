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

#ifndef incl_HPHP_SATELLITE_SERVER_H_
#define incl_HPHP_SATELLITE_SERVER_H_

#include "hphp/util/hdf.h"

#include <chrono>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SatelliteServerInfo);
DECLARE_BOOST_TYPES(SatelliteServer);
class SatelliteServer {
public:
  enum class Type {
    Unknown,

    KindOfInternalPageServer,  // handles restricted URLs
    KindOfDanglingPageServer,  // handles old version requests during shutdown
    KindOfRPCServer,           // invokes one PHP function and returns JSON
    KindOfXboxServer,          // handles internal xbox tasks
  };

  void setName(const std::string &name) { m_name = name;}
  const std::string &getName() const { return m_name;}

public:
  static SatelliteServerPtr Create(SatelliteServerInfoPtr info);

  virtual ~SatelliteServer() {}

  virtual void start() = 0;
  virtual void stop() = 0;
  virtual int getActiveWorker() = 0;
  virtual int getQueuedJobs() = 0;

private:
  std::string m_name;
};

///////////////////////////////////////////////////////////////////////////////
// helpers

class SatelliteServerInfo {
public:
  /**
   * These are regular expressions of URLs that are not allowed on main server.
   * These are collected from all internal page servers.
   */
  static std::set<std::string> InternalURLs;
  static int DanglingServerPort;

  /**
   * Check whether a requested path should be allowed on the main server.
   */
  static bool checkMainURL(const std::string& path);

public:
  explicit SatelliteServerInfo(Hdf hdf);

  const std::string &getName() const { return m_name;}
  SatelliteServer::Type getType() const { return m_type;}
  int getPort() const { return m_port;}
  int getThreadCount() const { return m_threadCount;}

  // for all libevent servers
  std::chrono::seconds getTimeoutSeconds() const { return m_timeoutSeconds;}

  // only for InternalPageServer
  const std::set<std::string> &getURLs() const { return m_urls;}

  // only for RPCServer
  int getMaxRequest() const { return m_maxRequest;}
  int getMaxDuration() const { return m_maxDuration;}
  const std::string &getReqInitFunc() const { return m_reqInitFunc;}
  const std::string &getReqInitDoc() const { return m_reqInitDoc;}
  const std::string &getPassword() const { return m_password;}
  const std::set<std::string> &getPasswords() const { return m_passwords;}
  bool alwaysReset() const { return m_alwaysReset;}

protected:
  std::string m_name;
  SatelliteServer::Type m_type;
  int m_port;
  int m_threadCount;
  int m_maxRequest;
  int m_maxDuration;
  std::chrono::seconds m_timeoutSeconds;
  std::set<std::string> m_urls; // url regex patterns
  std::string m_reqInitFunc;
  std::string m_reqInitDoc;
  std::string m_password;
  std::set<std::string> m_passwords;
  bool m_alwaysReset;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SATELLITE_SERVER_H_
