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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_VARIABLE_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_VARIABLE_H_

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/req-root.h"
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
  req::root<Array> m_variables;
  int m_frame;

  /* Serialization limit on a single variable value.  -1 means unlimited. */
  int m_formatMaxLen{200};

  /* Set true by onServer if it used g_context->m_globalVarEnv. */
  bool m_global;

  req::root<String> m_varName;
  req::root<String> m_filter;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_VARIABLE_H_
