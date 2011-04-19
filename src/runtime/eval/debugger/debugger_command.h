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

#ifndef __HPHP_EVAL_DEBUGGER_COMMAND_H__
#define __HPHP_EVAL_DEBUGGER_COMMAND_H__

#include <util/base.h>
#include <runtime/eval/debugger/debugger_thrift_buffer.h>
#include <runtime/eval/debugger/debugger_client.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

/**
 * Binary communication format between DebuggerProxy and DebuggerClient.
 */
DECLARE_BOOST_TYPES(DebuggerCommand);
class DebuggerCommand {
public:
  /**
   * Warning: Do NOT modify exists values, as they are used in binary network
   * protocol, and changing them may create incompatibility between different
   * versions of debugger and server!
   */
  enum Type {
    // Now I sing my A, B, C...
    KindOfAbort               = 1,
    KindOfBreak               = 2,
    KindOfContinue            = 3,
    KindOfDown                = 4,
    KindOfException           = 5,
    KindOfFrame               = 6,
    KindOfGlobal              = 7,
    KindOfHelp                = 8,
    KindOfInfo                = 9,
    KindOfJump                = 10,
    KindOfConstant            = 11,
    KindOfList                = 12,
    KindOfMachine             = 13,
    KindOfNext                = 14,
    KindOfOut                 = 15,
    KindOfPrint               = 16,
    KindOfQuit                = 17,
    KindOfRun                 = 18,
    KindOfStep                = 19,
    KindOfThread              = 20,
    KindOfUp                  = 21,
    KindOfVariable            = 22,
    KindOfWhere               = 23,
    KindOfExtended            = 24,
    KindOfUser                = 25,
    KindOfZend                = 26,

    KindOfEval                = 1000,
    KindOfShell               = 1001,
    KindOfMacro               = 1002,

    // DebuggerProxy -> DebuggerClient
    KindOfInterrupt           = 10000,
    KindOfSignal              = 10001,
  };

  static bool Receive(DebuggerThriftBuffer &thrift, DebuggerCommandPtr &cmd,
                      const char *caller);

public:
  DebuggerCommand(Type type)
      : m_type(type), m_version(0), m_exitInterrupt(false) {}

  bool is(Type type) const { return m_type == type;}
  Type getType() const { return m_type;}

  bool send(DebuggerThriftBuffer &thrift);
  bool recv(DebuggerThriftBuffer &thrift);

  virtual void list(DebuggerClient *client);
  virtual bool help(DebuggerClient *client);
  virtual bool onClient(DebuggerClient *client);
  virtual bool onServer(DebuggerProxy *proxy);
  virtual void sendImpl(DebuggerThriftBuffer &thrift);
  virtual void recvImpl(DebuggerThriftBuffer &thrift);

  /**
   * A server command processing can set m_exitInterrupt to true to break
   * message loop in DebuggerProxy::processInterrupt().
   */
  virtual bool shouldExitInterrupt() { return m_exitInterrupt;}

protected:
  Type m_type;
  std::string m_class; // for CmdExtended
  std::string m_body;
  int m_version;

  bool m_exitInterrupt; // server side breaking out of message loop
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_COMMAND_H__
