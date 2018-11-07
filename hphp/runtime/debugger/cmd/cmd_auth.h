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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_AUTH_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_AUTH_H_

#include "hphp/runtime/debugger/debugger_command.h"
#include <string>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdAuth : DebuggerCommand {
  CmdAuth() : DebuggerCommand(KindOfAuth) {}

  const std::string& getToken() const { return m_token; }
  const std::string& getSession() const { return m_session; }
  const std::string& getSandboxPath() const { return m_sandboxPath; }

  void setSandboxPath(const std::string& sandboxPath) {
    m_sandboxPath = sandboxPath;
  };

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  std::string m_token;
  std::string m_session;
  std::string m_sandboxPath = "";
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_AUTH_H_
