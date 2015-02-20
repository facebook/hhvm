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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_VARIABLE_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_VARIABLE_H_

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/debugger/debugger_command.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

struct CmdVariable : DebuggerCommand {
  static Array GetGlobalVariables();
  static void PrintVariable(DebuggerClient &client, const String& varName);
  static void PrintVariables(DebuggerClient &client, const Array& variables,
                              int frame, const String& text, int version);

  explicit CmdVariable(Type type = KindOfVariable) : DebuggerCommand(type) {
    m_frame = 0;
    m_version = 1;
    m_global = false;
  }

  void help(DebuggerClient&) override;

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

private:
  Array m_variables;
  int m_frame;
  bool m_global; // Set true by onServer if it used g_context->m_globalVarEnv
  String m_varName;
  String m_filter;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_VARIABLE_H_
