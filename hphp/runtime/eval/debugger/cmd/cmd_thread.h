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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_THREAD_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_THREAD_H_

#include "hphp/runtime/eval/debugger/debugger_command.h"
#include "hphp/runtime/base/debuggable.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdThread);
class CmdThread : public DebuggerCommand, public IDebuggable {
public:
  CmdThread() : DebuggerCommand(KindOfThread) {}

  virtual void list(DebuggerClient &client);
  virtual void help(DebuggerClient &client);

  virtual bool onServer(DebuggerProxy &proxy);

protected:
  virtual void onClientImpl(DebuggerClient &client);
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  // implementing IDebuggable
  virtual void debuggerInfo(InfoVec &info);

  String m_out;
  DThreadInfoPtrVec m_threads;

  void processList(DebuggerClient &client, bool output = true);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_THREAD_H_
