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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_SIGNAL_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_SIGNAL_H_

#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdSignal);
class CmdSignal : public DebuggerCommand {
public:
  enum Signal {
    SignalNone,
    SignalBreak,
  };

public:
  explicit CmdSignal(Signal sig = SignalNone)
      : DebuggerCommand(KindOfSignal), m_signum(sig) {}

  Signal getSignal() const { return (Signal)m_signum;}

  virtual bool onServer(DebuggerProxy &proxy);
  virtual void onClient(DebuggerClient &client);

protected:
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  int32_t m_signum;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_SIGNAL_H_
