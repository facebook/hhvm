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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_USER_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_USER_H_

#include "hphp/runtime/eval/debugger/cmd/cmd_extended.h"
#include "hphp/util/lock.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdUser);
class CmdUser : public CmdExtended {
public:
  static bool InstallCommand(CStrRef cmd, CStrRef clsname);
  static Array GetCommands();

public:
  CmdUser() {
    m_type = KindOfUser;
  }
  explicit CmdUser(Object cmd) : m_cmd(cmd) {
    m_type = KindOfUser;
  }

  Object getUserCommand() { return m_cmd;}

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  virtual const ExtendedCommandMap &getCommandMap();
  virtual void invokeList(DebuggerClient *client, const std::string &cls);
  virtual bool invokeHelp(DebuggerClient *client, const std::string &cls);
  virtual bool invokeClient(DebuggerClient *client, const std::string &cls);

protected:
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  static Mutex s_mutex;
  static ExtendedCommandMap s_commands;

  Object m_cmd;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_USER_H_
