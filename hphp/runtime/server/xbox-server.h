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

#ifndef incl_HPHP_XBOX_SERVER_H_
#define incl_HPHP_XBOX_SERVER_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/satellite-server.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(XboxServerInfo);

class RPCRequestHandler;

class XboxServer {
public:
  /**
   * Start or restart xbox server.
   */
  static void Restart();
  static void Stop();

public:
  /**
   * Send/PostMessage paradigm for local and remote RPC.
   */
  static bool SendMessage(CStrRef message, Variant &ret, int timeout_ms,
                          CStrRef host = "localhost");
  static bool PostMessage(CStrRef message, CStrRef host = "localhost");

  /**
   * Local tasklet for parallel processing.
   */
  static Resource TaskStart(CStrRef msg, CStrRef reqInitDoc = "");
  static bool TaskStatus(CResRef task);
  static int TaskResult(CResRef task, int timeout_ms, Variant &ret);

  /**
   * Gets the ServerInfo and RequestHandler for the current xbox worker thread.
   * Returns NULL for non-xbox threads.
   */
  static XboxServerInfoPtr GetServerInfo();
  static RPCRequestHandler *GetRequestHandler();
};

///////////////////////////////////////////////////////////////////////////////

class XboxServerInfo : public SatelliteServerInfo {
public:
  XboxServerInfo() : SatelliteServerInfo(Hdf()) {
    m_type = SatelliteServer::Type::KindOfXboxServer;
    m_name = "xbox";
    reload();
  }

  void reload() {
    m_threadCount = RuntimeOption::XboxServerThreadCount;
    m_port        = RuntimeOption::XboxServerPort;
    m_maxRequest  = RuntimeOption::XboxServerInfoMaxRequest;
    m_maxDuration = RuntimeOption::XboxServerInfoDuration;
    m_reqInitFunc = RuntimeOption::XboxServerInfoReqInitFunc;
    m_reqInitDoc  = RuntimeOption::XboxServerInfoReqInitDoc;
    m_alwaysReset = RuntimeOption::XboxServerInfoAlwaysReset;
  }

  void setMaxDuration(int duration) { m_maxDuration = duration; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XBOX_SERVER_H_
