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

#ifndef __HPHP_EVAL_DEBUGGER_CMD_EXTENDED_H__
#define __HPHP_EVAL_DEBUGGER_CMD_EXTENDED_H__

#include <runtime/eval/debugger/debugger_command.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

// we want to use std::map for sorted commands
typedef std::map<std::string, std::string> ExtendedCommandMap;

DECLARE_BOOST_TYPES(CmdExtended);
class CmdExtended : public DebuggerCommand {
public:
  static const ExtendedCommandMap &GetExtendedCommandMap();
  static DebuggerCommandPtr CreateExtendedCommand(const std::string &cls);

public:
  CmdExtended() : DebuggerCommand(KindOfExtended) {}

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);

  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);

  // so CmdUser can override these functions
  virtual const ExtendedCommandMap &getCommandMap();
  virtual void invokeList(DebuggerClient *client, const std::string &cls);
  virtual bool invokeHelp(DebuggerClient *client, const std::string &cls);
  virtual bool invokeClient(DebuggerClient *client, const std::string &cls);

protected:
  void helpImpl(DebuggerClient *client, const char *name);

private:
  ExtendedCommandMap match(DebuggerClient *client, int argIndex);
  bool helpCommands(DebuggerClient *client, const ExtendedCommandMap &matches);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_CMD_EXTENDED_H__
