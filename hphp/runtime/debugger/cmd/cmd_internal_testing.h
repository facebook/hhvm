/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_DEBUGGER_CMD_INTERNAL_TESTING_H_
#define incl_HPHP_DEBUGGER_CMD_INTERNAL_TESTING_H_

#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdInternalTesting : DebuggerCommand {
  CmdInternalTesting() : DebuggerCommand(KindOfInternalTesting) {}

  void help(DebuggerClient&) override;
  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  std::string m_arg;
  bool m_unused{false};
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_DEBUGGER_CMD_INTERNAL_TESTING_H_
