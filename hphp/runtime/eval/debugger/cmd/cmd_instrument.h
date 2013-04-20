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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_INSTRUMENT_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_INSTRUMENT_H_

#include <runtime/eval/debugger/debugger_command.h>
#include <runtime/eval/debugger/inst_point.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(CmdInstrument);
class CmdInstrument : public DebuggerCommand {
public:
  CmdInstrument() : DebuggerCommand(KindOfInstrument), m_type(ActionRead),
                    m_enabled(false), m_instPoints(nullptr) {}

  virtual bool help(DebuggerClient *client);
  virtual bool onClient(DebuggerClient *client) {
    client->error("not supported\n");
    return true;
  }
  virtual void setClientOutput(DebuggerClient *client);
  virtual bool onClientVM(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy) {
    return true;
  }
  virtual bool onServerVM(DebuggerProxy *proxy);

  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

private:
  enum ActionType {
    ActionRead,
    ActionWrite,
  };
  int8_t m_type;
  bool m_enabled;

  InstPointInfoPtrVec *m_instPoints;
  InstPointInfoPtrVec m_ips;

  void readFromTable(DebuggerProxyVM *proxy);
  void validateAndWriteToTable(DebuggerProxyVM *proxy);

  void listInst(DebuggerClient *client);
  void clearInst(DebuggerClient *client);

  static void PrintInstPoints(DebuggerClient *client);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_INSTRUMENT_H_
