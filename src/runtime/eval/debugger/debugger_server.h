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

#ifndef __HPHP_EVAL_DEBUGGER_SERVER_H__
#define __HPHP_EVAL_DEBUGGER_SERVER_H__

#include <util/async_func.h>
#include <runtime/base/file/socket.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

/**
 * Only needed for accepting remote debugger client's connection requests.
 */
class DebuggerServer {
public:
  /**
   * Start/stop for remote debugging.
   */
  static bool Start();
  static void Stop();

public:
  DebuggerServer();

  bool start();
  void stop();

  // server thread
  void accept();

private:
  static DebuggerServer s_debugger_server;

  AsyncFunc<DebuggerServer> m_serverThread;
  bool m_stopped;
  SmartPtr<Socket> m_sock;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_SERVER_H__
