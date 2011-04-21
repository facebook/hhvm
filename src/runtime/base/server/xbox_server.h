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

#ifndef __HPHP_XBOX_SERVER_H__
#define __HPHP_XBOX_SERVER_H__

#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/satellite_server.h>

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

public:
  /**
   * Send/PostMessage paradigm for local and remote RPC.
   */
  static bool SendMessage(CStrRef message, Variant &ret, int timeout_ms,
                          CStrRef host = "localhost");
  static bool PostMessage(CStrRef message, CStrRef host = "localhost");

  /**
   * Check whether all the the xbox threads are busy
   */
  static bool Available();

  /**
   * Local tasklet for parallel processing.
   */
  static Object TaskStart(CStrRef message);
  static bool TaskStatus(CObjRef task);
  static int TaskResult(CObjRef task, int timeout_ms, Variant &ret);

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
    m_type = SatelliteServer::KindOfXboxServer;
    m_name = "xbox";
    reload();
  }

  void reload() {
    m_threadCount = RuntimeOption::XboxServerThreadCount;
    m_port        = RuntimeOption::XboxServerPort;
    m_maxRequest  = RuntimeOption::XboxServerInfoMaxRequest;
    m_maxDuration = RuntimeOption::XboxServerInfoDuration;
    m_warmupDoc   = RuntimeOption::XboxServerInfoWarmupDoc;
    m_reqInitFunc = RuntimeOption::XboxServerInfoReqInitFunc;
    m_reqInitDoc  = RuntimeOption::XboxServerInfoReqInitDoc;
  }

  void setMaxDuration(int duration) { m_maxDuration = duration; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_XBOX_SERVER_H__
