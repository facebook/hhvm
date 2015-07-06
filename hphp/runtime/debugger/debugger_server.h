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

#ifndef incl_HPHP_EVAL_DEBUGGER_SERVER_H_
#define incl_HPHP_EVAL_DEBUGGER_SERVER_H_

#include <vector>

#include "hphp/runtime/base/socket.h"
#include "hphp/util/async-func.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

/*
 * Only needed for accepting remote debugger client's connection requests.
 */
struct DebuggerServer {
  /*
   * Start/stop for remote debugging.
   */
  static bool Start();
  static void Stop();

  /////////////////////////////////////////////////////////////////////////////

  DebuggerServer();
  ~DebuggerServer();

  bool start();
  void stop();

  // server thread
  void accept();

private:
  static DebuggerServer s_debugger_server;

  req::ptr<Socket> nthSocket(unsigned i) const {
    return req::make<Socket>(m_socks[i]);
  }

  AsyncFunc<DebuggerServer> m_serverThread;
  bool m_stopped;
  std::vector<std::shared_ptr<SocketData>> m_socks;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_SERVER_H_
