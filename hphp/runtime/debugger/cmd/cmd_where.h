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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_WHERE_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_WHERE_H_

#include "hphp/runtime/debugger/debugger_command.h"
#include "hphp/runtime/base/req-root.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdWhere : DebuggerCommand {
  CmdWhere() : DebuggerCommand(KindOfWhere) {}

  void help(DebuggerClient& client) override;

  bool onServer(DebuggerProxy& proxy) override;
  void onClient(DebuggerClient& client) override;

  Array fetchStackTrace(DebuggerClient &client); // client side

protected:
 void sendImpl(DebuggerThriftBuffer& thrift) override;
 void recvImpl(DebuggerThriftBuffer& thrift) override;

private:
  req::root<Array> m_stacktrace;
  bool m_stackArgs{true};
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_WHERE_H_
