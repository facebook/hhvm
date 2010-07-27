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

#include <runtime/eval/debugger/cmd/cmd_where.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdWhere::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_stacktrace);
}

void CmdWhere::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_stacktrace);
}

bool CmdWhere::help(DebuggerClient *client) {
  client->helpTitle("Where Command");
  client->help("[w]here: displays current stacktrace");
  client->helpBody(
    "Use '[u]p {num}' or '[d]own {num}' to walk up or down the stacktrace. "
    "Use '[f]rame {index}' to jump to one particular frame. At any frame, "
    "use '[v]ariable' command to display all local variables."
  );
  return true;
}

Array CmdWhere::fetchStackTrace(DebuggerClient *client) {
  Array st = client->getStackTrace();
  if (st.isNull()) {
    CmdWherePtr cmd = client->xend<CmdWhere>(this);
    client->setStackTrace(cmd->m_stacktrace);
  }
  return st;
}

bool CmdWhere::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;
  if (client->argCount() != 0) {
    return help(client);
  }

  Array st = fetchStackTrace(client);
  if (st.empty()) {
    client->info("(no stacktrace to display)");
  } else {
    int i = 0;
    for (ArrayIter iter(st); iter; ++iter) {
      client->printFrame(iter.first().toInt32(), iter.second());
      if (++i % 10 == 0 &&
          client->ask("There are %d more frames. Continue? [Y/n]",
                      st.size() - i) == 'n') {
        break;
      }
    }
  }

  return true;
}

bool CmdWhere::onServer(DebuggerProxy *proxy) {
  m_stacktrace = FrameInjection::GetBacktrace(false, false, false);
  return proxy->send(this);
}

///////////////////////////////////////////////////////////////////////////////
}}
