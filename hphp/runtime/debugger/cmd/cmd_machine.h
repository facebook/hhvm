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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_MACHINE_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_MACHINE_H_

#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdMachine);
class CmdMachine : public DebuggerCommand {
public:
  static bool AttachSandbox(DebuggerClient &client, const char *user = nullptr,
                            const char *name = nullptr, bool force = false);
  static void UpdateIntercept(DebuggerClient &client,
                              const std::string &host, int port);

public:
  CmdMachine() : DebuggerCommand(KindOfMachine),
                 m_force(false), m_succeed(false) {}

  virtual void list(DebuggerClient &client);
  virtual void help(DebuggerClient &client);

  virtual bool onServer(DebuggerProxy &proxy);
  virtual void onClient(DebuggerClient &client);

protected:
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  static bool AttachSandbox(DebuggerClient &client,
                            DSandboxInfoPtr sandbox,
                            bool force = false);

  DSandboxInfoPtrVec m_sandboxes;
  Array m_rpcConfig;
  bool m_force;
  bool m_succeed;

  bool processList(DebuggerClient &client, bool output = true);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_MACHINE_H_
