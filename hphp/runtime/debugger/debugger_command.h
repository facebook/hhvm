/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EVAL_DEBUGGER_COMMAND_H_
#define incl_HPHP_EVAL_DEBUGGER_COMMAND_H_

#include <memory>
#include <string>

#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct DebuggerThriftBuffer;

namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct DebuggerClient;
struct DebuggerProxy;

struct DebuggerCommand;
using DebuggerCommandPtr = std::shared_ptr<DebuggerCommand>;

/*
 * DebuggerCommand is the base of all commands executed by the debugger. It
 * also represents the base binary communication format between DebuggerProxy
 * and DebuggerClient.
 *
 * Each command has serialization logic, plus client- and server-side logic.
 * Client-side logic is implemented in the onClient* methods, while server-side
 * is in the onServer* methods.
 */
struct DebuggerCommand {
  /*
   * Warning: Do NOT modify exists values, as they are used in binary network
   * protocol, and changing them may create incompatibility between different
   * versions of debugger and server!
   */
  enum Type {
    KindOfNone                = 0,
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
    KindOfVariableAsync       = 222,
    KindOfWhere               = 23,
    KindOfWhereAsync          = 223,
    KindOfExtended            = 24,
    KindOfComplete            = 27,

    KindOfEval                = 1000,
    KindOfShell               = 1001,
    KindOfMacro               = 1002,
    KindOfConfig              = 1003,
    KindOfInstrument          = 1004,

    // DebuggerProxy -> DebuggerClient
    KindOfInterrupt           = 10000,
    KindOfSignal              = 10001,

    // Internal testing only
    KindOfInternalTesting     = 20000, // The real test command
    KindOfInternalTestingBad  = 20001, // A command type we never recgonize
  };

  static bool Receive(DebuggerThriftBuffer &thrift, DebuggerCommandPtr &cmd,
                      const char *caller);

public:
  explicit DebuggerCommand(Type type): m_type(type) {}
  virtual ~DebuggerCommand() {}

  bool is(Type type) const {
    return m_type == type;
  }

  Type getType() const {
    return m_type;
  }

  bool send(DebuggerThriftBuffer&);
  bool recv(DebuggerThriftBuffer&);

  virtual void list(DebuggerClient&);
  virtual void help(DebuggerClient&);

  virtual void onClient(DebuggerClient&) = 0;
  virtual bool onServer(DebuggerProxy&);

  // Returns true if DebuggerProxy::processInterrupt() should return
  // to its caller instead of processing further commands from the client.
  bool shouldExitInterrupt() const {
    return m_exitInterrupt;
  }

  // Returns a non empty error message if the receipt of this command
  // did not complete successfully.
  const String& getWireError() const {
    return m_wireError;
  }

protected:
  bool displayedHelp(DebuggerClient&);
  virtual void sendImpl(DebuggerThriftBuffer&);
  virtual void recvImpl(DebuggerThriftBuffer&);

  Type m_type;
  int m_version{0};

  std::string m_class; // for CmdExtended
  std::string m_body;

  // Used to save temporary error happened on the wire.
  String m_wireError;

  // Server side breaking out of message loop.
  bool m_exitInterrupt{false};

  // Another interrupt comes before the command could finish.
  bool m_incomplete{false};
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_COMMAND_H_
