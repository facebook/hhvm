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

#include <runtime/base/complex_types.h>
#include <runtime/base/server/server.h>
#include <runtime/base/server/satellite_server.h>
#include <runtime/base/preg.h>
#include <signal.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// statics

static HPHP::ServerPtrVec AllServers;
static void on_kill(int sig) {
  signal(sig, SIG_DFL);
  for (unsigned int i = 0; i < AllServers.size(); i++) {
    AllServers[i]->stop();
  }
  raise(sig);
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool Server::StackTraceOnError = true;

void Server::InstallStopSignalHandlers(ServerPtr server) {
  if (AllServers.empty()) {
    signal(SIGTERM, on_kill);
    signal(SIGUSR1, on_kill);
  }

  AllServers.push_back(server);
}

///////////////////////////////////////////////////////////////////////////////

Server::Server(const std::string &address, int port, int threadCount)
  : m_address(address), m_port(port), m_threadCount(threadCount),
    m_status(NOT_YET_STARTED) {
}

bool Server::shouldHandle(const std::string &cmd) {
  String url(cmd.c_str(), cmd.size(), AttachLiteral);
  for (set<string>::const_iterator iter =
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
}
