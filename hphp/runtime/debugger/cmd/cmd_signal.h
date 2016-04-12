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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_SIGNAL_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_SIGNAL_H_

#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdSignal : DebuggerCommand {
  enum Signal {
    SignalNone,
    SignalBreak,
  };

  explicit CmdSignal(Signal sig = SignalNone)
      : DebuggerCommand(KindOfSignal), m_signum(sig) {}

  Signal getSignal() const { return (Signal)m_signum;}

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  int32_t m_signum;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_SIGNAL_H_
