/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/cmd/cmd_eval.h>
#include <runtime/eval/eval.h>
#include <runtime/base/externals.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

bool CmdEval::onClient(DebuggerClient *client) {
  m_body = client->getCode();
  CmdEvalPtr res = client->xend<CmdEval>(this);
  client->print(res->m_body);
  return true;
}

bool CmdEval::onServer(DebuggerProxy *proxy) {
  PSEUDOMAIN_INJECTION(_); // using "_" as filename

  String output;
  try {
    g_context->obStart("");
    eval(get_variable_table(), Object(),
         String(m_body.c_str(), m_body.size(), AttachLiteral), false);
    output = Debugger::ColorStdout(g_context->obDetachContents());
    g_context->obClean();
    g_context->obEnd();
  } catch (Exception &e) {
    output = Debugger::ColorStderr(String(e.what()));
  }

  m_body = std::string(output.data(), output.size());
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
